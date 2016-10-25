/* GUI_LocalLibrary.cpp */

/* Copyright (C) 2011-2016 Lucio Carreras
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


/*
 * GUI_LocalLibrary.cpp
 *
 *  Created on: Apr 24, 2011
 *      Author: Lucio Carreras
 */

#include "GUI_LocalLibrary.h"
#include "GUI/Library/ui_GUI_LocalLibrary.h"
#include "GUI/Library/Helper/LocalLibraryMenu.h"

#include "Helper/Message/Message.h"
#include "InfoBox/GUI_LibraryInfoBox.h"
#include "ImportFolderDialog/GUI_ImportFolder.h"

#include "Components/Library/LocalLibrary.h"
#include "Helper/Helper.h"
#include "Helper/Settings/Settings.h"
#include "Helper/Language.h"

#include <QFileDialog>
#include <QDir>
#include <QTimer>
#include <QShortcut>
#include <QInputDialog>
#include <QMessageBox>


GUI_LocalLibrary::GUI_LocalLibrary(QWidget* parent) :
	GUI_AbstractLibrary(LocalLibrary::getInstance(), parent)
{
	setup_parent(this, &ui);

	LocalLibrary* library = LocalLibrary::getInstance();

	_local_library_menu = new LocalLibraryMenu(this);

	ui->pb_progress->setVisible(false);
	ui->lab_progress->setVisible(false);

	connect(_library, &LocalLibrary::sig_reloading_library, this, &GUI_LocalLibrary::progress_changed);
	connect(_library, &LocalLibrary::sig_reloading_library_finished, this, &GUI_LocalLibrary::reload_finished);
	connect(ui->btn_setLibrary, &QPushButton::clicked, this, &GUI_LocalLibrary::set_library_path_clicked);

	connect(ui->lv_album, &LibraryViewAlbum::sig_disc_pressed, this, &GUI_LocalLibrary::disc_pressed);
	connect(ui->lv_album, &LibraryViewAlbum::sig_import_files, this, &GUI_LocalLibrary::import_files);
	connect(ui->lv_album, &LibraryView::sig_merge, library, &LocalLibrary::merge_albums);

	connect(ui->lv_artist, &LibraryView::sig_import_files, this, &GUI_LocalLibrary::import_files);
	connect(ui->lv_artist, &LibraryView::sig_merge, library, &LocalLibrary::merge_artists);
	connect(ui->tb_title, &LibraryView::sig_import_files, this, &GUI_LocalLibrary::import_files);
	connect(ui->lv_genres, &QAbstractItemView::clicked, this, &GUI_LocalLibrary::genre_selection_changed);
	connect(ui->lv_genres, &QAbstractItemView::activated, this, &GUI_LocalLibrary::genre_selection_changed);
	connect(ui->lv_genres, &LibraryGenreView::sig_progress, this, &GUI_LocalLibrary::progress_changed);

	connect(_local_library_menu, &LocalLibraryMenu::sig_reload_library, this, &GUI_LocalLibrary::reload_library_requested);
	connect(_local_library_menu, &LocalLibraryMenu::sig_import_file, this, &GUI_LocalLibrary::import_files_requested);
	connect(_local_library_menu, &LocalLibraryMenu::sig_import_folder, this, &GUI_LocalLibrary::import_dirs_requested);
	connect(_local_library_menu, &LocalLibraryMenu::sig_info, this, &GUI_LocalLibrary::show_info_box);
	connect(_local_library_menu, &LocalLibraryMenu::sig_libpath_clicked, this, &GUI_LocalLibrary::set_library_path_clicked);
	connect(ui->btn_reload_library, &QPushButton::clicked, this, &GUI_LocalLibrary::reload_library_requested);

	connect(ui->splitter_artist_album, &QSplitter::splitterMoved, this, &GUI_LocalLibrary::splitter_artist_moved);
	connect(ui->splitter_tracks, &QSplitter::splitterMoved, this, &GUI_LocalLibrary::splitter_tracks_moved);
	connect(ui->splitter_genre, &QSplitter::splitterMoved, this, &GUI_LocalLibrary::splitter_genre_moved);

	connect(library, &LocalLibrary::sig_no_library_path, this, &GUI_LocalLibrary::lib_no_lib_path);
	connect(library, &LocalLibrary::sig_import_dialog_requested, this, &GUI_LocalLibrary::import_dialog_requested);

	setAcceptDrops(true);

	QTimer::singleShot(0, library, SLOT(load()));

	if(ui->lv_genres->get_row_count() <= 1){
		ui->stacked_genre_widget->setCurrentIndex(1);
	}

	else{
		ui->stacked_genre_widget->setCurrentIndex(0);
	}

	REGISTER_LISTENER(Set::Lib_Path, _sl_libpath_changed);
}


