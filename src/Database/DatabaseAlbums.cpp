/* DatabaseAlbums.cpp */

/* Copyright (C) 2011-2017  Lucio Carreras
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Database/SayonaraQuery.h"
#include "Database/DatabaseAlbums.h"
#include "Utils/MetaData/Album.h"
#include "Utils/Library/Filter.h"

using DB::Albums;
using DB::Query;

struct Albums::Private
{
	QString artist_id_field;
	QString artist_name_field;
	QString search_view;
	QString track_view;

	Private(int8_t library_id)
	{
		artist_id_field = "artistID";
		artist_name_field = "name";
		search_view = QString("track_search_view_%1").arg(library_id);

		if(library_id < 0) {
			track_view = "tracks";
		}

		else {
			track_view = QString("track_view_%1").arg(library_id);
		}
	}
};

Albums::Albums(QSqlDatabase db, uint8_t db_id, int8_t library_id) :
	DB::SearchMode(db, db_id)
{
	m = Pimpl::make<Private>(library_id);
}

Albums::~Albums() {}

QString Albums::fetch_query_albums(bool also_empty) const
{
	QString sql =
			"SELECT "
			"  albums.albumID AS albumID"
			", albums.name AS albumName"
			", SUM( " + m->track_view + ".length) / 1000 AS albumLength"
			", albums.rating AS albumRating"
			", COUNT(DISTINCT " + m->track_view + ".trackID) AS trackCount"
			", MAX(" + m->track_view + ".year) AS albumYear"
			", GROUP_CONCAT(DISTINCT artists.name)"
			", GROUP_CONCAT(DISTINCT albumArtists.name)"
			", GROUP_CONCAT(DISTINCT " + m->track_view + ".discnumber)"
			" FROM albums ";

	QString join = " INNER JOIN ";
	if(also_empty){
		join = " LEFT OUTER JOIN ";
	}

	sql +=	join + " " +  m->track_view + " ON " +  m->track_view + ".albumID = albums.albumID " +
			join + " artists ON " + m->track_view + ".artistID = artists.artistID " +
			join + " artists albumArtists ON " + m->track_view + ".albumArtistID = albumArtists.artistID ";

	return sql;
}


bool Albums::db_fetch_albums(Query& q, AlbumList& result)
{
	result.clear();

	if (!q.exec()) {
		q.show_error("Could not get all albums from database");
		return false;
	}

	result.reserve(q.fetched_rows());

	while(q.next())
	{
		Album album;

		album.id =			q.value(0).toInt();
		album.set_name(		q.value(1).toString().trimmed());
		album.length_sec =	q.value(2).toInt();
		album.rating =		q.value(3).toInt();
		album.num_songs =	q.value(4).toInt();
		album.year =		q.value(5).toInt();
		album.set_artists(	q.value(6).toString().split(','));
		album.set_album_artists(q.value(7).toString().split(','));

		album.discnumbers.clear();
		QStringList discs =	q.value(8).toString().split(',');
		discs.removeDuplicates();

		for(const QString& disc : discs) {
			album.discnumbers << disc.toInt();
		}

		if(album.discnumbers.isEmpty()) {
			album.discnumbers << 1;
		}

		album.n_discs = album.discnumbers.size();
		album.is_sampler = (album.artists().size() > 1);
		album.set_db_id(module_db_id());

		result.push_back(std::move(album));
	};

	return true;
}


QString Albums::_create_order_string(Library::SortOrder sortorder)
{
	switch(sortorder)
	{
		case Library::SortOrder::AlbumNameAsc:
			return QString (" ORDER BY albumName ASC ");

		case Library::SortOrder::AlbumNameDesc:
			return QString (" ORDER BY albumName DESC ");

		case Library::SortOrder::AlbumYearAsc:
			return QString (" ORDER BY albumYear ASC, albumName ASC ");

		case Library::SortOrder::AlbumYearDesc:
			return QString (" ORDER BY albumYear DESC, albumName ASC ");

		case Library::SortOrder::AlbumTracksAsc:
			return QString (" ORDER BY trackCount ASC, albumName ASC ");

		case Library::SortOrder::AlbumTracksDesc:
			return QString (" ORDER BY trackCount DESC, albumName ASC ");

		case Library::SortOrder::AlbumDurationAsc:
			return QString (" ORDER BY albumLength ASC, albumName ASC ");

		case Library::SortOrder::AlbumDurationDesc:
			return QString (" ORDER BY albumLength DESC, albumName ASC ");

		case Library::SortOrder::AlbumRatingAsc:
			return QString (" ORDER BY albumRating ASC, albumName ASC ");

		case Library::SortOrder::AlbumRatingDesc:
			return QString (" ORDER BY albumRating DESC, albumName ASC ");

		default:
			return "";
	}
}

int Albums::getAlbumID (const QString & album)
{
	Query q(this);
	int albumID = -1;

	q.prepare("SELECT albumID FROM albums WHERE name = ?;");
	q.addBindValue(album);

	if(!q.exec()) {
		return -1;
	}

	if (q.next()) {
		albumID = q.value(0).toInt();
	}

	return albumID;
}


bool Albums::getAlbumByID(const int& id, Album& album, bool also_empty)
{
	if(id == -1) {
		return false;
	}

	Query q(this);
	QString querytext =	fetch_query_albums(also_empty) +
						" WHERE albums.albumID = :id "
						" GROUP BY albums.albumID, albums.name, albums.rating ";

	q.prepare(querytext);
	q.bindValue(":id", id);

	AlbumList albums;
	db_fetch_albums(q, albums);

	if(albums.size() > 0) {
		album = albums.first();
	}

	return (albums.size() > 0);
}

bool Albums::getAllAlbums(AlbumList& result, bool also_empty)
{
	return getAllAlbums(result, Library::SortOrder::AlbumNameAsc, also_empty);
}

bool Albums::getAllAlbums(AlbumList& result, Library::SortOrder sortorder, bool also_empty)
{
	Query q(this);
	QString querytext = fetch_query_albums(also_empty);

	querytext += " GROUP BY albums.albumID, albums.name, albums.rating ";
	querytext += _create_order_string(sortorder) + ";";

	q.prepare(querytext);

	return db_fetch_albums(q, result);
}


bool Albums::getAllAlbumsByArtist(IDList artists, AlbumList& result)
{
	return getAllAlbumsByArtist(artists, result, Library::Filter());
}


bool Albums::getAllAlbumsByArtist(IDList artists, AlbumList& result, const Library::Filter& filter, Library::SortOrder sortorder)
{
	if(artists.isEmpty()) {
		return false;
	}

	Query q(this);
	QString select = "SELECT "
					 "  albumID"
					 ", albumName"
					 ", SUM(length) / 1000 AS albumLength"
					 ", albumRating"
					 ", COUNT(DISTINCT trackID) AS trackCount"
					 ", MAX(year) AS albumYear"
					 ", GROUP_CONCAT(DISTINCT artistName)"
					 ", GROUP_CONCAT(DISTINCT albumArtistName)"
					 ", GROUP_CONCAT(DISTINCT discnumber)"
					 " FROM " + m->search_view + " ";
	QString query = select;

	if( !filter.cleared() )
	{
		switch(filter.mode())
		{
			case Library::Filter::Genre:
				query += "WHERE genre LIKE :searchterm AND ";			// track title is like filter
				break;

			case Library::Filter::Filename:
				query += "WHERE filename LIKE :searchterm AND ";			// track title is like filter

				break;

			case Library::Filter::Fulltext:
				query += "WHERE allCissearch LIKE :searchterm AND ";
				break;
		}
	}

	else{
		query += "WHERE ";
	}

	if(!artists.isEmpty())
	{
		QString artist_id_field = m->search_view + "." + m->artist_id_field;
		query += "(" + artist_id_field + " = :artist_id_0 ";

		for(int i=1; i<artists.size(); i++) {
			query += "OR " + artist_id_field + " = :artist_id_"
					+ QString::number(i) + " ";
		}

		query += ") ";
	}

	query += "GROUP BY albumID, albumName ";
	query += _create_order_string(sortorder) + ";";

	q.prepare(query);

	q.bindValue(":searchterm", filter.filtertext());

	for(int i=0; i<artists.size(); i++) {
		q.bindValue(QString(":artist_id_") + QString::number(i), artists[i]);
	}

	return db_fetch_albums(q, result);
}

bool Albums::getAllAlbumsByArtist(int artist, AlbumList& result)
{
	return getAllAlbumsByArtist(artist, result, Library::Filter());
}

bool Albums::getAllAlbumsByArtist(int artist, AlbumList& result, const Library::Filter& filter, Library::SortOrder sortorder)
{
	IDList list;
	list << artist;
	return getAllAlbumsByArtist(list, result, filter, sortorder);
}


bool Albums::getAllAlbumsBySearchString(const Library::Filter& filter, AlbumList& result, Library::SortOrder sortorder)
{
	Query q(this);
	QString select = "SELECT "
					 "  albumID"
					 ", albumName"
					 ", SUM(length) / 1000 AS albumLength"
					 ", albumRating"
					 ", COUNT(DISTINCT trackID) AS trackCount"
					 ", MAX(year) AS albumYear"
					 ", GROUP_CONCAT(DISTINCT artistName)"
					 ", GROUP_CONCAT(DISTINCT albumArtistName)"
					 ", GROUP_CONCAT(DISTINCT discnumber)"
					 " FROM " + m->search_view + " ";
	QString query;
	QString search_field;
	switch(filter.mode())
	{
		case Library::Filter::Genre:
			search_field = "genre";
			break;

		case Library::Filter::Filename:
			search_field = "filename";
			break;

		case Library::Filter::Fulltext:
		default:
			search_field = "allCissearch";
			break;
	}


	if(query.isEmpty()){
		query = select +
					  "WHERE " + search_field + " LIKE :searchterm " +
					  "GROUP BY albumID, albumName ";
	}

	query += _create_order_string(sortorder) + ";";

	q.prepare(query);
	q.bindValue(":searchterm", filter.filtertext());

	return db_fetch_albums(q, result);
}

int Albums::updateAlbum (const Album & album)
{
	Query q(this);

	q.prepare("UPDATE albums "
			  "SET name=:name, "
			  "    cissearch=:cissearch, "
			  "    rating=:rating "
			  "WHERE albumID = :id;");

	QString cissearch = Library::Util::convert_search_string(album.name(), search_mode());

	q.bindValue(":id", album.id);
	q.bindValue(":name", album.name());
	q.bindValue(":cissearch", cissearch);
	q.bindValue(":rating", album.rating);

	if (!q.exec()) {
		q.show_error(QString("Cannot update album ") + album.name());
		return -1;
	}

	return getAlbumID (album.name());
}

void Albums::updateAlbumCissearch()
{
	SearchMode::update_search_mode();

	AlbumList albums;
	getAllAlbums(albums, true);

	module_db().transaction();

	for(const Album& album : albums)
	{
		QString str = "UPDATE albums SET cissearch=:cissearch WHERE albumID=:id;";
		Query q(this);
		QString cis = Library::Util::convert_search_string(album.name(), search_mode());
		q.prepare(str);
		q.bindValue(":cissearch", cis);
		q.bindValue(":id", album.id);

		if(!q.exec()){
			q.show_error("Cannot update album cissearch");
		}
	}

	module_db().commit();
}

int Albums::insertAlbumIntoDatabase (const QString& album)
{
	Query q(this);

	int album_id = getAlbumID(album);
	if(album_id >= 0){
		Album a;
		getAlbumByID(album_id, a, true);
		return updateAlbum(a);
	}

	QString cissearch = Library::Util::convert_search_string(album, search_mode());
	q.prepare("INSERT INTO albums (name, cissearch) values (:album, :cissearch);");
	q.bindValue(":album", album);
	q.bindValue(":cissearch", cissearch);

	if (!q.exec()) {
		q.show_error(QString("Cannot insert album ") + album + " to db");
		return -1;
	}

	return getAlbumID(album);
}

int Albums::insertAlbumIntoDatabase (const Album& album)
{
	if(album.id >= 0){
		return updateAlbum(album);
	}

	Query q(this);
	QString cissearch = Library::Util::convert_search_string(album.name(), search_mode());

	q.prepare("INSERT INTO albums (name, cissearch, rating) values (:name, :cissearch, :rating);");

	q.bindValue(":name", album.name());
	q.bindValue(":cissearch", cissearch);
	q.bindValue(":rating", album.rating);

	if (!q.exec()) {
		q.show_error("SQL: Cannot insert album into database");
		return -1;
	}

	return album.id;
}

void Albums::change_artistid_field(const QString& id, const QString& name)
{
	m->artist_id_field = id;
	m->artist_name_field = name;
}

void Albums::change_track_lookup_field(const QString& search_view)
{
	m->search_view = search_view;
}
