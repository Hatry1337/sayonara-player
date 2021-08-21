/* Lyrics.h */

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

#ifndef SAYONARA_ID3V2_LYRICS_FRAME_H
#define SAYONARA_ID3V2_LYRICS_FRAME_H

#include "ID3v2Frame.h"

#include <QString>

#include <taglib/unsynchronizedlyricsframe.h>

#include <optional>

namespace ID3v2
{
	class LyricsFrame :
		public ID3v2Frame<QString, TagLib::ID3v2::UnsynchronizedLyricsFrame>
	{
		public:
			explicit LyricsFrame(TagLib::ID3v2::Tag* tag);
			~LyricsFrame() override;

		protected:
			std::optional<QString> mapFrameToData(const TagLib::ID3v2::UnsynchronizedLyricsFrame* frame) const override;
			void mapDataToFrame(const QString& data, TagLib::ID3v2::UnsynchronizedLyricsFrame* frame) override;

			TagLib::ID3v2::Frame* createId3v2Frame() override;
	};
}

#endif // SAYONARA_ID3V2_LYRICS_FRAME_H
