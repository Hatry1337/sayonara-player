/* DBusMediaKeysInterfaceMate.cpp */

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

#include "DBusMediaKeysInterfaceMate.h"
#include "DBus/mate_settings_daemon.h"
#include "Utils/Logger/Logger.h"

struct DBusMediaKeysInterfaceMate::Private
{
	OrgMateSettingsDaemonMediaKeysInterface* mediaKeyInterface=nullptr;

	Private(DBusMediaKeysInterface* parent)
	{
		mediaKeyInterface = new OrgMateSettingsDaemonMediaKeysInterface(
				"org.mate.SettingsDaemon",
				"/org/mate/SettingsDaemon/MediaKeys",
				QDBusConnection::sessionBus(),
				parent);
	}
};

DBusMediaKeysInterfaceMate::DBusMediaKeysInterfaceMate(PlayManager* playManager, QObject* parent) :
	DBusMediaKeysInterface(playManager, parent)
{
	m = Pimpl::make<Private>(this);

	init();
}

DBusMediaKeysInterfaceMate::~DBusMediaKeysInterfaceMate() {}

QString DBusMediaKeysInterfaceMate::serviceName() const
{
	return QString("org.mate.SettingsDaemon");
}

QDBusPendingReply<> DBusMediaKeysInterfaceMate::grabMediaKeyReply()
{
	return m->mediaKeyInterface->GrabMediaPlayerKeys("sayonara", 0);
}

void DBusMediaKeysInterfaceMate::connectMediaKeys()
{
	connect( m->mediaKeyInterface,
			 &OrgMateSettingsDaemonMediaKeysInterface::MediaPlayerKeyPressed,
			 this,
			 &DBusMediaKeysInterfaceMate::mediaKeyPressed
	);
}


