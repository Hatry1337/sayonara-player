/* DiscogsCoverFetcher.cpp */

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



#include "DiscogsCoverFetcher.h"
#include <QRegExp>
#include <QStringList>
#include <QUrl>
#include "Utils/Logger/Logger.h"

using namespace Cover::Fetcher;

static QString basic_url(const QString& str)
{
	QString str2 = str;
	str2 = str2.replace(" ", "+");

	QString domain = "www.discogs.com";

	return "https://" + domain + "/search/?q=" +
			QUrl::toPercentEncoding(str2);
}

bool Discogs::can_fetch_cover_directly() const
{
	return false;
}

QStringList Discogs::calc_addresses_from_website(const QByteArray& website) const
{
	QStringList ret;

	QRegExp re("class=\"thumbnail_center\">\\s*<img\\s*data-src\\s*=\\s*\"(.+)\"");
	re.setMinimal(true);
	QString website_str = QString::fromLocal8Bit(website);
	int idx = re.indexIn(website_str);
	while(idx > 0){
		ret << re.cap(1);
		website_str.remove(0, idx + 5);
		idx = re.indexIn(website_str);
	}

	return ret;
}

QString Discogs::artist_address(const QString& artist) const
{

    return basic_url(artist) + "&type=artist";
}

QString Discogs::album_address(const QString& artist, const QString& album) const
{
    return basic_url(artist + "+" + album) + "&type=all";
}

QString Discogs::search_address(const QString& str) const
{
    return basic_url(str) + "&type=all";
}

bool Discogs::is_search_supported() const
{
	return true;
}

bool Discogs::is_album_supported() const
{
	return true;
}

bool Discogs::is_artist_supported() const
{
	return true;
}

int Discogs::estimated_size() const
{
	return 350;
}

QString Discogs::keyword() const
{
	return "discogs";
}
