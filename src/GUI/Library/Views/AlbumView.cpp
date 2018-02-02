/* LibraryViewAlbum.cpp */

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

#include "AlbumView.h"
#include "GUI/Library/Models/AlbumModel.h"
#include "GUI/Library/Delegates/RatingDelegate.h"
#include "GUI/Library/Utils/DiscPopupMenu.h"
#include "GUI/Library/Utils/ColumnIndex.h"
#include "GUI/Library/Utils/ColumnHeader.h"
#include "Utils/Settings/Settings.h"

#include "Components/Library/AbstractLibrary.h"

#include <QHeaderView>
#include <QVBoxLayout>

using namespace Library;

struct AlbumView::Private
{
	AbstractLibrary*		library=nullptr;
	QList< QList<uint8_t> >	discnumbers;
	DiscPopupMenu*			discmenu=nullptr;
	QPoint					discmenu_point;
};

AlbumView::AlbumView(QWidget* parent) :
	TableView(parent)
{
	m = Pimpl::make<Private>();

	connect(this, &QTableView::clicked, this, &AlbumView::index_clicked);
}

AlbumView::~AlbumView() {}

void AlbumView::init_view(AbstractLibrary* library)
{
	m->library = library;

	AlbumModel* album_model = new AlbumModel(this, m->library);
	RatingDelegate* album_delegate = new RatingDelegate(this, (int) ColumnIndex::Album::Rating, true);

	this->set_item_model(album_model);
	this->set_search_model(album_model);
	this->setItemDelegate(album_delegate);
	this->set_metadata_interpretation(MD::Interpretation::Albums);

	connect(this, &AlbumView::doubleClicked, this, &AlbumView::double_clicked);
	connect(m->library, &AbstractLibrary::sig_all_albums_loaded, this, &AlbumView::albums_ready);

	Set::listen<Set::Lib_UseViewClearButton>(this, &AlbumView::use_clear_button_changed);
}

ColumnHeaderList AlbumView::column_headers() const
{
	ColumnHeaderList album_columns;

	ColumnHeader* al_h0 = new ColumnHeader(ColumnHeader::Sharp, true, SortOrder::NoSorting, SortOrder::NoSorting, 20);
	ColumnHeader* al_h1 = new ColumnHeader(ColumnHeader::Album, false, SortOrder::AlbumNameAsc, SortOrder::AlbumNameDesc, 1.0, 160);
	ColumnHeader* al_h2 = new ColumnHeader(ColumnHeader::Duration, true, SortOrder::AlbumDurationAsc, SortOrder::AlbumDurationDesc, 90);
	ColumnHeader* al_h3 = new ColumnHeader(ColumnHeader::NumTracks, true, SortOrder::AlbumTracksAsc, SortOrder::AlbumTracksDesc, 80);
	ColumnHeader* al_h4 = new ColumnHeader(ColumnHeader::Year, true, SortOrder::AlbumYearAsc, SortOrder::AlbumYearDesc, 50);
	ColumnHeader* al_h5 = new ColumnHeader(ColumnHeader::Rating, true, SortOrder::AlbumRatingAsc, SortOrder::AlbumRatingDesc, 80);

	album_columns  << al_h0 << al_h1 << al_h2 << al_h3 << al_h4 << al_h5;
	return album_columns;
}

BoolList AlbumView::visible_columns() const
{
	return _settings->get<Set::Lib_ColsAlbum>();
}

void AlbumView::save_visible_columns(const BoolList& lst)
{
	_settings->set<Set::Lib_ColsAlbum>(lst);
}

SortOrder AlbumView::sortorder() const
{
	Sortings so = _settings->get<Set::Lib_Sorting>();
	return so.so_albums;
}

void AlbumView::save_sortorder(SortOrder s)
{
	m->library->change_album_sortorder(s);
}

void AlbumView::context_menu_show(const QPoint & p)
{
	delete_discmenu();

	TableView::context_menu_show(p);
}

