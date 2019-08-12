/* GenreView.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#ifndef LIBRARYGENREVIEW_H
#define LIBRARYGENREVIEW_H

#include "Gui/Utils/Widgets/WidgetTemplate.h"
#include "Gui/Utils/SearchableWidget/SearchableView.h"
#include "Utils/Pimpl.h"

#include <QTreeWidget>

class Genre;
class TreeDelegate;
class LocalLibrary;
class QItemSelection;

namespace Util
{
	template<typename T>
	class Tree;
}

using GenreNode=Util::Tree<QString>;

namespace Library
{
	class GenreView :
			public Gui::WidgetTemplate<QTreeWidget>
	{
		using Parent=Gui::WidgetTemplate<QTreeWidget>;

		Q_OBJECT
		PIMPL(GenreView)

	signals:
		void sig_progress(const QString& name, int progress);
		void sig_selected_changed(const QStringList& genres);
		void sig_invalid_genre_selected();

	private:
		using Parent::activated;
		using Parent::clicked;
		using Parent::pressed;

	public:
		explicit GenreView(QWidget* parent=nullptr);
		~GenreView() override;

		void init(LocalLibrary* library);
		void reload_genres();

		static QString invalid_genre_name();

	private:
		void set_genres(const Util::Set<Genre>& genres);
		void build_genre_data_tree(const Util::Set<Genre>& genres);
		void populate_widget(QTreeWidgetItem* parent_item, GenreNode* node);

		QTreeWidgetItem* find_genre(const QString& genre);

		void init_context_menu();


	private slots:
		void item_expanded(QTreeWidgetItem* item);
		void item_collapsed(QTreeWidgetItem* item);
		void expand_current_item();

		void progress_changed(int progress);
		void update_finished();

		void new_pressed();
		void rename_pressed();
		void delete_pressed();

		void switch_tree_list();

		void selection_changed(const QItemSelection& selected, const QItemSelection& deselected);


	protected:
		void language_changed() override;
		void dragEnterEvent(QDragEnterEvent* e) override;
		void dragMoveEvent(QDragMoveEvent* e) override;
		void dragLeaveEvent(QDragLeaveEvent* e) override;
		void dropEvent(QDropEvent* e) override;
		void contextMenuEvent(QContextMenuEvent* e) override;
	};
}

#endif // LIBRARYGENREVIEW_H
