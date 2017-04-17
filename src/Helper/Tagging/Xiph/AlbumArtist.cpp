/* AlbumArtist.cpp */

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



#include "AlbumArtist.h"
#include <QString>

Xiph::AlbumArtistFrame::AlbumArtistFrame(TagLib::Tag* tag) :
	Xiph::XiphFrame<QString>(tag, "ALBUMARTIST") {}

Xiph::AlbumArtistFrame::~AlbumArtistFrame() {}

bool Xiph::AlbumArtistFrame::map_tag_to_model(QString& model)
{
	TagLib::String str;
	bool success = value(str);
	if(success){
		model = cvt_string(str);
	}

	return success;
}

bool Xiph::AlbumArtistFrame::map_model_to_tag(const QString& model)
{
	set_value(model);
    return true;
}
