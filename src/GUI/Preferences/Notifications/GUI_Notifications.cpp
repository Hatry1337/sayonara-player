/* GUI_Notifications.cpp */

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

#include "GUI_Notifications.h"
#include "GUI/Preferences/ui_GUI_Notifications.h"

#include "Utils/Settings/Settings.h"
#include "Interfaces/Notification/NotificationHandler.h"

GUI_Notifications::GUI_Notifications(const QString& identifier) :
	Preferences::Base(identifier) {}

GUI_Notifications::~GUI_Notifications()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

void GUI_Notifications::retranslate_ui()
{
	ui->retranslateUi(this);

	notifications_changed();
}

void GUI_Notifications::notifications_changed()
{
	if(!is_ui_initialized()){
		return;
	}

	NotificationHandler* nh = NotificationHandler::instance();
	NotificatonList notifications = nh->notificators();

	ui->combo_notification->clear();

	for(const NotificationInterface* notification : notifications)
	{
		ui->combo_notification->addItem(notification->display_name(), notification->name());
	}

	ui->combo_notification->setCurrentIndex(nh->current_index());
}


bool GUI_Notifications::commit()
{
	NotificationHandler* nh = NotificationHandler::instance();

	bool active =       ui->cb_activate->isChecked();
	int timeout =       ui->sb_timeout->value();
	QString cur_data =  ui->combo_notification->currentData().toString();

	_settings->set<Set::Notification_Name>(cur_data);
	_settings->set<Set::Notification_Timeout>(timeout);
	_settings->set<Set::Notification_Show>(active);

	nh->notificator_changed(cur_data);

	return true;
}

void GUI_Notifications::revert()
{
	int timeout = _settings->get<Set::Notification_Timeout>();
	int active = _settings->get<Set::Notification_Show>();

	ui->sb_timeout->setValue(timeout);
	ui->cb_activate->setChecked(active);

	notifications_changed();
}


QString GUI_Notifications::action_name() const
{
	return tr("Notifications");
}

void GUI_Notifications::init_ui()
{
	setup_parent(this, &ui);

	NotificationHandler* nh = NotificationHandler::instance();

	revert();

	connect(nh,	&NotificationHandler::sig_notifications_changed,
			this, &GUI_Notifications::notifications_changed);
}