GUI_LocalLibrary::~GUI_LocalLibrary()
{
	if(ui)
	{
		delete ui; ui = nullptr;
	}
}


QComboBox* GUI_LocalLibrary::get_libchooser() const
{
	return ui->combo_lib_chooser;
}

QMenu* GUI_LocalLibrary::get_menu() const
{
	return _local_library_menu;
}

void GUI_LocalLibrary::showEvent(QShowEvent* e)
{
	GUI_AbstractLibrary::showEvent(e);

	QByteArray artist_splitter_state, track_splitter_state, genre_splitter_state;

	artist_splitter_state = _settings->get(Set::Lib_SplitterStateArtist);
	track_splitter_state = _settings->get(Set::Lib_SplitterStateTrack);
	genre_splitter_state = _settings->get(Set::Lib_SplitterStateGenre);

	if(!artist_splitter_state.isEmpty()){
		ui->splitter_artist_album->restoreState(artist_splitter_state);
	}

	if(!track_splitter_state.isEmpty()){
		ui->splitter_tracks->restoreState(track_splitter_state);
	}

	if(!genre_splitter_state.isEmpty()){
		ui->splitter_genre->restoreState(genre_splitter_state);
	}
}

void GUI_LocalLibrary::init_shortcuts()
{
	ui->le_search->setShortcutEnabled(QKeySequence::Find, true);

	new QShortcut(QKeySequence(Qt::Key_Escape), this, SLOT(clear_button_pressed()), nullptr, Qt::WidgetWithChildrenShortcut);
	new QShortcut(QKeySequence::Find, ui->le_search, SLOT(setFocus()), nullptr, Qt::WindowShortcut);
}


Library::ReloadQuality GUI_LocalLibrary::show_quality_dialog()
{
	QStringList lst;
	bool ok = false;

	lst << tr("Check for changed files (fast)") + "\t";
	lst << tr("Deep scan (slow)") + "\t";

	QString str = QInputDialog::getItem(this,
						  "Sayonara",
						  tr("Select reload mode") + "\n",
						  lst,
						  0,
						  false,
						  &ok);

	if(!ok){
		return Library::ReloadQuality::Unknown;
	}

	if(str.isEmpty()){
		return Library::ReloadQuality::Unknown;
	}

	if(str.compare(lst.first()) == 0){
		return Library::ReloadQuality::Fast;
	}

	if(str.compare(lst.first()) == 0){
		return Library::ReloadQuality::Accurate;
	}

	return Library::ReloadQuality::Unknown;
}


void GUI_LocalLibrary::language_changed() {

	ui->retranslateUi(this);

	GUI_AbstractLibrary::language_changed();
}


void GUI_LocalLibrary::_sl_libpath_changed()
{
	QString library_path = _settings->get(Set::Lib_Path);
	if(!library_path.isEmpty()){
		ui->stacked_widget->setCurrentIndex(0);
	}

	else{
		ui->stacked_widget->setCurrentIndex(1);
	}

	ui->combo_searchfilter->setVisible(!library_path.isEmpty());
	ui->le_search->setVisible(!library_path.isEmpty());
	ui->btn_clear->setVisible(!library_path.isEmpty());
}


void GUI_LocalLibrary::genre_selection_changed(const QModelIndex& index){
	QVariant data = index.data();
	ui->combo_searchfilter->setCurrentIndex(1);
	ui->le_search->setText(data.toString());
	text_line_edited(data.toString());
}



