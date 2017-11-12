/* SoundcloudLibraryContainer.cpp */

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

/* SoundcloudLibraryContainer.cpp */

#include "SoundcloudLibraryContainer.h"
#include "ui/GUI_SoundcloudLibrary.h"
#include "src/SoundcloudLibrary.h"

#include <QPixmap>

static void sc_init_icons()
{
	Q_INIT_RESOURCE(SoundcloudIcons);
}

SC::LibraryContainer::LibraryContainer(QObject *parent) :
	::Library::Container(parent)
{
	sc_init_icons();
}

SC::LibraryContainer::~LibraryContainer() {}

QString SC::LibraryContainer::name() const
{
	return "soundcloud";
}

QString SC::LibraryContainer::display_name() const
{
	return "Soundcloud";
}

QWidget* SC::LibraryContainer::widget() const
{
	return static_cast<QWidget*>(ui);
}

QMenu* SC::LibraryContainer::menu()
{
	if(ui){
		return ui->get_menu();
	}

	return nullptr;
}

void SC::LibraryContainer::init_ui()
{
	SC::Library* library = new SC::Library(this);
	ui = new SC::GUI_Library(library);
}

QFrame* SC::LibraryContainer::header() const
{
	return ui->header_frame();
}

QPixmap SC::LibraryContainer::icon() const
{
	return QPixmap(":/sc_icons/ui/icon.png");
}
