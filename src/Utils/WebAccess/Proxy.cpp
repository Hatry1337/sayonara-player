/* Proxy.cpp */

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



#include "Proxy.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

#include <QNetworkProxy>

Proxy::Proxy() :
	QObject(),
	SayonaraClass()
{
	Set::listen(Set::Proxy_Active, this, &Proxy::proxy_changed);
	Set::listen(Set::Proxy_Hostname, this, &Proxy::proxy_changed);
	Set::listen(Set::Proxy_Port, this, &Proxy::proxy_changed);
	Set::listen(Set::Proxy_Username, this, &Proxy::proxy_changed);
	Set::listen(Set::Proxy_Password, this, &Proxy::proxy_changed);
}

Proxy::~Proxy() {}

void Proxy::proxy_changed()
{
	QNetworkProxy proxy;

	if(active())
	{
		proxy.setType(QNetworkProxy::HttpProxy);
		proxy.setHostName(hostname());
		proxy.setPort(port());

		if(has_username()){
			proxy.setUser(username());
			proxy.setPassword(password());
		}

		QString url = full_url();

		Util::set_environment("http_proxy", url.toLocal8Bit().data());
		Util::set_environment("https_proxy", url.toLocal8Bit().data());
		Util::set_environment("HTTP_PROXY", url.toLocal8Bit().data());
		Util::set_environment("HTTPS_PROXY", url.toLocal8Bit().data());
	}

	else {
		proxy.setType(QNetworkProxy::NoProxy);
	
		Util::unset_environment("http_proxy");
		Util::unset_environment("https_proxy");
		Util::unset_environment("HTTP_PROXY");
		Util::unset_environment("HTTPS_PROXY");
	}

	QNetworkProxy::setApplicationProxy(proxy);
}

void Proxy::init()
{
	proxy_changed();
}


QString Proxy::hostname() const
{
	return _settings->get(Set::Proxy_Hostname);
}

int Proxy::port() const
{
	return _settings->get(Set::Proxy_Port);
}

QString Proxy::username() const
{
	return _settings->get(Set::Proxy_Username);
}

QString Proxy::password() const
{
	return _settings->get(Set::Proxy_Password);
}

bool Proxy::active() const
{
	return _settings->get(Set::Proxy_Active);
}

bool Proxy::has_username() const
{
	return ((username() + password()).size() > 0);
}

QString Proxy::full_url() const
{
	if(!active()){
		return QString();
	}

	QString host_name = hostname();
	if(!host_name.startsWith("http")){
		host_name.prepend("http://");
	}

	return host_name + ":" + QString::number(port());
}