Library::TrackDeletionMode GUI_LocalLibrary::show_delete_dialog(int n_tracks) {

	QMessageBox dialog(this);
	QAbstractButton* clicked_button;
	QPushButton* only_library_button;

	dialog.setFocus();
	dialog.setIcon(QMessageBox::Warning);
	dialog.setText("<b>" + Lang::get(Lang::Warning) + "!</b>");
	dialog.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
	only_library_button = dialog.addButton(tr("Only from library"), QMessageBox::AcceptRole);
	dialog.setDefaultButton(QMessageBox::No);
	QString info_text = tr("You are about to delete %1 files").arg(n_tracks);

	dialog.setInformativeText(info_text + "\n" + Lang::get(Lang::Continue).question() );

	int answer = dialog.exec();
	clicked_button = dialog.clickedButton();
	dialog.close();


	if(answer == QMessageBox::No){
		return Library::TrackDeletionMode::None;
	}

	if(answer == QMessageBox::Yes){
		return Library::TrackDeletionMode::AlsoFiles;
	}

	if(clicked_button->text() == only_library_button->text()) {
		return Library::TrackDeletionMode::OnlyLibrary;
	}

	return Library::TrackDeletionMode::None;
}


void GUI_LocalLibrary::disc_pressed(int disc) {
	LocalLibrary* ll = dynamic_cast<LocalLibrary*>(_library);
	ll->psl_disc_pressed(disc);
}


void GUI_LocalLibrary::lib_no_lib_path(){

	Message::warning(tr("Please select your library path first and reload again."));

	QString dir = QFileDialog::getExistingDirectory(this, Lang::get(Lang::OpenDir),	QDir::homePath(), QFileDialog::ShowDirsOnly);

	if(dir.size() > 3){
		_settings->set(Set::Lib_Path, dir);
	}
}

void GUI_LocalLibrary::progress_changed(const QString& type, int progress)
{

	ui->pb_progress->setVisible(progress >= 0);
	ui->lab_progress->setVisible(progress >= 0);

	ui->lab_progress->setText(type);

	if(progress == 0){
		if(ui->pb_progress->maximum() != 0){
			ui->pb_progress->setMaximum(0);
		}
	}

	if(progress > 0){
		if(ui->pb_progress->maximum() != 100){
			ui->pb_progress->setMaximum(100);
		}

		ui->pb_progress->setValue(progress);
	}
}


void GUI_LocalLibrary::reload_library_requested()
{

	Library::ReloadQuality quality = show_quality_dialog();
	if(quality == Library::ReloadQuality::Unknown){
		return;
	}

	_library->psl_reload_library(false, quality);
	ui->btn_reload_library->setEnabled(false);
}


void GUI_LocalLibrary::reload_finished()
{
	ui->btn_reload_library->setEnabled(true);
	ui->lv_genres->reload_genres();

	if(ui->lv_genres->get_row_count() <= 1){
		ui->stacked_genre_widget->setCurrentIndex(1);
	}
	else{
		ui->stacked_genre_widget->setCurrentIndex(0);
	}
}

void GUI_LocalLibrary::show_info_box()
{
	if(!_library_info_box){
		_library_info_box = new GUI_LibraryInfoBox(this);
	}

	_library_info_box->psl_refresh();
}

