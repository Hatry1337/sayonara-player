/* Broadcaster.h */

/* Copyright (C) 2011-2020 Lucio Carreras
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

#ifndef BROADCASTER_H
#define BROADCASTER_H

#include "Utils/Pimpl.h"
#include <gst/gst.h>

namespace PipelineExtensions
{
	class BroadcastDataReceiver
	{
		public:
			virtual void set_raw_data(const QByteArray& data)=0;
	};
}

/**
 * @brief The Broadcaster class
 * @ingroup EngineInterfaces
 */
class Broadcaster
{
	PIMPL(Broadcaster)

	public:
		Broadcaster(PipelineExtensions::BroadcastDataReceiver* broadcast_data_receiver, GstElement* pipeline, GstElement* tee);
		virtual ~Broadcaster();

		bool init();
		bool set_enabled(bool b);
};

#endif // BROADCASTER_H
