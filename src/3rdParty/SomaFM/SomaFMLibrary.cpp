/* SomaFMLibrary.cpp */

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

/* SomaFMLibrary.cpp */

#include "SomaFMLibrary.h"
#include "SomaFMStation.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/WebAccess/AsyncWebAccess.h"
#include "Utils/Parser/StreamParser.h"
#include "Utils/globals.h"
#include "Utils/MetaData/MetaDataList.h"

#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Covers/CoverLocation.h"

#include <QMap>
#include <QSettings>
#include <algorithm>

struct SomaFM::Library::Private
{
	QMap<QString, SomaFM::Station> 	station_map;
	QString 						requested_station;
	QSettings*						qsettings=nullptr;
};

SomaFM::Library::Library(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();
	QString path = Util::sayonara_path("somafm.ini");

	m->qsettings = new QSettings(path, QSettings::IniFormat, this);
}

SomaFM::Library::~Library()
{
	m->qsettings->deleteLater();
}


void SomaFM::Library::search_stations()
{
	emit sig_loading_started();

	AsyncWebAccess* awa = new AsyncWebAccess(this);
	connect(awa, &AsyncWebAccess::sig_finished, this, &SomaFM::Library::soma_website_fetched);

	awa->run("https://somafm.com/listen/");
}


SomaFM::Station SomaFM::Library::station(const QString& name)
{
	m->requested_station = name;
	return m->station_map[name];
}


void SomaFM::Library::soma_website_fetched()
{
	AsyncWebAccess* awa = static_cast<AsyncWebAccess*>(sender());
	QList<SomaFM::Station> stations;

	if(awa->status() != AsyncWebAccess::Status::GotData)
	{
		awa->deleteLater();

		emit sig_stations_loaded(stations);
		emit sig_loading_finished();

		return;
	}

	QString content = QString::fromUtf8(awa->data());
	QStringList station_contents = content.split("<li");


	for(const QString& station_content : station_contents)
	{
		SomaFM::Station station(station_content);
		if(!station.is_valid()){
			continue;
		}

		QString station_name = station.name();

		bool loved = m->qsettings->value(station_name, false).toBool();

		station.set_loved( loved );

		m->station_map[station_name] = station;
		stations << station;
	}

	sort_stations(stations);

	emit sig_stations_loaded(stations);
	emit sig_loading_finished();

	awa->deleteLater();
}

void SomaFM::Library::create_playlist_from_station(int row)
{
	Q_UNUSED(row)

	emit sig_loading_started();

	SomaFM::Station station = m->station_map[m->requested_station];
	StreamParser* parser = new StreamParser(station.name(), this);
	connect(parser, &StreamParser::sig_finished, this, &SomaFM::Library::soma_station_playlists_fetched);
	parser->parse_streams(station.urls());
}

void SomaFM::Library::soma_station_playlists_fetched(bool success)
{
	StreamParser* parser = dynamic_cast<StreamParser*>(sender());

	if(!success){
		parser->deleteLater();
		emit sig_loading_finished();
		return;
	}

	MetaDataList v_md  = parser->metadata();
	SomaFM::Station station = m->station_map[m->requested_station];
	QString cover_url;
	Cover::Location cl = station.cover_location();
	if(cl.has_search_urls()){
		cover_url = cl.search_urls().first();
	}

	for(auto it = v_md.begin(); it != v_md.end(); it++){
		it->set_cover_download_url(cover_url);
	}

	station.set_metadata(v_md);

	m->station_map[m->requested_station] = station;

	Playlist::Handler* plh = Playlist::Handler::instance();
	plh->create_playlist(v_md,
						 station.name(),
						 true,
						 Playlist::Type::Stream);

	parser->deleteLater();
	emit sig_loading_finished();
}


bool SomaFM::Library::create_playlist_from_playlist(int idx)
{
	SomaFM::Station station = m->station_map[m->requested_station];
	QStringList urls = station.urls();

	if( !between(idx, urls)) {
		return false;
	}

	emit sig_loading_started();

	QString url = urls[idx];
	StreamParser* stream_parser = new StreamParser(station.name(), this);
	connect(stream_parser, &StreamParser::sig_finished, this, &SomaFM::Library::soma_playlist_content_fetched);

	stream_parser->parse_stream(url);

	return true;
}


void SomaFM::Library::soma_playlist_content_fetched(bool success)
{
	StreamParser* parser = static_cast<StreamParser*>(sender());

	if(!success){
		parser->deleteLater();
		emit sig_loading_finished();
		return;
	}

	MetaDataList v_md = parser->metadata();

	SomaFM::Station station = m->station_map[m->requested_station];
	Cover::Location cl = station.cover_location();
	QString cover_url;
	if(cl.has_search_urls()){
		cover_url = cl.search_urls().first();
	}

	for(auto it = v_md.begin(); it != v_md.end(); it++){
		it->set_cover_download_url(cover_url);
	}

	station.set_metadata(v_md);

	m->station_map[m->requested_station] = station;

	Playlist::Handler* plh = Playlist::Handler::instance();
	plh->create_playlist(v_md,
						 station.name(),
						 true,
						 Playlist::Type::Stream);

	parser->deleteLater();

	emit sig_loading_finished();
}


void SomaFM::Library::set_station_loved(const QString& station_name, bool loved)
{
	m->station_map[station_name].set_loved(loved);
	m->qsettings->setValue(station_name, loved);

	QList<SomaFM::Station> stations;
	for(const QString& key : m->station_map.keys()){
		if(key.isEmpty()){
			continue;
		}

		stations << m->station_map[key];
	}

	sort_stations(stations);
	emit sig_stations_loaded(stations);
}


void SomaFM::Library::sort_stations(QList<SomaFM::Station>& stations)
{
	auto lambda = [](const SomaFM::Station& s1, const SomaFM::Station& s2){
		if(s1.is_loved() && !s2.is_loved()){
			return true;
		}

		else if(!s1.is_loved() && s2.is_loved()){
			return false;
		}

		return s1.name() < s2.name();
	};

	std::sort(stations.begin(), stations.end(), lambda);
}
