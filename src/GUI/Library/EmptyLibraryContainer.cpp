/* EmptyLibraryContainer.cpp */

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



#include "EmptyLibraryContainer.h"
#include "GUI/Helper/IconLoader/IconLoader.h"
#include "GUI/Library/GUI_EmptyLibrary.h"
#include "GUI/Library/EmptyLibraryContainer.h"

#include <QIcon>
#include <QComboBox>

struct EmptyLibraryContainer::Private
{
	GUI_EmptyLibrary* ui=nullptr;

};

EmptyLibraryContainer::EmptyLibraryContainer(QObject* parent) :
	LibraryContainerInterface(parent)
{
	_m = Pimpl::make<Private>();
}

EmptyLibraryContainer::~EmptyLibraryContainer() {}

QIcon EmptyLibraryContainer::get_icon() const
{
	return IconLoader::getInstance()->get_icon("document-new", "new");
}

QString EmptyLibraryContainer::get_name() const
{
	return "empty-library";
}

QString EmptyLibraryContainer::get_display_name() const
{
	return tr("Empty Library");
}

QWidget* EmptyLibraryContainer::get_ui() const
{
	return static_cast<QWidget*>(_m->ui);
}

QComboBox* EmptyLibraryContainer::get_libchooser()
{
	if(_m->ui){
		return _m->ui->get_libchooser();
	}

	return nullptr;
}

QMenu*EmptyLibraryContainer::get_menu()
{
	return nullptr;
}

void EmptyLibraryContainer::init_ui()
{
	_m->ui = new GUI_EmptyLibrary();
}