/* DiscnumberFrame.h */

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



#ifndef MP4_DISCNUMBERFRAME_H
#define MP4_DISCNUMBERFRAME_H

#include "Helper/Tagging/Models/Discnumber.h"
#include "Helper/Tagging/MP4/MP4Frame.h"

/**
 * @ingroup Tagging
 */
namespace MP4
{
	/**
	 * @brief The DiscnumberFrame class
	 * @ingroup Xiph
	 */
	class DiscnumberFrame :
		public MP4::MP4Frame<Models::Discnumber>
	{
	public:
		DiscnumberFrame(TagLib::Tag* tag);
		~DiscnumberFrame();

	protected:
		bool map_tag_to_model(Models::Discnumber& model);
		bool map_model_to_tag(const Models::Discnumber& model);
	};
}

#endif // MP4_DISCNUMBERXIPHFRAME_H