#include <QListView>
#include <QTreeView>
#include <QStandardPaths>
void GUI_LocalLibrary::import_dirs_requested()
{
	QStringList dirs;

	QFileDialog* dialog = new QFileDialog(this);
	dialog->setDirectory(QDir::homePath());
	dialog->setWindowTitle(Lang::get(Lang::ImportDir));
	dialog->setFileMode(QFileDialog::DirectoryOnly);
	dialog->setOption(QFileDialog::DontUseNativeDialog, true);
	QList<QUrl> sidebar_urls = dialog->sidebarUrls();

	QList<QStandardPaths::StandardLocation> locations;
	locations << QStandardPaths::HomeLocation;
	locations << QStandardPaths::DesktopLocation;
	locations << QStandardPaths::DownloadLocation;
	locations << QStandardPaths::MusicLocation;
	locations << QStandardPaths::TempLocation;

	for(QStandardPaths::StandardLocation location : locations)
	{
		QStringList std_locations = QStandardPaths::standardLocations(location);
		for(const QString& std_location : std_locations){
			QUrl url = QUrl::fromLocalFile(std_location);
			if(sidebar_urls.contains(url)){
				continue;
			}

			sidebar_urls << url;
		}
	}

	dialog->setSidebarUrls(sidebar_urls);

	QListView* list_view = dialog->findChild<QListView*>("listView");
	if(list_view == nullptr)
	{
		delete dialog;

		QString dir = QFileDialog::getExistingDirectory(this, Lang::get(Lang::ImportDir),
					QDir::homePath(), QFileDialog::ShowDirsOnly);
		if(!dir.isEmpty()){
			dirs << dir;
		}
	}

	else{
		list_view->setSelectionMode(QAbstractItemView::MultiSelection);
		QTreeView* tree_view = dialog->findChild<QTreeView*>();
		if(tree_view){
			tree_view->setSelectionMode(QAbstractItemView::MultiSelection);
		}

		if(dialog->exec() == QFileDialog::Accepted){
			dirs = dialog->selectedFiles();
		}
	}

	if(!dirs.isEmpty()){
		_library->import_files(dirs);
	}
}

void GUI_LocalLibrary::import_files_requested()
{
	QStringList extensions = Helper::get_soundfile_extensions();
	QString filter = QString("Soundfiles (") + extensions.join(" ") + ")";
	QStringList files = QFileDialog::getOpenFileNames(this, Lang::get(Lang::ImportFiles),
					QDir::homePath(), filter);

	if(files.size() > 0) {
		_library->import_files(files);
	}
}


void GUI_LocalLibrary::import_files(const QStringList& files){
	_library->import_files(files);
}


void GUI_LocalLibrary::import_dialog_requested(){

	if(!_ui_importer){
		_ui_importer = new GUI_ImportFolder(this, true);
	}

	_ui_importer->show();
}


void GUI_LocalLibrary::set_library_path_clicked() {

	QString start_dir = QDir::homePath();
	QString old_dir = _settings->get(Set::Lib_Path);

	if (old_dir.size() > 0 && QFile::exists(old_dir)) {
		start_dir = old_dir;
	}

	QString dir = QFileDialog::getExistingDirectory(this, Lang::get(Lang::OpenDir),
			old_dir, QFileDialog::ShowDirsOnly);

	if(dir.isEmpty()){
		return;
	}

	if(old_dir.compare(dir) == 0) {
		return;
	}

	_settings->set(Set::Lib_Path, dir);

	GlobalMessage::Answer answer = Message::question_yn(tr("Do you want to reload the Library?"), "Library");

	if(answer == GlobalMessage::Answer::No){
		return;
	}

	Library::ReloadQuality quality = show_quality_dialog();
	if(quality == Library::ReloadQuality::Unknown){
		return;
	}

	_library->psl_reload_library(false, quality);
}


void GUI_LocalLibrary::splitter_artist_moved(int pos, int idx)
{
	Q_UNUSED(pos)
	Q_UNUSED(idx)

	QByteArray arr = ui->splitter_artist_album->saveState();
	_settings->set(Set::Lib_SplitterStateArtist, arr);
}

void GUI_LocalLibrary::splitter_tracks_moved(int pos, int idx)
{
	Q_UNUSED(pos)
	Q_UNUSED(idx)

	QByteArray arr = ui->splitter_tracks->saveState();
	_settings->set(Set::Lib_SplitterStateTrack, arr);
}

void GUI_LocalLibrary::splitter_genre_moved(int pos, int idx)
{
	Q_UNUSED(pos)
	Q_UNUSED(idx)

	QByteArray arr = ui->splitter_genre->saveState();
	_settings->set(Set::Lib_SplitterStateGenre, arr);
}
