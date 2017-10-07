
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

#include "PlaylistTabMenu.h"

#include "GUI/Helper/IconLoader/IconLoader.h"
#include "GUI/Helper/GUI_Helper.h"
#include "Helper/Settings/Settings.h"
#include "Helper/Language.h"

using namespace Gui;

PlaylistTabMenu::PlaylistTabMenu(QWidget* parent) :
	WidgetTemplate<QMenu>(parent)
{
	_action_open_file = new QAction(Util::icon("play"), QString(), this);
	_action_open_dir = new QAction(Util::icon("play"), QString(), this);
	_action_reset = new QAction(Util::icon("undo"), QString(), this);
	_action_rename = new QAction(Util::icon("edit"), QString(), this);
	_action_delete = new QAction(Util::icon("delete"), QString(), this);
	_action_save = new QAction(Util::icon("save"), QString(), this);
	_action_save_as = new QAction(Util::icon("save_as"), QString(), this);
	_action_clear = new QAction(Util::icon("broom.png"), QString(), this);
	_action_close = new QAction(Util::icon("power_off"), QString(), this);
	_action_close_others = new QAction(Util::icon("power_on"), QString(), this);

	_icon_loader = IconLoader::instance();

	QList<QAction*> actions;
	actions << _action_open_file
			<< _action_open_dir
			<< this->addSeparator()
			<< _action_reset
			<< this->addSeparator()
			<< _action_rename
			<< _action_save
			<< _action_save_as
			<< _action_delete
			<< this->addSeparator()
			<< _action_clear
			<< this->addSeparator()
			<< _action_close_others
			<< _action_close;

	this->addActions(actions);

	connect(_action_open_file, &QAction::triggered, this, &PlaylistTabMenu::sig_open_file_clicked);
	connect(_action_open_dir, &QAction::triggered, this, &PlaylistTabMenu::sig_open_dir_clicked);
	connect(_action_reset, &QAction::triggered, this, &PlaylistTabMenu::sig_reset_clicked);
	connect(_action_rename, &QAction::triggered, this, &PlaylistTabMenu::sig_rename_clicked);
	connect(_action_delete, &QAction::triggered, this, &PlaylistTabMenu::sig_delete_clicked);
	connect(_action_save, &QAction::triggered, this, &PlaylistTabMenu::sig_save_clicked);
	connect(_action_save_as, &QAction::triggered, this, &PlaylistTabMenu::sig_save_as_clicked);
	connect(_action_clear, &QAction::triggered, this, &PlaylistTabMenu::sig_clear_clicked);
	connect(_action_close, &QAction::triggered, this, &PlaylistTabMenu::sig_close_clicked);
	connect(_action_close_others, &QAction::triggered, this, &PlaylistTabMenu::sig_close_others_clicked);
}

PlaylistTabMenu::~PlaylistTabMenu()
{
	this->clear();
}

void PlaylistTabMenu::language_changed()
{
	_action_open_file->setText(Lang::get(Lang::OpenFile).triplePt());
	_action_open_dir->setText(Lang::get(Lang::OpenDir).triplePt());
	_action_reset->setText(Lang::get(Lang::Reset));
	_action_rename->setText(Lang::get(Lang::Rename).triplePt());
	_action_delete->setText(Lang::get(Lang::Delete));
	_action_save->setText(Lang::get(Lang::Save));
	_action_save_as->setText(Lang::get(Lang::SaveAs).triplePt());
	_action_clear->setText(Lang::get(Lang::Clear));
	_action_close->setText(Lang::get(Lang::Close));
	_action_close_others->setText(Lang::get(Lang::CloseOthers));
}

void PlaylistTabMenu::skin_changed()
{
	_action_open_file->setIcon(_icon_loader->icon( "document-open", "play_small") );
	_action_open_dir->setIcon(_icon_loader->icon( "document-open", "play_small") );

	_action_reset->setIcon(_icon_loader->icon( "edit-undo", "undo") );
	_action_rename->setIcon(_icon_loader->icon( "accessories-text-editor", "edit") );
	_action_delete->setIcon(_icon_loader->icon( "edit-delete", "delete") );
	_action_save->setIcon(_icon_loader->icon( "document-save", "save") );
	_action_save_as->setIcon(_icon_loader->icon( "document-save-as", "save_as") );

	_action_clear->setIcon(_icon_loader->icon( "edit-clear", "broom.png") );
	_action_close->setIcon(_icon_loader->icon( "window-close", "power_off") );

	_action_close_others->setIcon(_icon_loader->icon( "window-close", "power_on") );
}

void PlaylistTabMenu::show_menu_items(PlaylistMenuEntries entries)
{
	_action_open_file->setVisible(entries & PlaylistMenuEntry::OpenFile);
	_action_open_dir->setVisible(entries & PlaylistMenuEntry::OpenDir);
	_action_reset->setVisible(entries & PlaylistMenuEntry::Reset);
	_action_rename->setVisible(entries & PlaylistMenuEntry::Rename);
	_action_delete->setVisible(entries & PlaylistMenuEntry::Delete);
	_action_save->setVisible(entries & PlaylistMenuEntry::Save);
	_action_save_as->setVisible(entries & PlaylistMenuEntry::SaveAs);
	_action_clear->setVisible(entries & PlaylistMenuEntry::Clear);
	_action_close->setVisible(entries & PlaylistMenuEntry::Close);
	_action_close_others->setVisible(entries & PlaylistMenuEntry::CloseOthers);
}


void PlaylistTabMenu::show_close(bool b)
{
	_action_close->setVisible(b);
	_action_close_others->setVisible(b);
}

