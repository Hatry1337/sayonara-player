/* View.h */

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
 * MyListView.h
 *
 *  Created on: Jun 26, 2011
 *      Author: Lucio Carreras
 */

#ifndef ITEM_VIEW_H_
#define ITEM_VIEW_H_

#include "GUI/Utils/Widgets/WidgetTemplate.h"
#include "GUI/Utils/Widgets/Dragable.h"
#include "GUI/Utils/SearchableWidget/SearchableView.h"

#include "GUI/InfoDialog/InfoDialogContainer.h"
#include "GUI/Library/Models/ItemModel.h"

#include "Utils/MetaData/MetaDataFwd.h"
#include "Utils/typedefs.h"
#include "Utils/Set.h"
#include "Utils/Pimpl.h"

class LibraryContextMenu;
class ColumnHeaderList;
class QStringList;
class QMenu;

namespace Library
{
	class ItemView :
			public Gui::WidgetTemplate<SearchableTableView>,
			public InfoDialogContainer,
			protected Dragable
	{
		Q_OBJECT
		PIMPL(ItemView)

	protected:
		ItemModel* _model=nullptr;

	signals:
		void sig_middle_button_clicked();
		void sig_all_selected();
		void sig_delete_clicked();
		void sig_play_next_clicked();
		void sig_append_clicked();
		void sig_refresh_clicked();
		void sig_import_files(const QStringList& files);
		void sig_double_clicked(const IndexSet& indexes);
		void sig_sel_changed(const IndexSet& indexes);
		void sig_merge(const SP::Set<Id>& ids, int target_id);

	private:
		ItemView(const ItemView& other)=delete;
		ItemView& operator =(const ItemView& other)=delete;

	public:
		explicit ItemView(QWidget* parent=nullptr);
		virtual ~ItemView();

		using QTableView::setModel;
		virtual void setModel(ItemModel* model);

		void show_context_menu_actions(int entries);
		void add_context_action(QAction* action);
		void remove_context_action(QAction* action);

		/** Dragable **/
		QMimeData* dragable_mimedata() const override;
		QPixmap pixmap() const override;

		void set_metadata_interpretation(MD::Interpretation type);
		void set_selection_type(SelectionViewInterface::SelectionType type) override;

		void show_clear_button(bool visible);
		void use_clear_button(bool yesno);

		bool is_valid_drag_position(const QPoint &p) const override;


	protected:
		// Events implemented in LibraryViewEvents.cpp
		virtual void mousePressEvent(QMouseEvent* event) override;
		virtual void mouseMoveEvent(QMouseEvent* event) override;
		virtual void keyPressEvent(QKeyEvent* event) override;
		virtual void contextMenuEvent(QContextMenuEvent* event) override;

		virtual void dragEnterEvent(QDragEnterEvent *event) override;
		virtual void dragMoveEvent(QDragMoveEvent *event) override;
		virtual void dropEvent(QDropEvent* event) override;
		virtual void changeEvent(QEvent* event) override;
		virtual void resizeEvent(QResizeEvent *event) override;

		virtual void selectionChanged (const QItemSelection& selected, const QItemSelection& deselected ) override;
		virtual void init_context_menu();
		LibraryContextMenu* context_menu() const;

		// InfoDialogContainer
		virtual MD::Interpretation metadata_interpretation() const override final;
		MetaDataList info_dialog_data() const override;

		virtual void selection_changed(const IndexSet& indexes);

	protected slots:
		virtual void context_menu_show(const QPoint&);
		virtual void merge_action_triggered();
		virtual void play_next_clicked();
		virtual void delete_clicked();
		virtual void middle_clicked();
		virtual void append_clicked();
		virtual void refresh_clicked();


	public:
		template < typename T, typename ModelType >
		void fill(const T& input_data)
		{
			int old_size, new_size;
			_model->refresh_data(&old_size, &new_size);

			IndexSet selections;
			for(int row=0; row < new_size; row++)
			{
				if(_model->is_selected(input_data[row].id)){
					selections.insert(row);
				}
			}

			select_rows(selections, 0, _model->columnCount() - 1);

			if(new_size > old_size) {
				resize_rows_to_contents(old_size, new_size - old_size);
			}
		}

		void resize_rows_to_contents();
		void resize_rows_to_contents(int first_row, int count);
	};
}

#endif /* ITEM_VIEW_H_ */
