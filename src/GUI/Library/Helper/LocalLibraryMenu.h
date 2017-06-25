/* LocalLibraryMenu.h */

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

#ifndef LOCALLIBRARYMENU_H
#define LOCALLIBRARYMENU_H

#include <QMenu>
#include <QAction>

#include "Helper/Settings/SayonaraClass.h"
#include "Helper/Pimpl.h"

class QString;
class IconLoader;
class LocalLibraryMenu :
		public QMenu,
		private SayonaraClass
{
	Q_OBJECT
	PIMPL(LocalLibraryMenu)

signals:
	void sig_reload_library();
	void sig_import_file();
	void sig_import_folder();
	void sig_info();
	void sig_name_changed(const QString& name);
	void sig_path_changed(const QString& path);
	void sig_show_album_artists_changed();

public:
	explicit LocalLibraryMenu(const QString& name, const QString& path, QWidget* parent=nullptr);
	virtual ~LocalLibraryMenu();

	void refresh_name(const QString& name);
	void refresh_path(const QString& path);

private slots:
	void show_album_cover_view_changed();
	void show_album_artists_changed();
	void language_changed();
	void skin_changed();

	void realtime_search_changed();
	void auto_update_changed();

	void edit_clicked();
	void edit_accepted();
};

#endif // LOCALLIBRARYMENU_H
