/* LocalLibraryContainer.cpp */

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

#include "LocalLibraryContainer.h"
#include "GUI/Library/GUI_LocalLibrary.h"
#include "GUI/Utils/GuiUtils.h"
#include "Utils/Library/LibraryInfo.h"

#include <QAction>

using namespace Library;

struct LocalLibraryContainer::Private
{
	GUI_LocalLibrary*   ui=nullptr;
	Info			library;
	QString				name;
	QString				library_path;

	Private(const Info& library) :
		library(library)
	{
		name = library.name();
	}
};

LocalLibraryContainer::LocalLibraryContainer(const Library::Info& library, QObject* parent) :
	Container(parent)
{
	m = Pimpl::make<Private>(library);
}

LocalLibraryContainer::~LocalLibraryContainer()
{
	/*if(m->ui) {
		delete m->ui; m->ui=nullptr;
	}*/
}

QString LocalLibraryContainer::name() const
{
	QString name = display_name();
	name = name.toLower();
	name.replace(" ", "-");
	return name;
}

QString LocalLibraryContainer::display_name() const
{
	return m->name;
}

QWidget* LocalLibraryContainer::widget() const
{
	return static_cast<QWidget*>(m->ui);
}

QMenu*LocalLibraryContainer::menu()
{
	if(m->ui){
		return m->ui->menu();
	}

	return nullptr;
}

void LocalLibraryContainer::init_ui()
{
	m->ui = new GUI_LocalLibrary(m->library.id());
}

int8_t LocalLibraryContainer::id()
{
	return m->library.id();
}


QFrame* LocalLibraryContainer::header() const
{
	return m->ui->header_frame();
}

QPixmap LocalLibraryContainer::icon() const
{
	return Gui::Util::pixmap("append.svg");
}

void LocalLibraryContainer::set_name(const QString& name)
{
	m->name = name;

	if(menu_action()){
		menu_action()->setText(name);
	}
}
