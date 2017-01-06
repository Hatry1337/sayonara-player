/* GUI_Podcasts.cpp */

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

#include "GUI_Podcasts.h"
#include "GUI/Plugins/Stream/ui_GUI_Podcasts.h"

GUI_Podcasts::GUI_Podcasts(QWidget *parent) :
	GUI_AbstractStream(new StreamHandlerPodcasts(), parent)
 {}

GUI_Podcasts::~GUI_Podcasts()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

QString GUI_Podcasts::get_name() const
{
	return "Podcasts";
}

QString GUI_Podcasts::get_display_name() const
{
	return tr("Podcasts");
}

void GUI_Podcasts::init_ui()
{
	setup_parent(this, &ui);
	GUI_AbstractStream::init_ui();
}

void GUI_Podcasts::language_changed()
{
	if(ui){
		ui->retranslateUi(this);
	}
}

QString GUI_Podcasts::get_title_fallback_name() const
{
	return tr("Podcast");
}
