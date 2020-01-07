/* SomaFMStation.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

/* SomaFMStation.cpp */

#include "SomaFMStation.h"

#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"

#include "Components/Covers/CoverLocation.h"

#include <QMap>
#include <QStringList>
#include <QUrl>

struct SomaFM::Station::Private
{
	QString			content;
	QString			station_name;
	QMap<QString, SomaFM::Station::UrlType> urls;
	QString			description;
    Cover::Location	cover;
	MetaDataList	v_md;
	bool			loved;
	QMap<QString, QString> image_low_high_map;

	Private()
	{
		image_low_high_map = QMap<QString, QString>
		{
			{"blender120.png",		"beatblender-400"},
			{"1023brc.jpg",			"brfm-400"},
			{"gsclassic120.jpg",	"gsclassic400"},
			{"illstreet.jpg",		"illstreet-400"},
			{"indychick.jpg",		"indiepop-400"},
			{"seventies120.jpg",	"seventies400"},
			{"lush-x120.jpg",		"lush-400"},
			{"sss.jpg",				"spacestation-400"},
			{"sog120.jpg",			"suburbsofgoa-400"}
		};
	}

	Private(const Private& other) :
		CASSIGN(content),
		CASSIGN(station_name),
		CASSIGN(urls),
		CASSIGN(description),
		CASSIGN(cover),
		CASSIGN(v_md),
		CASSIGN(loved),
		CASSIGN(image_low_high_map)
	{}

	Private& operator=(const Private& other)
	{
		ASSIGN(content);
		ASSIGN(station_name);
		ASSIGN(urls);
		ASSIGN(description);
		ASSIGN(cover);
		ASSIGN(v_md);
		ASSIGN(loved);
		ASSIGN(image_low_high_map);

		return *this;
	}

	QString complete_url(const QString& url)
	{
		if(url.startsWith("/")){
			return QString("https://somafm.com") + url;
		}

		return url;
	}

	void parse_station_name()
	{
		QString pattern("<h3>(.*).*</h3>");
		QRegExp re(pattern);
		re.setMinimal(true);

		int idx = re.indexIn(content);
		if(idx > 0){
			station_name = Util::cvt_str_to_first_upper(re.cap(1)).toLocal8Bit();
		}
	}

	void parse_urls()
	{
		QString mp3_pattern("<nobr>\\s*MP3:\\s*<a\\s+href=\"(.*)\"");
		QString aac_pattern("<nobr>\\s*AAC:\\s*<a\\s+href=\"(.*)\"");
		QRegExp re_mp3(mp3_pattern);
		QRegExp re_aac(aac_pattern);

		re_mp3.setMinimal(true);
		re_aac.setMinimal(true);

		int idx=-1;
		do{
			idx = re_mp3.indexIn(content, idx+1);
			if(idx > 0){
				QString url = complete_url(re_mp3.cap(1));
				urls[url] = SomaFM::Station::UrlType::MP3;
			}
		} while(idx > 0);

		idx=-1;
		do
		{
			idx = re_aac.indexIn(content, idx+1);

			if(idx > 0){
				QString url = complete_url(re_aac.cap(1));
				urls[url] = SomaFM::Station::UrlType::AAC;
			}

		} while(idx > 0);
	}


	void parse_description()
	{
		QString pattern("<p\\s*class=\"descr\">(.*)</p>");
		QRegExp re(pattern);
		re.setMinimal(true);

		int idx = re.indexIn(content);
		if(idx > 0){
			description = re.cap(1).toLocal8Bit();
		}
	}

	void parse_image()
	{
		QList<QUrl> urls;
		QString pattern("<img\\s*src=\\s*\"(.*)\"");
		QRegExp re(pattern);

		re.setMinimal(true);

		int idx = re.indexIn(content);
		if(idx > 0)
		{
			QString cap = re.cap(1);
			QString filename = Util::File::get_filename_of_path(cap);

			QString mapping;
			if(this->image_low_high_map.contains(filename))
			{ // the high res image has its own naming scheme
				mapping = this->image_low_high_map[filename];
			}

			else
			{ // the high res image follows the same naming scheme as the low res version
				QRegExp re_pure_station_name("([a-zA-Z0-9]+)-*120\\..*");
				re_pure_station_name.setMinimal(true);
				idx = re_pure_station_name.indexIn(cap);
				if(idx >= 0)
				{
					mapping = re_pure_station_name.cap(1) + "-400";
				}
			}

			if(!mapping.isEmpty())
			{
				QString part_url1 = QString("/img3/%1.jpg").arg(mapping);
				QString part_url2 = QString("/img3/%1.png").arg(mapping);

				urls << QUrl(complete_url(part_url1));
				urls << QUrl(complete_url(part_url2));
			}

			urls << QUrl(complete_url(cap));

			QString cover_path = QString("%1/covers/%2.%3")
				.arg(Util::sayonara_path())
				.arg(station_name)
				.arg(Util::File::get_file_extension(cap));

            cover = Cover::Location::cover_location(urls, cover_path);
		}
	}
};

SomaFM::Station::Station()
{
	m = Pimpl::make<SomaFM::Station::Private>();
    m->cover = Cover::Location::invalid_location();
	m->loved = false;
}

SomaFM::Station::Station(const QString& content) :
	SomaFM::Station()
{
	m->content = content;

	m->parse_description();
	m->parse_station_name();
	m->parse_urls();
	//m->parse_image();
}

SomaFM::Station::Station(const SomaFM::Station& other)
{
	m = Pimpl::make<SomaFM::Station::Private>();
	SomaFM::Station::Private data = *(other.m.get());
	(*m) = data;
}

SomaFM::Station& SomaFM::Station::operator=(const SomaFM::Station& other)
{
	SomaFM::Station::Private data = *(other.m.get());
	(*m) = data;
	return *this;
}

SomaFM::Station::~Station() = default;

QString SomaFM::Station::name() const
{
	return m->station_name;
}

QStringList SomaFM::Station::playlists() const
{
	return m->urls.keys();
}

SomaFM::Station::UrlType SomaFM::Station::url_type(const QString& url) const
{
	return m->urls[url];
}

QString SomaFM::Station::description() const
{
	return m->description;
}

Cover::Location SomaFM::Station::cover_location() const
{
	if(!m->cover.is_valid())
	{
		m->parse_image();
	}

	return m->cover;
}

bool SomaFM::Station::is_valid() const
{
	return
	(
			!m->station_name.isEmpty()
			&& (!m->urls.isEmpty())
			&& (!m->description.isEmpty())
//			&& m->cover.is_valid()
	);
}


MetaDataList SomaFM::Station::metadata() const
{
	return m->v_md;
}

void SomaFM::Station::set_metadata(const MetaDataList& v_md)
{
	m->v_md = v_md;
}

void SomaFM::Station::set_loved(bool loved){
	m->loved = loved;
}

bool SomaFM::Station::is_loved() const
{
	return m->loved;
}
