/* SmartPlaylistByListeningDate.cpp */
/*
 * Copyright (C) 2011-2022 Michael Lugmair
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

#include "SmartPlaylistByListeningDate.h"
#include "DateConverter.h"
#include "TimeSpanConverter.h"

#include "Database/Connector.h"
#include "Database/Session.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"

#include <QDate>
#include <QDateTime>
#include <QList>
#include <QMap>

namespace
{
	constexpr const auto DaysPerYear = 365;
	constexpr const auto DaysPerMonth = 30;
	constexpr const auto MaxYears = 1;
	constexpr const auto MaxMonths = 11;
	constexpr const auto MaximumTimeSpan = DaysPerYear * MaxYears +
	                                       DaysPerMonth * MaxMonths +
	                                       DaysPerMonth - 1;

	QDate calculateRelativeDate(const int offset)
	{
		const auto today = QDate::currentDate();
		return today.addDays(-offset);
	}
}

SmartPlaylistByListeningDate::SmartPlaylistByListeningDate(
	const int id, const int from, const int to) :
	SmartPlaylist(id, from, to) {}

SmartPlaylistByListeningDate::~SmartPlaylistByListeningDate() = default;

int SmartPlaylistByListeningDate::minimumValue() const { return 0; }

int SmartPlaylistByListeningDate::maximumValue() const { return MaximumTimeSpan; }

MetaDataList SmartPlaylistByListeningDate::filterTracks(MetaDataList tracks)
{
	const auto dateFrom = calculateRelativeDate(from());
	const auto dateTo = calculateRelativeDate(to());

	auto* dbConnector = DB::Connector::instance();
	auto* dbSession = dbConnector->sessionConnector();

	auto entryListMap = dbSession->getSessions(
		dateTo.startOfDay(),
		dateFrom.endOfDay());

	tracks.clear();

	auto processedFilepaths = Util::Set<QString> {};
	for(auto& entryList: entryListMap)
	{
		for(auto& entry: entryList)
		{
			const auto filepath = entry.track.filepath();
			if(processedFilepaths.contains(filepath))
			{
				continue;
			}

			if(!Util::File::isWWW(filepath) && Util::File::exists(filepath))
			{
				processedFilepaths << filepath;
				tracks.push_back(std::move(entry.track));
			}
		}
	}

	return tracks;
}

QString SmartPlaylistByListeningDate::classType() const
{
	return SmartPlaylistByListeningDate::ClassType;
}

QString SmartPlaylistByListeningDate::displayClassType() const
{
	return QObject::tr("Days since last play time");
}

QString SmartPlaylistByListeningDate::name() const
{
	const auto sc = stringConverter();
	if(from() == to())
	{
		return QObject::tr("Last played: %1 ago").arg(sc->intToString(from()));
	}

	if(from() == 0)
	{
		return QObject::tr("Last played: Less than %1 ago").arg(sc->intToString(to()));
	}

	if(to() == maximumValue())
	{
		return QObject::tr("Last played: More than %1 ago").arg(sc->intToString(from()));
	}

	return QObject::tr("Last played: Between %1 and %2 ago")
		.arg(sc->intToString(from()))
		.arg(sc->intToString(to()));
}

SmartPlaylists::Type SmartPlaylistByListeningDate::type() const { return SmartPlaylists::Type::LastPlayed; }

SmartPlaylists::StringConverterPtr SmartPlaylistByListeningDate::createConverter() const
{
	return std::make_shared<SmartPlaylists::TimeSpanConverter>();
}

bool SmartPlaylistByListeningDate::canFetchTracks() const { return true; }
