/* LibraryItemModelAlbums.cpp */

/* Copyright (C) 2011-2019 Lucio Carreras
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
 * LibraryItemModelAlbums.cpp
 *
 *  Created on: Apr 26, 2011
 *      Author: Lucio Carreras
 */

#include "AlbumModel.h"

#include "Components/Library/AbstractLibrary.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Tagging/UserTaggingOperations.h"

#include "Gui/Library/Header/ColumnIndex.h"
#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Icons.h"

#include "Utils/Utils.h"
#include "Utils/MetaData/Album.h"
#include "Utils/Language/Language.h"
#include "Utils/Set.h"

#include <QImage>
#include <QColor>

using namespace Library;

struct AlbumModel::Private
{
	QImage						pm_single;
	QImage						pm_multi;
	QPair<int, Rating>			tmp_rating;
	Tagging::UserOperations*	uto=nullptr;

	Private() :
		pm_single(Gui::Util::image("cd.png", Gui::Util::NoTheme, QSize(14, 14))),
		pm_multi(Gui::Util::image("cds.png", Gui::Util::NoTheme, QSize(16, 16)))
	{
		tmp_rating.first = -1;
	}
};

AlbumModel::AlbumModel(QObject* parent, AbstractLibrary* library) :
	ItemModel(parent, library)
{
	m = Pimpl::make<AlbumModel::Private>();

	connect(library, &AbstractLibrary::sig_album_rating_changed, this, &AlbumModel::rating_changed);
}

AlbumModel::~AlbumModel() = default;

Id AlbumModel::id_by_index(int index) const
{
	const AlbumList& albums = library()->albums();

	if(index < 0 || index >= albums.count()){
		return -1;
	}

	else {
		return albums[index].id;
	}
}

QString AlbumModel::searchable_string(int row) const
{
	const AlbumList& albums = library()->albums();

	if(row < 0 || row >= albums.count()){
		return QString();
	}

	else {
		return albums[row].name();
	}
}


Cover::Location AlbumModel::cover(const IndexSet& indexes) const
{
	if(indexes.isEmpty() || indexes.size() > 1){
		return Cover::Location();
	}

	int idx = indexes.first();
	const AlbumList& albums = library()->albums();
	if(idx < 0 || idx > albums.count()){
		return Cover::Location();
	}

	const Album& album = albums[idx];

	return Cover::Location::xcover_location(album);
}


QVariant AlbumModel::data(const QModelIndex& index, int role) const
{
	if (!index.isValid()) {
		return QVariant();
	}

	const AlbumList& albums = library()->albums();
	if (index.row() >= albums.count())
		return QVariant();

	int row = index.row();
	int column = index.column();
	ColumnIndex::Album col = scast(ColumnIndex::Album, column);

	const Album& album = albums[row];

	if(role == Qt::TextAlignmentRole )
	{
		int alignment = Qt::AlignVCenter;
		switch(col)
		{
			case ColumnIndex::Album::Name:
				alignment |= Qt::AlignLeft;
				break;
			default:
				alignment |= Qt::AlignRight;
		}

		return alignment;
	}

	else if(role == Qt::TextColorRole)
	{
		if(col == ColumnIndex::Album::MultiDisc)
		{
			return QColor(0, 0, 0);
		}
	}

	else if(role == Qt::DecorationRole)
	{
		if(col == ColumnIndex::Album::MultiDisc)
		{
			if(album.discnumbers.size() > 1){
				return m->pm_multi;
			}

			return m->pm_single;
		}
	}

	else if(role == Qt::DisplayRole || role == Qt::EditRole)
	{
		switch(col)
		{
			case ColumnIndex::Album::NumSongs:
				return QString::number(album.num_songs);

			case ColumnIndex::Album::Year:
				if(album.year == 0){
					return Lang::get(Lang::UnknownYear);
				}
				return album.year;

			case ColumnIndex::Album::Name:
				if(album.name().trimmed().isEmpty()){
					return Lang::get(Lang::UnknownAlbum);
				}
				return album.name();

			case ColumnIndex::Album::Duration:
				return ::Util::cvt_ms_to_string(album.length_sec * 1000, "$He $M:$S");

			case ColumnIndex::Album::Rating:
			{
				if(role == Qt::DisplayRole) {
					return QVariant();
				}

				Rating rating = album.rating;
				if(row == m->tmp_rating.first)
				{
					rating = m->tmp_rating.second;
				}

				return QVariant::fromValue(rating);
			}

			default:
				return QVariant();
		}
	}

	else if(role == Qt::SizeHintRole){
		return QSize(20, 20);
	}

	return QVariant();
}


bool AlbumModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
	if((index.column() != int(ColumnIndex::Album::Rating) ||
	   (role != Qt::EditRole)))
	{
		return false;
	}

	int row = index.row();

	const AlbumList& albums = library()->albums();
	if(row >= 0 && row < albums.count())
	{
		Album album = albums[row];
		Rating rating = value.value<Rating>();

		if(album.rating != rating)
		{
			m->tmp_rating.first = row;
			m->tmp_rating.second = rating;

			if(!m->uto)
			{
				m->uto = new Tagging::UserOperations(-1, this);
			}

			m->uto->set_album_rating(album, rating);
		}
	}

	return false;
}


void AlbumModel::rating_changed(int row)
{
	emit dataChanged(this->index(row, int(ColumnIndex::Album::Rating)), this->index(row, int(ColumnIndex::Album::Rating)));
}


int AlbumModel::rowCount(const QModelIndex&) const
{
	return library()->albums().count();
}


Qt::ItemFlags AlbumModel::flags(const QModelIndex& index) const
{
	if (!index.isValid()) {
		return Qt::ItemIsEnabled;
	}

	int col = index.column();
	if(col == scast(int, ColumnIndex::Album::Rating))
	{
		return (ItemModel::flags(index) | Qt::ItemIsEditable);
	}

	return ItemModel::flags(index);
}


int AlbumModel::searchable_column() const
{
	return scast(int, ColumnIndex::Album::Name);
}


const Util::Set<Id>& AlbumModel::selections() const
{
	return library()->selected_albums();
}


const MetaDataList& Library::AlbumModel::mimedata_tracks() const
{
	return library()->tracks();
}
