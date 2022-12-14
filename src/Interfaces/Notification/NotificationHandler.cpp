
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

#include "NotificationHandler.h"

#include "Utils/Algorithm.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

namespace Algorithm=::Util::Algorithm;

struct NotificationHandler::Private
{
	NotificatonList notificators;
	int currentIndex;

	Private() :
		currentIndex(-1)
	{}
};

NotificationHandler::NotificationHandler(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();
}

NotificationHandler::~NotificationHandler() = default;

void NotificationHandler::notify(const MetaData& md)
{
	get()->notify(md);
}

void NotificationHandler::notify(const QString& title, const QString& message, const QString& imagePath)
{
	if(imagePath.isEmpty()){
		get()->notify(title, message, ":/Icons/logo.png");
	}

	else {
		get()->notify(title, message, imagePath);
	}
}

void NotificationHandler::registerNotificator(NotificationInterface* notificator)
{
	spLog(Log::Info, this) << "Notification handler " << notificator->name() << " registered";
	m->notificators << notificator;

	QString preferred = GetSetting(Set::Notification_Name);

	auto lambda = [preferred](NotificationInterface* n){
		return (n->name().compare(preferred, Qt::CaseInsensitive) == 0);
	};

	auto it = std::find_if(m->notificators.begin(), m->notificators.end(), lambda);
	m->currentIndex = (it - m->notificators.begin());

	if(m->currentIndex >= m->notificators.size()){
		m->currentIndex = 0;
	}

	emit sigNotificationsChanged();
}


void NotificationHandler::notificatorChanged(const QString& name)
{
	m->currentIndex = -1;
	int i = 0;

	for(NotificationInterface* n : Algorithm::AsConst(m->notificators))
	{
		if(n->name().compare(name, Qt::CaseInsensitive) == 0){
			m->currentIndex = i;
			break;
		}

		i++;
	}
}

NotificationInterface* NotificationHandler::get() const
{
	if(m->currentIndex < 0)
	{
		static DummyNotificator dummy;
		return &dummy;
	}

	return m->notificators[m->currentIndex];
}


NotificatonList NotificationHandler::notificators() const
{
	return m->notificators;
}

int NotificationHandler::currentIndex() const
{
	return m->currentIndex;
}


DummyNotificator::DummyNotificator() :
	NotificationInterface() {}

DummyNotificator::~DummyNotificator() = default;

void DummyNotificator::notify(const MetaData &md)
{
	Q_UNUSED(md)
}

QString DummyNotificator::name() const
{
	return "Dummy";
}

void DummyNotificator::notify(const QString& title, const QString& message, const QString& image_path)
{
	Q_UNUSED(title)
	Q_UNUSED(message)
	Q_UNUSED(image_path)
}
