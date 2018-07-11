/* TrackView.cpp */

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

#include "TrackView.h"

#include "GUI/Library/TrackModel.h"
#include "GUI/Library/RatingDelegate.h"
#include "GUI/Library/Utils/ColumnHeader.h"
#include "GUI/Library/Utils/ColumnIndex.h"

#include "Components/Library/AbstractLibrary.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Library/Sortorder.h"
#include "Utils/Library/Sorting.h"

using namespace Library;

struct TrackView::Private
{
	AbstractLibrary* library = nullptr;
};

TrackView::TrackView(QWidget* parent) :
	Library::TableView(parent)
{
	m = Pimpl::make<Private>();
}

TrackView::~TrackView() {}

AbstractLibrary* TrackView::library() const
{
	return m->library;
}

void TrackView::init_view(AbstractLibrary* library)
{
	m->library = library;

	TrackModel* track_model = new TrackModel(this, library);
	RatingDelegate* track_delegate = new RatingDelegate(this, (int) ColumnIndex::Track::Rating, true);

	this->set_item_model(track_model);
	this->setItemDelegate(track_delegate);
	this->set_metadata_interpretation(MD::Interpretation::Tracks);

	connect(library, &AbstractLibrary::sig_all_tracks_loaded, this, &TrackView::tracks_ready);
}


ColumnHeaderList TrackView::column_headers() const
{
	ColumnHeaderList columns;

	columns << std::make_shared<ColumnHeader>(ColumnHeader::Sharp, true, SortOrder::TrackNumAsc, SortOrder::TrackNumDesc, 25);
	columns << std::make_shared<ColumnHeader>(ColumnHeader::Title, false, SortOrder::TrackTitleAsc, SortOrder::TrackTitleDesc, 0.4, 200);
	columns << std::make_shared<ColumnHeader>(ColumnHeader::Artist, true, SortOrder::TrackArtistAsc, SortOrder::TrackArtistDesc, 0.3, 160);
	columns << std::make_shared<ColumnHeader>(ColumnHeader::Album, true, SortOrder::TrackAlbumAsc, SortOrder::TrackAlbumDesc, 0.3, 160);
	columns << std::make_shared<ColumnHeader>(ColumnHeader::Year, true, SortOrder::TrackYearAsc, SortOrder::TrackYearDesc, 50);
	columns << std::make_shared<ColumnHeader>(ColumnHeader::DurationShort, true, SortOrder::TrackLenghtAsc, SortOrder::TrackLengthDesc, 50);
	columns << std::make_shared<ColumnHeader>(ColumnHeader::Bitrate, true, SortOrder::TrackBitrateAsc, SortOrder::TrackBitrateDesc, 75);
	columns << std::make_shared<ColumnHeader>(ColumnHeader::Filesize, true, SortOrder::TrackSizeAsc, SortOrder::TrackSizeDesc, 75);
	columns << std::make_shared<ColumnHeader>(ColumnHeader::Rating, true, SortOrder::TrackRatingAsc, SortOrder::TrackRatingDesc, 80);

	return columns;
}

BoolList TrackView::visible_columns() const
{
	return _settings->get<Set::Lib_ColsTitle>();
}

void TrackView::save_visible_columns(const BoolList& lst)
{
	_settings->set<Set::Lib_ColsTitle>(lst);
}

SortOrder TrackView::sortorder() const
{
	Sortings so = _settings->get<Set::Lib_Sorting>();
	return so.so_tracks;
}

void TrackView::save_sortorder(SortOrder s)
{
	m->library->change_track_sortorder(s);
}

void TrackView::selection_changed(const IndexSet& lst)
{
	TableView::selection_changed(lst);
	m->library->selected_tracks_changed(lst);
}


void TrackView::play_clicked()
{
	m->library->prepare_current_tracks_for_playlist(false);
}

void TrackView::play_new_tab_clicked()
{
	TableView::play_new_tab_clicked();
	m->library->prepare_current_tracks_for_playlist(true);
}


void TrackView::play_next_clicked()
{
	TableView::play_next_clicked();
	m->library->play_next_current_tracks();
}

void TrackView::append_clicked()
{
	TableView::append_clicked();
	m->library->append_current_tracks();
}

void TrackView::refresh_clicked()
{
	TableView::refresh_clicked();
	m->library->refresh_tracks();
}

void TrackView::tracks_ready()
{
	const MetaDataList& v_md = m->library->tracks();

	this->fill<MetaDataList, TrackModel>(v_md);
}
