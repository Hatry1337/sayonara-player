/* GUI_Stream.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include "GUI_Stream.h"
#include "ConfigureStreamDialog.h"
#include "GUI_StationSearcher.h"
#include "Gui/Plugins/ui_GUI_Stream.h"

#include "Components/Streaming/Streams/StreamHandler.h"

#include "Utils/Language/Language.h"
#include "Gui/Utils/Icons.h"

#include <QAction>

struct GUI_Stream::Private
{
	GUI_StationSearcher* searcher = nullptr;
	QAction* radio_action=nullptr;
};

GUI_Stream::GUI_Stream(QWidget *parent) :
	Gui::AbstractStationPlugin(parent)
{
	m = Pimpl::make<Private>();
}

GUI_Stream::~GUI_Stream()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

QString GUI_Stream::get_name() const
{
	return "Webstreams";
}

QString GUI_Stream::get_display_name() const
{
	return Lang::get(Lang::Streams);
}

void GUI_Stream::retranslate_ui()
{
	Gui::AbstractStationPlugin::retranslate_ui();
	ui->retranslateUi(this);

	QString action_text = tr("Search radio station");

	if(m->radio_action)
	{
		m->radio_action->setText(action_text);
	}

	ui->btn_search->setText(Lang::get(Lang::SearchVerb));
	ui->btn_search->setToolTip(action_text);
}

void GUI_Stream::init_ui()
{
	setup_parent(this, &ui);

	m->radio_action = new QAction(ui->btn_tool);
	ui->btn_tool->register_action(m->radio_action);
	connect(m->radio_action, &QAction::triggered, this, &GUI_Stream::search_radio_triggered);
	connect(ui->btn_search, &QPushButton::clicked, this, &GUI_Stream::search_radio_triggered);

	retranslate_ui();
}

QString GUI_Stream::get_title_fallback_name() const
{
	return Lang::get(Lang::Radio);
}

QComboBox* GUI_Stream::combo_stream()
{
	return ui->combo_stream;
}

QPushButton* GUI_Stream::btn_play()
{
	return ui->btn_listen;
}

Gui::MenuToolButton* GUI_Stream::btn_menu()
{
	return ui->btn_tool;
}

AbstractStationHandler* GUI_Stream::stream_handler() const
{
	return new StreamHandler();
}

void GUI_Stream::skin_changed()
{
	Gui::AbstractStationPlugin::skin_changed();

	if(m->radio_action)
	{
		m->radio_action->setIcon(Gui::Icons::icon(Gui::Icons::Search));
	}

	if(ui)
	{
		ui->btn_search->setIcon(Gui::Icons::icon(Gui::Icons::Search));
	}
}

void GUI_Stream::search_radio_triggered()
{
	if(!m->searcher)
	{
		m->searcher = new GUI_StationSearcher(this);
		connect(m->searcher, &GUI_StationSearcher::sig_stream_selected, this, &GUI_Stream::stream_selected);
	}

	m->searcher->show();
}

void GUI_Stream::stream_selected(const QString& name, const QString& url)
{
	add_stream(name, url);
}


GUI_ConfigureStation* GUI_Stream::create_config_dialog()
{
	return new ConfigureStreamDialog(this);
}
