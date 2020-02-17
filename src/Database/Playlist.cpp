/* DatabasePlaylist.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#include "Database/Query.h"
#include "Database/Playlist.h"

#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Utils.h"

using DB::Query;

DB::Playlist::Playlist(const QString& connection_name, DbId databaseId) :
	Module(connection_name, databaseId) {}

DB::Playlist::~Playlist() {}

bool DB::Playlist::getAllPlaylistSkeletons(CustomPlaylistSkeletons& skeletons, ::Playlist::StoreType type, ::Playlist::SortOrder sortorder)
{
	skeletons.clear();

	QString sortorder_str;
	switch(sortorder){
		case ::Playlist::SortOrder::IDAsc:
			sortorder_str = " ORDER BY playlists.playlistID ASC ";
			break;
		case ::Playlist::SortOrder::IDDesc:
			sortorder_str = " ORDER BY playlists.playlistID DESC ";
			break;
		case ::Playlist::SortOrder::NameAsc:
			sortorder_str = " ORDER BY playlists.playlist ASC ";
			break;
		case ::Playlist::SortOrder::NameDesc:
			sortorder_str = " ORDER BY playlists.playlist DESC ";
			break;
		default:
			break;
	}

	QString type_clause;
	switch(type)
	{
		case ::Playlist::StoreType::OnlyTemporary:
			type_clause = " WHERE playlists.temporary = 1 ";
			break;
		case ::Playlist::StoreType::OnlyPermanent:
			type_clause = " WHERE playlists.temporary = 0 ";
			break;
		default:
			break;
	}

	Query q = runQuery
	(
		"SELECT "
		"playlists.playlistID, "
		"playlists.playlist, "
		"playlists.temporary, "
		"COUNT(playlisttotracks.trackID) "
		"FROM playlists LEFT OUTER JOIN playlisttotracks "
		"ON playlists.playlistID = playlisttotracks.playlistID "
		+ type_clause +
		"GROUP BY playlists.playlistID " +
		sortorder_str + ";",

		"Cannot fetch all playlists"
	);

	if(q.hasError()){
		return false;
	}

	while(q.next())
	{
		CustomPlaylistSkeleton skeleton;
		if(q.value(0).isNull()){
			continue;
		}

		skeleton.setId(q.value(0).toInt());
		skeleton.setName(q.value(1).toString());

		bool temporary = (q.value(2) == 1);
		skeleton.setTemporary(temporary);
		skeleton.setTrackCount(q.value(3).toInt());

		skeletons << skeleton;
	}

	return true;
}

bool DB::Playlist::getPlaylistSkeletonById(CustomPlaylistSkeleton& skeleton)
{
	if(skeleton.id() < 0){
		spLog(Log::Warning, this) << "Cannot fetch playlist -1";
		return false;
	}

	Query q = runQuery
	(
		"SELECT "
		"playlists.playlistID, "
		"playlists.playlist, "
		"playlists.temporary, "
		"COUNT(playlisttotracks.trackID) "
		"FROM playlists LEFT OUTER JOIN playlisttotracks "
		"ON playlists.playlistID = playlisttotracks.playlistID "
		"WHERE playlists.playlistid = :playlist_id "
		"GROUP BY playlists.playlistID;",

		{{":playlist_id", skeleton.id()}},
		"Cannot fetch all playlists"
	);

	if(q.hasError())
	{
		return false;
	}

	if(q.next())
	{
		skeleton.setId(q.value(0).toInt());
		skeleton.setName(q.value(1).toString());

		bool temporary = (q.value(2) == 1);
		skeleton.setTemporary(temporary);
		skeleton.setTrackCount(q.value(3).toInt());

		return true;
	}

	return false;
}

bool DB::Playlist::getPlaylistById(CustomPlaylist& pl)
{
	if(!getPlaylistSkeletonById(pl))
	{
		spLog(Log::Warning, this) << "Get playlist by id: cannot fetch skeleton id " << pl.id();
		return false;
	}

	pl.clear();

	QStringList fields
	{
		"tracks.trackID			AS trackID",		// 0
		"tracks.title			AS title",			// 1
		"tracks.length			AS length",			// 2
		"tracks.year			AS year",			// 3
		"tracks.bitrate			AS bitrate",		// 4
		"tracks.filename		AS filename",		// 5
		"tracks.track			AS trackNum",		// 6
		"albums.albumID			AS albumID",		// 7
		"artists.artistID		AS artistID",		// 8
		"albums.name			AS albumName",		// 9
		"artists.name			AS artistName",		// 10
		"tracks.genre			AS genrename",		// 11
		"tracks.filesize		AS filesize",		// 12
		"tracks.discnumber		AS discnumber",		// 13
		"tracks.rating			AS rating",			// 14
		"playlistToTracks.filepath AS filepath",	// 15
		"playlistToTracks.db_id AS databaseId",			// 16
		"tracks.libraryID		AS libraryId",		// 17
		"tracks.createdate		AS createdate",		// 18
		"tracks.modifydate		AS modifydate"		// 19
	};

	Query q = runQuery
	(
		"SELECT "
		+ fields.join(", ") + " " +
		"FROM tracks, albums, artists, playlists, playlisttotracks "
		"WHERE playlists.playlistID = :playlist_id "
		"AND playlists.playlistID = playlistToTracks.playlistID "
		"AND playlistToTracks.trackID = tracks.trackID "
		"AND tracks.albumID = albums.albumID "
		"AND tracks.artistID = artists.artistID "
		"ORDER BY playlistToTracks.position ASC; ",

		{{":playlist_id", pl.id()}},
		QString("Cannot get tracks for playlist %1").arg(pl.id())
	);


	if(!q.hasError())
	{
		while (q.next())
		{
			MetaData data;

			data.setId(q.value(0).toInt());
			data.setTitle(q.value(1).toString());
			data.setDurationMs(q.value(2).toInt());
			data.setYear(q.value(3).value<Year>());
			data.setBitrate(q.value(4).value<Bitrate>());
			data.setFilepath(q.value(5).toString());
			data.setTrackNumber(q.value(6).value<TrackNum>());
			data.setAlbumId(q.value(7).toInt());
			data.setArtistId(q.value(8).toInt());
			data.setAlbum(q.value(9).toString().trimmed());
			data.setArtist(q.value(10).toString().trimmed());
			QStringList genres = q.value(11).toString().split(",");
			data.setGenres(genres);
			data.setFilesize(q.value(12).value<Filesize>());
			data.setDiscnumber(q.value(13).value<Disc>());
			data.setRating(q.value(14).value<Rating>());
			data.setLibraryid(q.value(17).value<LibraryId>());
			data.setCreatedDate(q.value(18).value<uint64_t>());
			data.setModifiedDate(q.value(19).value<uint64_t>());
			data.setExtern(false);
			data.setDatabaseId(databaseId());

			if(q.value(16).toInt() == 0 || q.value(16).isNull()){
				pl.push_back(data);
			}
		}
	}

	// non database playlists
	Query q2 = runQuery
	(
		"SELECT "
		"playlisttotracks.filepath AS filepath, "
		"playlisttotracks.position AS position "
		"FROM playlists, playlisttotracks "
		"WHERE playlists.playlistID = :playlist_id "
		"AND playlists.playlistID =  playlistToTracks.playlistID "
		"AND playlistToTracks.trackID <= 0 "
		"ORDER BY playlistToTracks.position ASC;",

		{{":playlist_id", pl.id()}},
		QString("Playlist by id: Cannot fetch playlist %1").arg(pl.id())
	);

	if(q2.hasError()) {
		return false;
	}

	while (q2.next())
	{
		int position = q2.value(1).toInt();

		QString filepath = q2.value(0).toString();
		MetaData data(filepath);
		data.setId(-1);
		data.setExtern(true);
		data.setTitle(filepath);
		data.setArtist(filepath);
		data.setDatabaseId(databaseId());

		for(int row=0; row<=pl.count(); row++)
		{
			if( row >= position)
			{
				pl.insertTrack(data, row);
				break;
			}
		}
	}

	return true;
}

// negative, if error
// nonnegative else
int DB::Playlist::getPlaylistIdByName(const QString& name)
{
	Query q = runQuery
	(
		"SELECT playlistid FROM playlists WHERE playlist = :playlist_name;",
		{
			{":playlist_name", Util::convertNotNull(name)}
		},
		QString("Playlist by name: Cannot fetch playlist %1").arg(name)
	);

	if(q.hasError()) {
		return -1;
	}

	else
	{
		if(q.next()) {
		  return q.value(0).toInt();
		}

		return -1;
	}
}


bool DB::Playlist::insertTrackIntoPlaylist(const MetaData& md, int playlist_id, int pos)
{
	if(md.isDisabled()) {
		return false;
	}

	Query q = insert("playlisttotracks",
	{
		{"trackid", md.id()},
		{"playlistid", playlist_id},
		{"position", pos},
		{"filepath", Util::convertNotNull(md.filepath())},
		{"db_id", md.databaseId()}
	}, "Cannot insert track into playlist");

	return (!q.hasError());
}


// returns id if everything ok
// negative otherwise
int DB::Playlist::createPlaylist(QString playlist_name, bool temporary)
{
	Query q	= insert("playlists",
	{
		{"playlist", Util::convertNotNull(playlist_name)},
		{"temporary", (temporary == true) ? 1 : 0}
	}, "Cannot create playlist");

	if(q.hasError()){
		return false;
	}

	return q.lastInsertId().toInt();
}


bool DB::Playlist::renamePlaylist(int id, const QString& new_name)
{
	Query q = update("playlists",
		{{"playlist", Util::convertNotNull(new_name)}},
		{"playlistId", id}
	, "Cannot rename playlist");

	return (!q.hasError());
}


bool DB::Playlist::storePlaylist(const MetaDataList& vec_md, QString playlist_name, bool temporary)
{
	int playlist_id;

	if(playlist_name.isEmpty()){
		return false;
	}

	if(playlist_name.isEmpty()){
		spLog(Log::Warning, this) << "Try to save empty playlist";
		return false;
	}

	playlist_id = getPlaylistIdByName(playlist_name);
	if(playlist_id >= 0) {
		emptyPlaylist(playlist_id);
	}

	else {
		playlist_id = createPlaylist(playlist_name, temporary);
		if( playlist_id < 0) {
			return false;
		}
	}

	// fill playlist
	for(int i=0; i<vec_md.count(); i++)
	{
		bool success = insertTrackIntoPlaylist(vec_md[i], playlist_id, i);

		if( !success ) {
			return false;
		}
	}

	return true;
}


bool DB::Playlist::storePlaylist(const MetaDataList& vec_md, int playlist_id, bool temporary)
{
	CustomPlaylist pl;
	pl.setId(playlist_id);

	bool success = getPlaylistById(pl);
	if(!success){
		spLog(Log::Warning, this) << "Store: Cannot fetch playlist: " << pl.id();
		return false;
	}

	if(pl.name().isEmpty()){
		return false;
	}

	if( playlist_id < 0) {
		playlist_id = createPlaylist(pl.name(), temporary);
	}

	else{
		emptyPlaylist(playlist_id);
	}

	// fill playlist
	for(int i=0; i<vec_md.count(); i++)
	{
		bool success = insertTrackIntoPlaylist(vec_md[i], playlist_id, i);

		if( !success ) {
			return false;
		}
	}

	return true;
}

bool DB::Playlist::emptyPlaylist(int playlist_id)
{
	Query q(this);
	QString querytext = QString("DELETE FROM playlistToTracks WHERE playlistID = :playlist_id;");
	q.prepare(querytext);
	q.bindValue(":playlist_id", playlist_id);

	if(!q.exec()) {
		q.showError("DB: Playlist cannot be cleared");
		return false;
	}

	return true;
}

bool DB::Playlist::deletePlaylist(int playlist_id)
{
	emptyPlaylist(playlist_id);

	Query q(this);
	QString querytext = QString("DELETE FROM playlists WHERE playlistID = :playlist_id;");

	q.prepare(querytext);
	q.bindValue(":playlist_id", playlist_id);

	if(!q.exec()){
		q.showError(QString("Cannot delete playlist ") + QString::number(playlist_id));
		return false;
	}

	return true;
}
