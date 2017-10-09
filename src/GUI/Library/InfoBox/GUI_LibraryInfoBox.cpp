/* GUILibraryInfoBox.cpp

 * Copyright (C) 2011-2017 Lucio Carreras
 *
 * This file is part of sayonara-player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * created by Lucio Carreras,
 * Sep 2, 2012
 *
 */

#include "GUI_LibraryInfoBox.h"
#include "GUI/Library/ui_GUI_LibraryInfoBox.h"

#include "Components/Library/LibraryManager.h"
#include "Database/DatabaseConnector.h"
#include "Database/LibraryDatabase.h"

#include "Utils/Settings/Settings.h"
#include "Utils/FileUtils.h"
#include "Utils/Utils.h"
#include "Utils/Language.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/MetaDataList.h"

#include "GUI/Utils/GuiUtils.h"

#include <QMap>
#include <QPixmap>


GUI_LibraryInfoBox::GUI_LibraryInfoBox(int8_t library_id, QWidget* parent) :
	Dialog(parent)
{
	ui = new Ui::GUI_LibraryInfoBox();
	ui->setupUi(this);
    ui->lab_icon->setPixmap(
		Gui::Util::pixmap("logo.png", QSize(24,24), true)
    );

	_library_id = library_id;
}

GUI_LibraryInfoBox::~GUI_LibraryInfoBox() {}

void GUI_LibraryInfoBox::language_changed()
{
    ui->retranslateUi(this);

	ui->lab_tracks->setText(Lang::get(Lang::Tracks).toFirstUpper());
	ui->lab_artists->setText(Lang::get(Lang::Artists));
	ui->lab_albums->setText(Lang::get(Lang::Albums));
	ui->lab_duration->setText(Lang::get(Lang::Duration));
	ui->lab_filesize_descr->setText(Lang::get(Lang::Filesize));
	ui->btn_close->setText(Lang::get(Lang::Close));

    LibraryManager* manager = LibraryManager::instance();
    LibraryInfo info = manager->library_info(_library_id);

    ui->lab_name->setText(Lang::get(Lang::Library) + ": " + info.name());

    this->setWindowTitle(Lang::get(Lang::Info));
}

void GUI_LibraryInfoBox::skin_changed()
{
    LibraryManager* manager = LibraryManager::instance();
    LibraryInfo info = manager->library_info(_library_id);
    bool dark = (_settings->get(Set::Player_Style) == 1);

    ui->lab_path->setText(Util::create_link(info.path(), dark));
}


void GUI_LibraryInfoBox::showEvent(QShowEvent *e)
{
    refresh();
    Dialog::showEvent(e);
}


void GUI_LibraryInfoBox::refresh()
{
	DatabaseConnector* db = DatabaseConnector::instance();
	LibraryDatabase* lib_db = db->library_db(_library_id, 0);

	MetaDataList v_md;
	AlbumList v_albums;
	ArtistList v_artists;

	lib_db->getAllTracks(v_md);
	lib_db->getAllAlbums(v_albums);
	lib_db->getAllArtists(v_artists);

	int n_tracks = v_md.size();
	int n_albums = v_albums.size();
	int n_artists = v_artists.size();
	int64_t duration_ms = 0;
	int64_t filesize = 0;

	for( const MetaData& md : v_md ) {
		duration_ms += md.length_ms;
		filesize += md.filesize;
	}

	QString duration_string = Util::cvt_ms_to_string(duration_ms, false);
	QString filesize_str = Util::File::calc_filesize_str(filesize);

	ui->lab_album_count->setText(QString::number(n_albums));
	ui->lab_track_count->setText(QString::number(n_tracks));
	ui->lab_artist_count->setText(QString::number(n_artists));
	ui->lab_duration_value->setText(duration_string + "s");
	ui->lab_filesize->setText(filesize_str);
}
