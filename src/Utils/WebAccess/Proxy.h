/* Proxy.h */

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

#ifndef PROXY_H
#define PROXY_H

#include "Utils/Singleton.h"

#include <QObject>

namespace Proxy
{
	void init();
	void set_proxy();
	void unset_proxy();

	QString hostname();
	int port();
	QString username();
	QString password();
	bool active();
	bool has_username();

	QString full_url();

	QString env_hostname();
	int env_port();
};


#endif // PROXY_H
