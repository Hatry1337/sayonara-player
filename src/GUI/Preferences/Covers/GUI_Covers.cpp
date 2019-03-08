/* GUI_Covers.cpp */

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

#include "GUI/Preferences/ui_GUI_Covers.h"

#include "GUI_Covers.h"

#include "Database/Connector.h"
#include "Database/CoverConnector.h"

#include "Components/Covers/CoverFetchManager.h"
#include "Components/Covers/CoverFetcherInterface.h"
#include "Components/Covers/CoverChangeNotifier.h"
#include "Components/Covers/CoverUtils.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Language.h"
#include "Utils/FileUtils.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

#include "GUI/Utils/Delegates/StyledItemDelegate.h"

#include <QListWidgetItem>
#include <QList>
#include <QDir>
#include <QFileInfo>
#include <QCheckBox>

using namespace Cover;

GUI_Covers::GUI_Covers(const QString& identifier) :
	Base (identifier) {}


GUI_Covers::~GUI_Covers()
{
	if(ui){
		delete ui; ui = nullptr;
	}
}

bool GUI_Covers::commit()
{
	Settings* settings = Settings::instance();
	QStringList active_items;

	for(int i=0; i<ui->lv_cover_searchers->count(); i++)
	{
		QListWidgetItem* item = ui->lv_cover_searchers->item(i);
		active_items << item->text();
	}

	settings->set<Set::Cover_Server>(active_items);
	settings->set<Set::Cover_FetchFromWWW>(ui->cb_fetch_covers_from_www->isChecked());

	return true;
}

void GUI_Covers::revert()
{
	Settings* settings = Settings::instance();
	Cover::Fetcher::Manager* cfm = Cover::Fetcher::Manager::instance();

	QStringList cover_servers = settings->get<Set::Cover_Server>();

	ui->lv_cover_searchers->clear();
	ui->lv_cover_searchers_inactive->clear();

	QList<Cover::Fetcher::Base*> cover_fetchers = cfm->coverfetchers();
	for(const Cover::Fetcher::Base* b : cover_fetchers)
	{
		QString name = b->keyword();
		if(name.trimmed().isEmpty()) {
			continue;
		}

		if(cover_servers.contains(name)) {
			ui->lv_cover_searchers->addItem(name);
		}

		else {
			ui->lv_cover_searchers_inactive->addItem(name);
		}
	}

	bool fetch_from_www = settings->get<Set::Cover_FetchFromWWW>();
	ui->cb_fetch_covers_from_www->setChecked(fetch_from_www);

	fetch_covers_www_triggered(fetch_from_www);

	current_row_changed(ui->lv_cover_searchers->currentRow());
}

QString GUI_Covers::action_name() const
{
	return Lang::get(Lang::Covers);
}

void GUI_Covers::init_ui()
{
	if(ui){
		return;
	}

	setup_parent(this, &ui);

	ui->lv_cover_searchers->setItemDelegate(new Gui::StyledItemDelegate(ui->lv_cover_searchers));
	ui->lv_cover_searchers_inactive->setItemDelegate(new Gui::StyledItemDelegate(ui->lv_cover_searchers_inactive));

	connect(ui->btn_up, &QPushButton::clicked, this, &GUI_Covers::up_clicked);
	connect(ui->btn_down, &QPushButton::clicked, this, &GUI_Covers::down_clicked);
	connect(ui->lv_cover_searchers, &QListWidget::currentRowChanged, this, &GUI_Covers::current_row_changed);
	connect(ui->btn_delete_album_covers, &QPushButton::clicked, this, &GUI_Covers::delete_covers_from_db);
	connect(ui->btn_delete_files, &QPushButton::clicked, this, &GUI_Covers::delete_cover_files);
	connect(ui->cb_fetch_covers_from_www, &QCheckBox::toggled, this, &GUI_Covers::fetch_covers_www_triggered);
	connect(ui->btn_add, &QPushButton::clicked, this, &GUI_Covers::add_clicked);
	connect(ui->btn_remove, &QPushButton::clicked, this, &GUI_Covers::remove_clicked);

	revert();
}

void GUI_Covers::retranslate_ui()
{
	ui->retranslateUi(this);

	ui->btn_up->setText(Lang::get(Lang::MoveUp));
	ui->btn_down->setText(Lang::get(Lang::MoveDown));
}

void GUI_Covers::up_clicked()
{
	int cur_row = ui->lv_cover_searchers->currentRow();

	QListWidgetItem* item = ui->lv_cover_searchers->takeItem(cur_row);
	ui->lv_cover_searchers->insertItem(cur_row - 1, item);
	ui->lv_cover_searchers->setCurrentRow(cur_row - 1);
}

void GUI_Covers::down_clicked()
{
	int cur_row = ui->lv_cover_searchers->currentRow();

	QListWidgetItem* item = ui->lv_cover_searchers->takeItem(cur_row);
	ui->lv_cover_searchers->insertItem(cur_row + 1, item);
	ui->lv_cover_searchers->setCurrentRow(cur_row + 1);
}

void GUI_Covers::add_clicked()
{
	QListWidgetItem* item = ui->lv_cover_searchers_inactive->takeItem(ui->lv_cover_searchers_inactive->currentRow());
	if(!item){
		return;
	}

	ui->lv_cover_searchers->addItem(item->text());
	delete item; item=nullptr;
}

void GUI_Covers::remove_clicked()
{
	QListWidgetItem* item = ui->lv_cover_searchers->takeItem(ui->lv_cover_searchers->currentRow());
	if(!item){
		return;
	}

	ui->lv_cover_searchers_inactive->addItem(item->text());
	delete item; item=nullptr;
}

void GUI_Covers::current_row_changed(int row)
{
	ui->btn_up->setDisabled(row <= 0 || row >= ui->lv_cover_searchers->count());
	ui->btn_down->setDisabled(row < 0 || row >= ui->lv_cover_searchers->count() - 1);
}

void GUI_Covers::delete_covers_from_db()
{
	DB::Connector::instance()->cover_connector()->clear();
	Cover::ChangeNotfier::instance()->shout();
}

void GUI_Covers::delete_cover_files()
{
	::Util::File::remove_files_in_directory(Cover::Utils::cover_directory());
}

void GUI_Covers::fetch_covers_www_triggered(bool b)
{
	ui->lv_cover_searchers->setEnabled(b);
	ui->lv_cover_searchers_inactive->setEnabled(b);
	ui->btn_down->setEnabled(b);
	ui->btn_up->setEnabled(b);
	ui->btn_add->setEnabled(b);
	ui->btn_remove->setEnabled(b);
}
