/* GUI_PlaylistChooser.cpp */

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

#include "GUI_PlaylistChooser.h"
#include "GUI_TargetPlaylistDialog.h"
#include "GUI/Plugins/PlaylistChooser/ui_GUI_PlaylistChooser.h"

#include "Components/PlaylistChooser/PlaylistChooser.h"

#include "GUI/Helper/Delegates/ComboBoxDelegate.h"
#include "Helper/Message/Message.h"

#include "Helper/Helper.h"
#include "Helper/Playlist/CustomPlaylist.h"
#include "Helper/Language.h"
#include "Helper/Settings/Settings.h"

#include <algorithm>

GUI_PlaylistChooser::GUI_PlaylistChooser(QWidget *parent) :
	PlayerPluginInterface(parent)
{
	_playlist_chooser = new PlaylistChooser();
}


GUI_PlaylistChooser::~GUI_PlaylistChooser()
{
	if(ui){
		delete ui; ui=nullptr;
	}

	delete _playlist_chooser;
}


void GUI_PlaylistChooser::init_ui()
{
	setup_parent(this, &ui);

	_last_dir = _settings->get(Set::Lib_Path);

	ui->combo_playlistchooser->setItemDelegate(new ComboBoxDelegate(this));

	ui->btn_tool_pl->show_action(ContextMenu::EntryDelete, false);
	ui->btn_tool_pl->show_action(ContextMenu::EntrySave, false);

	ui->btn_tool_file->show_action(ContextMenu::EntrySaveAs, true);
	ui->btn_tool_file->show_action(ContextMenu::EntryOpen, true);

	connect(ui->btn_tool_pl, &MenuToolButton::sig_save, this, &GUI_PlaylistChooser::save_button_pressed);
	connect(ui->btn_tool_pl, &MenuToolButton::sig_delete, this, &GUI_PlaylistChooser::delete_button_pressed);

	connect(ui->btn_tool_file, &MenuToolButton::sig_save_as, this, &GUI_PlaylistChooser::save_as_button_pressed);
	connect(ui->btn_tool_file, &MenuToolButton::sig_open, this, &GUI_PlaylistChooser::load_button_pressed);
	connect(ui->combo_playlistchooser, combo_activated_int, this, &GUI_PlaylistChooser::playlist_selected);
	connect(ui->combo_playlistchooser, &QComboBox::editTextChanged, this, &GUI_PlaylistChooser::edit_text_changed);

	connect(_playlist_chooser, &PlaylistChooser::sig_all_playlists_loaded,
			this, &GUI_PlaylistChooser::all_playlists_fetched);

	_playlist_chooser->load_all_playlists();
}


void GUI_PlaylistChooser::language_changed()
{
	if(!is_ui_initialized()){
		return;
	}

	ui->retranslateUi(this);
}


QString GUI_PlaylistChooser::get_name() const
{
	return "Playlists";
}


QString GUI_PlaylistChooser::get_display_name() const
{
	return Lang::get(Lang::Playlists);
}


void GUI_PlaylistChooser::edit_text_changed(const QString& name)
{
	ui->btn_tool_pl->show_action(ContextMenu::EntrySave, !name.isEmpty());
}


void GUI_PlaylistChooser::all_playlists_fetched(const CustomPlaylistSkeletons& skeletons)
{
	if(!is_ui_initialized()){
		return;
	}

	QString old_text = ui->combo_playlistchooser->currentText();

	ui->combo_playlistchooser->clear();
	ui->combo_playlistchooser->addItem("", -1);

	for(const CustomPlaylistSkeleton& skeleton : skeletons)
	{
		ui->combo_playlistchooser->addItem(skeleton.name(), skeleton.id());
	}

	int cur_idx = std::max(ui->combo_playlistchooser->findText(old_text), 0);

	ui->combo_playlistchooser->setCurrentIndex(cur_idx);
}


void GUI_PlaylistChooser::save_button_pressed()
{
	int id = _playlist_chooser->find_playlist(ui->combo_playlistchooser->currentText());

	if(id < 0){
		QString new_name = ui->combo_playlistchooser->currentText();
		_playlist_chooser->save_playlist(new_name);
	}

	else{
		GlobalMessage::Answer answer = Message::question_yn(Lang::get(Lang::Overwrite).question());
		if(answer == GlobalMessage::Answer::Yes){
			_playlist_chooser->save_playlist(id);
		}
	}
}


void GUI_PlaylistChooser::got_save_params(const QString& filename, bool relative)
{
	sender()->deleteLater();

	_playlist_chooser->save_playlist_file(filename, relative);
}


void GUI_PlaylistChooser::save_as_button_pressed()
{
	GUI_TargetPlaylistDialog* target_playlist_dialog = new GUI_TargetPlaylistDialog(this);

	connect(target_playlist_dialog, &GUI_TargetPlaylistDialog::sig_target_chosen,
			this, &GUI_PlaylistChooser::got_save_params);

	target_playlist_dialog->show();
}


void GUI_PlaylistChooser::delete_button_pressed()
{
	int cur_idx = ui->combo_playlistchooser->currentIndex();

	GlobalMessage::Answer answer = Message::question_yn(Lang::get(Lang::Delete).question());

	int val = ui->combo_playlistchooser->itemData(cur_idx).toInt();

	if(val >= 0 && answer == GlobalMessage::Answer::Yes){
		_playlist_chooser->delete_playlist(val);
	}
}


void GUI_PlaylistChooser::playlist_selected(int idx)
{
	int id = _playlist_chooser->find_playlist(ui->combo_playlistchooser->currentText());

	ui->btn_tool_pl->show_action(ContextMenu::EntrySave, idx > 0);
	ui->btn_tool_pl->show_action(ContextMenu::EntryDelete, id >= 0);

	if(id >= 0){
		_playlist_chooser->load_single_playlist(id);
	}

	ui->le_playlist_file->clear();
}


void GUI_PlaylistChooser::load_button_pressed()
{
    QStringList filelist = QFileDialog::getOpenFileNames(
                    this,
                    tr("Open Playlist files"),
                    _last_dir,
                    Helper::get_playlistfile_extensions().join(" "));

	QString lab_text;
	for(const QString& filename : filelist) {
		int last_index_of_sep = filename.lastIndexOf(QDir::separator());
        _last_dir = filename.left(last_index_of_sep);

        QString trimmed_filename = filename.right(filename.size() - last_index_of_sep - 1);
        trimmed_filename = trimmed_filename.left(trimmed_filename.lastIndexOf('.'));
        lab_text += trimmed_filename + ", ";
    }

	if(lab_text.size() > 2){
		lab_text = lab_text.left(lab_text.size() - 2);
	}

	ui->le_playlist_file->setText(lab_text);

    if(filelist.size() > 0) {
		_playlist_chooser->playlist_files_selected(filelist);
    }
}

