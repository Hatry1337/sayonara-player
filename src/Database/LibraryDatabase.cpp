/* LibraryDatabase.cpp */

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

#include "LibraryDatabase.h"
#include "SayonaraQuery.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"

using DB::LibraryDatabase;

struct LibraryDatabase::Private
{
	LibraryId library_id;

	Private(LibraryId library_id) :
		library_id(library_id)
	{}
};


LibraryDatabase::LibraryDatabase(const QString& db_name, DbId database_id, LibraryId library_id) :
	DB::Base(database_id, "", db_name),
	DB::Albums(db(), db_id(), library_id),
	DB::Artists(db(), db_id(), library_id),
	DB::Tracks(db(), db_id(), library_id)
{
	m = Pimpl::make<Private>(library_id);

	bool show_album_artists = false;

	AbstrSetting* s = Settings::instance()->setting(SettingKey::Lib_ShowAlbumArtists);
	QString db_key = s->db_key();

	Query q(db());
	QString querytext = "SELECT value FROM settings WHERE key = '" + db_key + "';";

	q.prepare(querytext);
	if(q.exec()){
		if(q.next()){
			QVariant var = q.value("value");
			show_album_artists = var.toBool();
		}
	}

	if(show_album_artists){
		change_artistid_field(LibraryDatabase::ArtistIDField::AlbumArtistID);
	}

	else{
		change_artistid_field(LibraryDatabase::ArtistIDField::ArtistID);
	}

	apply_fixes();
}

LibraryDatabase::~LibraryDatabase() {}

void LibraryDatabase::change_artistid_field(LibraryDatabase::ArtistIDField field)
{
	QString id, name;
	if(field == LibraryDatabase::ArtistIDField::AlbumArtistID){
		id = "albumArtistID";
		name = "albumArtistName";
	}

	else{
		id = "artistID";
		name = "artistName";
	}

	DB::Albums::change_artistid_field(id, name);
	DB::Artists::change_artistid_field(id, name);
	DB::Tracks::change_artistid_field(id, name);
}

void LibraryDatabase::clear()
{
	DB::Tracks::deleteAllTracks();
}

LibraryId LibraryDatabase::library_id() const
{
	return m->library_id;
}

bool LibraryDatabase::apply_fixes()
{
	return true;
}


bool DB::LibraryDatabase::store_metadata(const MetaDataList& v_md)
{
	bool success = true;

	if(v_md.isEmpty()) {
		return success;
	}

	db().transaction();

	AlbumList albums;
	ArtistList artists;
	QHash<QString, Album> album_map;
	QHash<QString, Artist> artist_map;

	DB::Albums::getAllAlbums(albums, true);
	DB::Artists::getAllArtists(artists, true);

	for(const Album& album : albums){
		album_map[album.name()] = album;
	}

	for(const Artist& artist : artists){
		artist_map[artist.name()] = artist;
	}

	albums.clear();
	artists.clear();

	for(MetaData md : v_md)
	{
		int artist_id, album_id, album_artist_id;
		//first check if we know the artist and its id
		Album album = album_map[md.album()];
		if(album.id < 0) {
			album_id = DB::Albums::insertAlbumIntoDatabase(md.album());
			album.id = album_id;
			album_map[md.album()] = album;
		}

		else{
			album_id = album.id;
		}

		Artist artist = artist_map[md.artist()];
		if (artist.id < 0) {
			artist_id = DB::Artists::insertArtistIntoDatabase(md.artist());
			artist.id = artist_id;
			artist_map[md.artist()] = artist;
		}

		else{
			artist_id = artist.id;
		}

		if(md.album_artist_id() == -1){
			md.set_album_artist_id(artist_id);
		}

		if(md.album_artist().isEmpty()){
			md.set_album_artist(md.artist());
		}

		Artist album_artist = artist_map[md.album_artist()];
		if (album_artist.id < 0) {
			if(md.album_artist().isEmpty()){
				album_artist_id = -1;
			}

			else{
				album_artist_id = DB::Artists::insertArtistIntoDatabase(md.album_artist());
				album_artist.id = album_artist_id;
				artist_map[md.album_artist()] = album_artist;
			}
		}

		else{
			album_artist_id = album_artist.id;
		}

		md.album_id = album_id;
		md.artist_id = artist_id;
		md.library_id = m->library_id;

		if(album_id == -1 || artist_id == -1 || md.library_id == -1){
			sp_log(Log::Warning) << "Cannot insert artist or album of " << md.filepath();
			continue;
		}

		DB::Tracks::insertTrackIntoDatabase(md, artist_id, album_id, album_artist_id);
	}

	success = db().commit();

	return success;
}
