/* Yandex.cpp */

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

#include "Yandex.h"

#include <QByteArray>
#include <QRegExp>
#include <QUrl>
#include <QStringList>
#include <QString>

using Cover::Fetcher::Yandex;

QString Yandex::privateIdentifier() const
{
	return "yandex";
}

bool Yandex::canFetchCoverDirectly() const
{
	return false;
}

QStringList Yandex::parseAddresses(const QByteArray& website) const
{
	QRegExp re("<img.+class=\"serp-item__thumb.+src=\"(.+)\"");
	re.setMinimal(true);
	int idx = re.indexIn(website);

	if(idx < 0){
		return QStringList();
	}

	QStringList ret;
	while(idx >= 0)
	{
		QString url = "https:" + re.cap(1);
		url.replace("&amp;", "&");
		ret << url;
		idx += re.cap(1).size();

		idx = re.indexIn(website, idx);
	}

	return ret;
}


QString Yandex::artistAddress(const QString& artist) const
{
	return fulltextSearchAddress(artist);
}

QString Yandex::albumAddress(const QString& artist, const QString& album) const
{
	return fulltextSearchAddress(album + " " + artist);
}

QString Yandex::fulltextSearchAddress(const QString& str) const
{
	QString pe = QUrl::toPercentEncoding(str);
	return QString("https://yandex.com/images/search?text=%1&iorient=square&from=tabbar").arg(pe);
}

int Yandex::estimatedSize() const
{
	return 300;
}
