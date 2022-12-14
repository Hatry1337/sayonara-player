/* StreamHandlerStreams.cpp */

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

#include "StreamHandler.h"
#include "Database/Connector.h"
#include "Database/Streams.h"

#include "Utils/Algorithm.h"
#include "Utils/Streams/Station.h"

StreamHandler::StreamHandler(PlaylistCreator* playlistCreator, QObject* parent) :
	AbstractStationHandler(playlistCreator, parent) {}

StreamHandler::~StreamHandler() = default;

bool StreamHandler::getAllStreams(QList<StationPtr>& stations)
{
	auto* db = DB::Connector::instance()->streamConnector();

	auto streams = QList<Stream> {};
	const auto b = db->getAllStreams(streams);

	Util::Algorithm::transform(streams, stations, [&](const auto& stream) {
		return createStreamInstance(stream.name(), stream.url());
	});

	return b;
}

bool StreamHandler::addNewStream(StationPtr station)
{
	auto* db = DB::Connector::instance()->streamConnector();

	const auto* stream = dynamic_cast<Stream*>(station.get());
	return (stream != nullptr)
	       ? db->addStream(*stream)
	       : false;
}

bool StreamHandler::deleteStream(const QString& name)
{
	auto* db = DB::Connector::instance()->streamConnector();

	return db->deleteStream(name);
}

bool StreamHandler::update(const QString& name, StationPtr station)
{
	auto* db = DB::Connector::instance()->streamConnector();
	const auto* stream = dynamic_cast<Stream*>(station.get());

	return (stream != nullptr)
	       ? db->updateStream(name, *stream)
	       : false;
}

StationPtr StreamHandler::createStreamInstance(const QString& name, const QString& url) const
{
	return std::make_shared<Stream>(name, url);
}

StationPtr StreamHandler::station(const QString& name)
{
	auto* db = DB::Connector::instance()->streamConnector();
	const auto stream = db->getStream(name);

	return !stream.name().isEmpty()
	       ? std::make_shared<Stream>(stream)
	       : nullptr;
}
