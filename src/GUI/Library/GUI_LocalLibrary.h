/* GUI_LocalLibrary.h */

/* Copyright (C) 2011-2017 Lucio Carreras
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
 * GUI_LocalLibrary.h
 *
 *  Created on: Apr 24, 2011
 *      Author: Lucio Carreras
 */

#ifndef GUI_LOCAL_LIBRARY_H_
#define GUI_LOCAL_LIBRARY_H_

#include "GUI_AbstractLibrary.h"
#include "Utils/Pimpl.h"
#include "Utils/Library/LibraryNamespaces.h"

class GUI_LibraryInfoBox;
class GUI_ImportFolder;
class QLabel;

UI_FWD(GUI_LocalLibrary)

namespace Library
{
	class LocalLibraryMenu;
	class CoverView;
	class CoverModel;

	class GUI_LocalLibrary :
			public GUI_AbstractLibrary
	{
		Q_OBJECT
		UI_CLASS(GUI_LocalLibrary)
		PIMPL(GUI_LocalLibrary)

	public:

		explicit GUI_LocalLibrary(LibraryId id, QWidget* parent=nullptr);
		virtual ~GUI_LocalLibrary();

		QMenu*		menu() const;
		QFrame*		header_frame() const;


	protected:
		void showEvent(QShowEvent* e) override;

		Library::TableView* lv_artist() const override;
		Library::TableView* lv_album() const override;
		Library::TableView* lv_tracks() const override;

		QLineEdit* le_search() const override;
		QList<Library::Filter::Mode> search_options() const override;

		void language_changed() override;
		void search_esc_pressed() override;


	private slots:
		void switch_album_view();

		void disc_pressed(int disc);
		void progress_changed(const QString& type, int progress);

		void genres_reloaded();
		void genre_selection_changed(const QModelIndex& index);

		void reload_library_requested();
		void reload_library_requested(Library::ReloadQuality quality);
		void reload_library_accepted(Library::ReloadQuality quality);
		void reload_finished();

		void import_dirs_requested();
		void import_files_requested();
		void import_files(const QStringList& files);
		void change_library_name(const QString& name);
		void change_library_path(const QString& path);
		void name_changed(LibraryId id);
		void path_changed(LibraryId id);

		// importer requests dialog
		void import_dialog_requested(const QString& target_dir);

		void splitter_artist_moved(int pos, int idx);
		void splitter_tracks_moved(int pos, int idx);
		void splitter_genre_moved(int pos, int idx);

		// reimplemented from Abstract Library
		Library::TrackDeletionMode show_delete_dialog(int n_tracks) override;

		void show_info_box();

	private:
		void init_album_cover_view();
	};
}

#endif /* GUI_LocalLibrary_H_ */

