/* GUI_Podcasts.cpp */

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

#include "GUI_Podcasts.h"
#include "ConfigurePodcastDialog.h"
#include "Gui/Plugins/ui_GUI_Podcasts.h"

#include "Components/Streaming/Streams/PodcastHandler.h"

#include "Utils/Language/Language.h"

struct GUI_Podcasts::Private
{
	PlaylistCreator* playlistCreator;

	Private(PlaylistCreator* playlistCreator) :
		playlistCreator(playlistCreator)
	{}
};

GUI_Podcasts::GUI_Podcasts(PlaylistCreator* playlistCreator, QWidget* parent) :
	Gui::AbstractStationPlugin(playlistCreator, parent)
{
	m = Pimpl::make<Private>(playlistCreator);
}

GUI_Podcasts::~GUI_Podcasts()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

QString GUI_Podcasts::name() const
{
	return "Podcasts";
}

QString GUI_Podcasts::displayName() const
{
	return Lang::get(Lang::Podcasts);
}

void GUI_Podcasts::initUi()
{
	setupParent(this, &ui);
	Gui::AbstractStationPlugin::initUi();
}

void GUI_Podcasts::retranslate()
{
	Gui::AbstractStationPlugin::retranslate();
	ui->retranslateUi(this);
}

QString GUI_Podcasts::titleFallbackName() const
{
	return tr("Podcast");
}

QComboBox* GUI_Podcasts::comboStream()
{
	return ui->comboStream;
}

QPushButton* GUI_Podcasts::btnPlay()
{
	return ui->btnListen;
}

Gui::MenuToolButton* GUI_Podcasts::btnMenu()
{
	return ui->btnTool;
}

AbstractStationHandler* GUI_Podcasts::streamHandler() const
{
	return new PodcastHandler(m->playlistCreator);
}

GUI_ConfigureStation* GUI_Podcasts::createConfigDialog()
{
	return new ConfigurePodcastDialog(this);
}
