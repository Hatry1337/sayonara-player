/* LibraryItemModelAlbums.cpp */

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
 * LibraryItemModelAlbums.cpp
 *
 *  Created on: Apr 26, 2011
 *      Author: Lucio Carreras
 */

#include "LibraryItemModelAlbums.h"

#include "Components/Library/AbstractLibrary.h"
#include "Components/Covers/CoverLocation.h"

#include "GUI/Library/Helper/ColumnIndex.h"
#include "GUI/Helper/GUI_Helper.h"

#include "Helper/Helper.h"
#include "Helper/MetaData/Album.h"
#include "Helper/Language.h"
#include "Helper/Set.h"

#include <QPixmap>

struct LibraryItemModelAlbums::Private
{
	QPixmap		pm_single;
    QPixmap		pm_multi;

    Private() :
        pm_single(GUI::get_pixmap("cd.png", QSize(14, 14))),
        pm_multi(GUI::get_pixmap("cds.png", QSize(16, 16)))
    {}
};

LibraryItemModelAlbums::LibraryItemModelAlbums(QObject* parent, AbstractLibrary* library) :
    LibraryItemModel(parent, library)
{
	m = Pimpl::make<LibraryItemModelAlbums::Private>();
}

LibraryItemModelAlbums::~LibraryItemModelAlbums() {}

int LibraryItemModelAlbums::get_id_by_row(int row)
{
    const AlbumList& albums = library()->albums();

    if(row < 0 || row >= albums.count()){
		return -1;
	}

	else {
        return albums[row].id;
	}
}

QString LibraryItemModelAlbums::get_string(int row) const
{
    const AlbumList& albums = library()->albums();

    if(row < 0 || row >= albums.count()){
		return QString();
	}

	else {
        return albums[row].name;
	}
}


CoverLocation LibraryItemModelAlbums::get_cover(const IndexSet& indexes) const
{
	if(indexes.isEmpty() || indexes.size() > 1){
		return CoverLocation();
	}

	int idx = indexes.first();
    const AlbumList& albums = library()->albums();
    if(idx < 0 || idx > albums.count()){
		return CoverLocation();
	}

    const Album& album = albums[idx];

	return CoverLocation::get_cover_location(album);
}


QVariant LibraryItemModelAlbums::data(const QModelIndex& index, int role) const
{
	if (!index.isValid())
		return QVariant();

    const AlbumList& albums = library()->albums();
    if (index.row() >= albums.count())
		return QVariant();

	int row = index.row();
	int column = index.column();
	ColumnIndex::Album col = (ColumnIndex::Album) column;

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

	else if(role == Qt::DisplayRole || role==Qt::EditRole)
	{
		switch(col) {
			case ColumnIndex::Album::NumSongs:
                return  QString::number(album.num_songs) + " " +
                        Lang::get(Lang::Tracks).toLower();

			case ColumnIndex::Album::Year:
				if(album.year == 0){
					return Lang::get(Lang::None);
				}
				return album.year;

			case ColumnIndex::Album::Name:
                if(album.name.trimmed().isEmpty()){
                    return Lang::get(Lang::None);
                }
				return album.name;

			case ColumnIndex::Album::Duration:
				return Helper::cvt_ms_to_string(album.length_sec * 1000, true, false);

			case ColumnIndex::Album::Rating:
				return album.rating;

			default: return QVariant();
		}
	}

	return QVariant();
}

bool LibraryItemModelAlbums::setData(const QModelIndex & index, const QVariant & value, int role)
{
    if(!index.isValid()){
        return false;
    }

    if (role == Qt::EditRole || role == Qt::DisplayRole)
    {
        int row = index.row();
        int col = index.column();

        if(col == (int) ColumnIndex::Album::Rating)
        {
            library()->change_album_rating(row, value.toInt());
            emit dataChanged(index, this->index(row, columnCount() - 1));

            return true;
        }
    }

    return false;
}

int LibraryItemModelAlbums::rowCount(const QModelIndex&) const
{
    return library()->albums().count();
}


Qt::ItemFlags LibraryItemModelAlbums::flags(const QModelIndex & index) const
{
	if (!index.isValid()) {
		return Qt::ItemIsEnabled;
	}

	int col = index.column();

	if(col == (int) ColumnIndex::Album::Rating){
		return (QAbstractItemModel::flags(index) | Qt::ItemIsEditable);
	}

	return QAbstractItemModel::flags(index);
}


int LibraryItemModelAlbums::get_searchable_column() const
{
    return (int) ColumnIndex::Album::Name;
}


const IndexSet& LibraryItemModelAlbums::selections() const
{
    return library()->selected_albums();
}
