/* Session.h */

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



#ifndef SESSION_H
#define SESSION_H

#include "Utils/Pimpl.h"

#include <QObject>
#include <QDateTime>

class QDateTime;

class Session :
		public QObject
{
	Q_OBJECT
	PIMPL(Session)

public:
	explicit Session(QObject* parent=nullptr);
	~Session();

	static QMap<QDateTime, MetaDataList> get_history(QDateTime beginning=QDateTime());

private slots:
	void track_changed(const MetaData& md);
};

#endif // SESSION_H
