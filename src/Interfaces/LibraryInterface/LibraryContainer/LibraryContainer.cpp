/* LibraryContainer.cpp */

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

#include "Interfaces/LibraryInterface/LibraryContainer/LibraryContainer.h"
#include "Helper/Settings/Settings.h"

#include <QAction>

struct LibraryContainerInterface::Private
{
	QAction*	action=nullptr;
	bool		initialized;

	Private()
	{
		initialized = false;
	}

	void set_initialized()
	{
		initialized = true;
	}

};

LibraryContainerInterface::LibraryContainerInterface(QObject* parent) :
	QObject(parent),
	SayonaraClass()
{
	m = Pimpl::make<Private>();

	Set::listen(Set::Player_Language, this, &LibraryContainerInterface::language_changed);
}

LibraryContainerInterface::~LibraryContainerInterface() {}

QString LibraryContainerInterface::display_name() const
{
	return name();
}

QMenu* LibraryContainerInterface::menu()
{
	return nullptr;
}

void LibraryContainerInterface::set_menu_action(QAction* action)
{
	m->action = action;
}

QAction* LibraryContainerInterface::menu_action() const
{
	return m->action;
}

void LibraryContainerInterface::set_initialized()
{
	m->set_initialized();
}

bool LibraryContainerInterface::is_initialized() const
{
	return m->initialized;
}


void LibraryContainerInterface::language_changed()
{
	if(m->action){
		m->action->setText(this->display_name());
	}
}


void LibraryContainerInterface::show()
{
	QWidget* own_widget = widget();

	if(own_widget) {
		own_widget->setVisible(true);
		QWidget* parent_widget = own_widget->parentWidget();
		if(parent_widget){
			own_widget->resize(parent_widget->size());
		}

		own_widget->update();
	}

	if(menu_action()){
		menu_action()->setText(this->name());
		menu_action()->setVisible(true);
	}
}

void LibraryContainerInterface::hide()
{
	if(!this->is_initialized()){
		return;
	}

	if(menu_action()){
		menu_action()->setVisible(false);
	}

	if(widget())
	{
		widget()->hide();
	}
}



