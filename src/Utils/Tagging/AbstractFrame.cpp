/* AbstractFrame.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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



#include "AbstractFrame.h"
#include "taglib/tstring.h"

#include <QString>

struct Tagging::AbstractFrameHelper::Private
{
	QString key;
};

Tagging::AbstractFrameHelper::AbstractFrameHelper(const QString& key)
{
	m = Pimpl::make<Private>();
	m->key = key;
}

Tagging::AbstractFrameHelper::~AbstractFrameHelper() {}


TagLib::String Tagging::AbstractFrameHelper::convert_string(const QString& str) const
{
	return TagLib::String(str.toUtf8().data(), TagLib::String::Type::UTF8);
}

QString Tagging::AbstractFrameHelper::convert_string(const TagLib::String& str) const
{
	return QString(str.toCString(true));
}

QString Tagging::AbstractFrameHelper::key() const
{
	return m->key;
}

TagLib::String Tagging::AbstractFrameHelper::tag_key() const
{
	return convert_string(m->key);
}

