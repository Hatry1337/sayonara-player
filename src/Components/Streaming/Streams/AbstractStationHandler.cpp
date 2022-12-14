/* AbstractStreamHandler.cpp */

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

#include "AbstractStationHandler.h"

#include "Components/Playlist/Playlist.h"
#include "Interfaces/PlaylistInterface.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Parser/StreamParser.h"
#include "Utils/Settings/Settings.h"
#include "Utils/WebAccess/WebClientFactory.h"

struct AbstractStationHandler::Private
{
	PlaylistCreator* playlistCreator;
	StreamParser* streamParser = nullptr;
	StationPtr parsedStation;

	Private(PlaylistCreator* playlistCreator) :
		playlistCreator(playlistCreator) {}
};

AbstractStationHandler::AbstractStationHandler(PlaylistCreator* playlistCreator, QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>(playlistCreator);
}

AbstractStationHandler::~AbstractStationHandler() = default;

void AbstractStationHandler::createPlaylist(StationPtr station, MetaDataList& tracks)
{
	const auto playlistName = GetSetting(Set::Stream_NewTab)
	                          ? station->name()
	                          : QString {};

	const auto index = m->playlistCreator->createPlaylist(tracks, playlistName);

	auto playlist = m->playlistCreator->playlist(index);
	playlist->changeTrack(0);
}

bool AbstractStationHandler::parseStation(StationPtr station)
{
	if(m->streamParser)
	{
		return false;
	}

	m->parsedStation = station;
	m->streamParser = new StreamParser(std::make_shared<WebClientFactory>(), this);

	connect(m->streamParser, &StreamParser::sigFinished, this, &AbstractStationHandler::parserFinished);
	connect(m->streamParser, &StreamParser::sigUrlCountExceeded, this, &AbstractStationHandler::sigUrlCountExceeded);
	connect(m->streamParser, &StreamParser::sigStopped, this, &AbstractStationHandler::parserStopped);

	m->streamParser->parse(station->name(), station->url());

	return true;
}

void AbstractStationHandler::parserFinished(bool success)
{
	if(!success)
	{
		emit sigError();
		spLog(Log::Warning, this) << "Stream parser finished with error";
	}

	else
	{
		auto tracks = m->streamParser->tracks();
		if(!tracks.isEmpty())
		{
			createPlaylist(m->parsedStation, tracks);
		}

		emit sigDataAvailable();
	}

	sender()->deleteLater(); // m->stream_parser may be nullptr here
	m->streamParser = nullptr;
}

void AbstractStationHandler::stop()
{
	if(m->streamParser && !m->streamParser->isStopped())
	{
		m->streamParser->stop();
	}

	m->streamParser = nullptr;
	emit sigStopped();
}

void AbstractStationHandler::parserStopped()
{
	sender()->deleteLater(); // m->stream_parser may be nullptr here
	m->streamParser = nullptr;

	emit sigStopped();
}

bool AbstractStationHandler::save(StationPtr station)
{
	return addNewStream(station);
}
