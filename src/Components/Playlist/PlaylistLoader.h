/* PlaylistLoader.h */

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

#ifndef PLAYLISTLOADER_H
#define PLAYLISTLOADER_H

#include "Helper/Settings/SayonaraClass.h"
#include "Helper/Playlist/CustomPlaylistFwd.h"

#include <QObject>

class PlaylistDBWrapper;

/**
 * @brief The PlaylistLoader class
 * @ingroup Playlists
 */
class PlaylistLoader :
		public QObject,
		protected SayonaraClass
{
	Q_OBJECT

private:

	CustomPlaylists			_playlists;

	PlaylistDBWrapper*		_playlist_db_connector=nullptr;

	int						_last_playlist_idx;
	int						_last_track_idx;


public:
	explicit PlaylistLoader(QObject* parent=nullptr);
	virtual ~PlaylistLoader();


	CustomPlaylists			get_playlists() const;
	int						get_last_playlist_idx() const;
	int						get_last_track_idx() const;
	int						create_playlists();

};

#endif // PLAYLISTLOADER_H
