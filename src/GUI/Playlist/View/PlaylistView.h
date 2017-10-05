/* PlaylistView.h */

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
 * PlaylistView.h
 *
 *  Created on: Jun 27, 2011
 *      Author: Lucio Carreras
 */

#ifndef PLAYLISTVIEW_H_
#define PLAYLISTVIEW_H_

#include "GUI/Helper/SearchableWidget/SearchableView.h"
#include "GUI/Helper/Dragable/Dragable.h"
#include "GUI/InfoDialog/InfoDialogContainer.h"
#include "Helper/Playlist/PlaylistFwd.h"
#include "Helper/MetaData/MetaDataFwd.h"
#include "Helper/Pimpl.h"

#include <QPoint>
#include <QDrag>
#include <QList>

class SayonaraLoadingBar;
class LibraryContextMenu;
class PlaylistItemModel;
class PlaylistItemDelegate;
class BookmarksMenu;

class PlaylistView :
		public SearchableListView,
		public InfoDialogContainer,
		private Dragable
{
	Q_OBJECT
    PIMPL(PlaylistView)

signals:
	void sig_double_clicked(int row);
	void sig_left_tab_clicked();
	void sig_right_tab_clicked();
	void sig_time_changed();
    void sig_delete_tracks(const IndexSet& rows);

public:
	explicit PlaylistView(PlaylistPtr pl, QWidget* parent=nullptr);
	virtual ~PlaylistView();

	void fill(PlaylistPtr pl);

	void goto_row(int row);
	void scroll_up();
	void scroll_down();

	int row_count() const;
	void remove_cur_selected_rows();
	void delete_cur_selected_tracks();

	/**
	 * @brief called from GUI_Playlist when data has not been dropped
	 * directly into the view widget. Insert on first row then
	 * @param event
	 */
	void dropEventFromOutside(QDropEvent* event);


public slots:
	void clear();



private:

    void init_rc_menu();

	// d & d
	void clear_drag_drop_lines(int row);
	int calc_drag_drop_line(QPoint pos);
	void handle_drop(QDropEvent* event);
	void handle_inner_drag_drop(int row, bool copy);


	// overloaded stuff
	void contextMenuEvent(QContextMenuEvent* e) override;

	/**
	 * @brief we start the drag action, all lines has to be cleared
	 * @param the event
	 */
	void dragLeaveEvent(QDragLeaveEvent* event) override;
	void dragEnterEvent(QDragEnterEvent* event) override;
	void dragMoveEvent(QDragMoveEvent* event) override;
	void dropEvent(QDropEvent* event) override;

	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseDoubleClickEvent(QMouseEvent* event) override;

	void keyPressEvent(QKeyEvent *event) override;
	void selectionChanged ( const QItemSelection & selected, const QItemSelection & deselected ) override;

	MD::Interpretation metadata_interpretation() const override;
	MetaDataList info_dialog_data() const override;
	QMimeData* get_mimedata() const override;


private slots:
	void handle_async_drop(bool success);
	void rating_changed(int rating);

	// SayonaraSelectionView interface
public:
	int get_index_by_model_index(const QModelIndex& idx) const override;
	QModelIndex get_model_index_by_index(int idx) const override;
};

#endif /* PlaylistView_H_ */
