/* AlbumArtist.h */

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

#ifndef ALBUMARTIST_H
#define ALBUMARTIST_H

#include "AbstractFrame.h"

namespace ID3v2Frame
{
	/**
	 * @brief The AlbumArtistFrame class
	 * @ingroup ID3v2
	 */
	class AlbumArtistFrame :
			public AbstractFrame<QString, TagLib::ID3v2::TextIdentificationFrame>
	{
		public:
			explicit AlbumArtistFrame(const TagLib::FileRef& f);
			virtual ~AlbumArtistFrame();

		protected:
			TagLib::ID3v2::Frame* create_id3v2_frame() override;

			void map_model_to_frame() override;
			void map_frame_to_model() override;
	};
}

#endif // ALBUMARTIST_H