void AlbumView::index_clicked(const QModelIndex &idx)
{
	if(idx.column() != static_cast<int>(ColumnIndex::Album::MultiDisc))
	{
		return;
	}

	QModelIndexList selections = this->selectionModel()->selectedRows();
	if(selections.size() == 1){
		init_discmenu(idx);
		show_discmenu();
	}
}


/* where to show the popup */
void AlbumView::calc_discmenu_point(QModelIndex idx)
{
	m->discmenu_point = QCursor::pos();

	QRect box = this->geometry();
	box.moveTopLeft(this->parentWidget()->mapToGlobal(box.topLeft()));

	if(!box.contains(m->discmenu_point))
	{
		m->discmenu_point.setX(box.x() + (box.width() * 2) / 3);
		m->discmenu_point.setY(box.y());

		QPoint dmp_tmp = parentWidget()->pos();
		dmp_tmp.setY(dmp_tmp.y() - this->verticalHeader()->sizeHint().height());

		while(idx.row() != indexAt(dmp_tmp).row()){
			  dmp_tmp.setY(dmp_tmp.y() + 10);
			  m->discmenu_point.setY(m->discmenu_point.y() + 10);
		}
	}
}

void AlbumView::init_discmenu(QModelIndex idx)
{
	int row = idx.row();
	QList<Disc> discnumbers;
	delete_discmenu();

	if( !idx.isValid() ||
		(row > m->discnumbers.size()) ||
		(row < 0) )
	{
		return;
	}

	discnumbers = m->discnumbers[row];
	if(discnumbers.size() < 2) {
		return;
	}

	calc_discmenu_point(idx);

	m->discmenu = new DiscPopupMenu(this, discnumbers);

	connect(m->discmenu, &DiscPopupMenu::sig_disc_pressed, this, &AlbumView::sig_disc_pressed);
}


void AlbumView::delete_discmenu()
{
	if(!m->discmenu) {
		return;
	}

	m->discmenu->hide();
	m->discmenu->close();

	disconnect(m->discmenu, &DiscPopupMenu::sig_disc_pressed, this, &AlbumView::sig_disc_pressed);

	m->discmenu->deleteLater();
	m->discmenu = nullptr;
}


void AlbumView::show_discmenu()
{
	if(!m->discmenu) return;

	m->discmenu->popup(m->discmenu_point);
}


void AlbumView::clear_discnumbers()
{
	m->discnumbers.clear();
}

void AlbumView::add_discnumbers(const QList<Disc>& dns)
{
	m->discnumbers << dns;
}

void AlbumView::middle_clicked()
{
	TableView::middle_clicked();
	m->library->prepare_fetched_tracks_for_playlist(true);
}

void AlbumView::play_clicked()
{
	TableView::play_clicked();
	emit doubleClicked(QModelIndex());
}

void AlbumView::play_new_tab_clicked()
{
	TableView::play_new_tab_clicked();
	m->library->prepare_fetched_tracks_for_playlist(true);
}

void AlbumView::play_next_clicked()
{
	TableView::play_next_clicked();
	m->library->play_next_fetched_tracks();
}

void AlbumView::append_clicked()
{
	TableView::append_clicked();
	m->library->append_fetched_tracks();
}

void AlbumView::selection_changed(const IndexSet& indexes)
{
	TableView::selection_changed(indexes);
	m->library->selected_albums_changed(indexes);
}

void AlbumView::refresh_clicked()
{
	TableView::refresh_clicked();
	m->library->refresh_albums();
}

void AlbumView::double_clicked(const QModelIndex& index)
{
	Q_UNUSED(index)
	m->library->prepare_fetched_tracks_for_playlist(false);
}

void AlbumView::albums_ready()
{
	const AlbumList& albums = m->library->albums();

	this->fill<AlbumList, AlbumModel>(albums);
}

void AlbumView::use_clear_button_changed()
{
	bool b = _settings->get<Set::Lib_UseViewClearButton>();
	use_clear_button(b);
}
