/* CustomPlaylist.h */

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


#ifndef CUSTOMPLAYLIST_H
#define CUSTOMPLAYLIST_H

#include "CustomPlaylistSkeleton.h"
#include "Helper/MetaData/MetaDataList.h"

/**
 * @brief The CustomPlaylist class
 * @ingroup PlaylistHelper
 */
class CustomPlaylist :
		public CustomPlaylistSkeleton,
		public MetaDataList
{

public:

	bool valid() const;

	int num_tracks() const override;

	CustomPlaylist();
	explicit CustomPlaylist(const CustomPlaylistSkeleton& skeleton);
	virtual ~CustomPlaylist();

private:
	void set_num_tracks(int num_tracks) override;
};


#endif // CUSTOMPLAYLIST_H
