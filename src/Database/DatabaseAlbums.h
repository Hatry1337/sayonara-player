/* DatabaseAlbums.h */

/* Copyright (C) 2011-2016  Lucio Carreras
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



#ifndef DATABASEALBUMS_H
#define DATABASEALBUMS_H

#include "Database/DatabaseSearchMode.h"
#include "Database/DatabaseModule.h"
#include "Helper/Library/Sortorder.h"
#include "Helper/typedefs.h"

namespace Library {class Filter;}
class Album;
class AlbumList;
class DatabaseAlbums :
		private DatabaseModule,
		private DatabaseSearchMode
{

private:
	QString _fetch_query;

	QString _create_order_string(Library::SortOrder order);

protected:
	void set_album_fetch_query(const QString& query);

public:
	DatabaseAlbums(QSqlDatabase db, quint8 db_id);

	virtual bool db_fetch_albums(SayonaraQuery& q, AlbumList& result);

	virtual int getAlbumID (const QString& album);

	virtual bool getAlbumByID(const int& id, Album& album);

	virtual bool getAllAlbums(AlbumList& result, Library::SortOrder sortorder=Library::SortOrder::AlbumNameAsc);

	virtual bool getAllAlbumsByArtist(int artist, AlbumList& result);
	virtual bool getAllAlbumsByArtist(int artist, AlbumList& result, const Library::Filter& filter, Library::SortOrder sortorder = Library::SortOrder::AlbumNameAsc);
	virtual bool getAllAlbumsByArtist(IDList artists, AlbumList& result);
	virtual bool getAllAlbumsByArtist(IDList artists, AlbumList& result, const Library::Filter& filter, Library::SortOrder sortorder = Library::SortOrder::AlbumNameAsc);

	virtual bool getAllAlbumsBySearchString(const Library::Filter& filter, AlbumList& result, Library::SortOrder sortorder = Library::SortOrder::AlbumNameAsc);

	virtual int insertAlbumIntoDatabase (const QString & album);
	virtual int insertAlbumIntoDatabase (const Album& album);

	virtual int updateAlbum(const Album& album);

	virtual void updateAlbumCissearch();
};

#endif // DATABASEALBUMS_H
