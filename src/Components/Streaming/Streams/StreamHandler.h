/* StreamHandlerStreams.h */

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

#ifndef STREAMHANDLERSTREAMS_H
#define STREAMHANDLERSTREAMS_H

#include "AbstractStationHandler.h"

class StreamHandler :
	public AbstractStationHandler
{
	public:
		explicit StreamHandler(PlaylistCreator* playlistCreator, QObject* parent = nullptr);
		~StreamHandler() override;

		bool getAllStreams(QList<StationPtr>& stations) override;
		bool addNewStream(StationPtr station) override;
		bool deleteStream(const QString& stationName) override;
		bool update(const QString& stationName, StationPtr station) override;

		StationPtr createStreamInstance(const QString& name, const QString& url) const override;
		StationPtr station(const QString& name) override;
};

#endif // STREAMHANDLERSTREAMS_H
