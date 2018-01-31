/* ArtistView.cpp */

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



#include "ArtistView.h"

#include "Components/Library/AbstractLibrary.h"
#include "GUI/Library/Models/ArtistModel.h"
#include "GUI/Library/Utils/ColumnIndex.h"
#include "GUI/Library/Utils/ColumnHeader.h"
#include "GUI/Utils/Delegates/StyledItemDelegate.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Library/Sorting.h"


using namespace Library;

struct ArtistView::Private
{
	AbstractLibrary* library=nullptr;
};

ArtistView::ArtistView(QWidget* parent) :
	Library::TableView(parent)
{
	m = Pimpl::make<Private>();
}

ArtistView::~ArtistView() {}

void ArtistView::init_view(AbstractLibrary* library)
{
	m->library = library;

	ArtistModel* artist_model = new ArtistModel(this, m->library);

	this->set_item_model(artist_model);
	this->set_search_model(artist_model);
	this->setItemDelegate(new Gui::StyledItemDelegate(this));
	this->set_metadata_interpretation(MD::Interpretation::Artists);

	connect(this, &TableView::doubleClicked, this, &ArtistView::double_clicked);
	connect(m->library, &AbstractLibrary::sig_all_artists_loaded, this, &ArtistView::artists_ready);

	Set::listen(Set::Lib_UseViewClearButton, this, &ArtistView::use_clear_button_changed);
}

ColumnHeaderList ArtistView::column_headers() const
{
	ColumnHeaderList columns;

	ColumnHeader* ar_h0 = new ColumnHeader(ColumnHeader::Sharp, true, SortOrder::NoSorting, SortOrder::NoSorting, 20);
	ColumnHeader* ar_h1 = new ColumnHeader(ColumnHeader::Artist, false, SortOrder::ArtistNameAsc, SortOrder::ArtistNameDesc, 1.0, 160 );
	ColumnHeader* ar_h2 = new ColumnHeader(ColumnHeader::NumTracks, true, SortOrder::ArtistTrackcountAsc, SortOrder::ArtistTrackcountDesc, 80);

	columns << ar_h0 << ar_h1 << ar_h2;

	return columns;
}

BoolList ArtistView::shown_columns() const
{
	BoolList columns = _settings->get(Set::Lib_ColsArtist);
	columns[0] = false;
	return columns;
}

SortOrder ArtistView::sortorder() const
{
	Library::Sortings so = _settings->get(Set::Lib_Sorting);
	return so.so_artists;
}

void ArtistView::selection_changed(const IndexSet& indexes)
{
	TableView::selection_changed(indexes);
	m->library->selected_artists_changed(indexes);
}

void ArtistView::merge_action_triggered() {}

void ArtistView::play_clicked()
{
	TableView::play_clicked();
	emit doubleClicked(QModelIndex());
}

void ArtistView::play_new_tab_clicked()
{
	TableView::play_new_tab_clicked();
	m->library->prepare_fetched_tracks_for_playlist(true);
}

void ArtistView::play_next_clicked()
{
	TableView::play_next_clicked();
	m->library->play_next_fetched_tracks();
}

void ArtistView::middle_clicked()
{
	TableView::middle_clicked();
	m->library->prepare_fetched_tracks_for_playlist(true);
}

void ArtistView::append_clicked()
{
	TableView::append_clicked();
	m->library->append_fetched_tracks();
}

void ArtistView::refresh_clicked()
{
	TableView::refresh_clicked();
	m->library->refresh_artist();
}

void ArtistView::columns_changed()
{
	TableView::columns_changed();
	_settings->set(Set::Lib_ColsArtist, this->shown_columns());
}

void ArtistView::sortorder_changed(SortOrder s)
{
	TableView::sortorder_changed(s);
	m->library->change_artist_sortorder(s);
}

void ArtistView::double_clicked(const QModelIndex& index)
{
	Q_UNUSED(index)
	m->library->prepare_fetched_tracks_for_playlist(false);
}

void ArtistView::artists_ready()
{
	const ArtistList& artists = m->library->artists();
	this->fill<ArtistList, ArtistModel>(artists);
}

void ArtistView::use_clear_button_changed()
{
	bool b = _settings->get(Set::Lib_UseViewClearButton);
	use_clear_button(b);
}
