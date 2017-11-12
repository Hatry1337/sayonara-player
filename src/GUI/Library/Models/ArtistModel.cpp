/* LibraryItemModelArtists.cpp */

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
 * LibraryItemModelArtists.cpp
 *
 *  Created on: Apr 26, 2011
 *      Author: Lucio Carreras
 */

#include "ArtistModel.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/Library/AbstractLibrary.h"

#include "GUI/Library/Utils/ColumnIndex.h"
#include "GUI/Utils/GuiUtils.h"

#include "Utils/MetaData/Artist.h"
#include "Utils/Language.h"
#include "Utils/Set.h"

#include <QPixmap>

using namespace Library;

struct ArtistModel::Private
{
	QPixmap		pm_single;
    QPixmap		pm_multi;

    Private() :
		pm_single(Gui::Util::pixmap("play", QSize(16, 16))),
		pm_multi(Gui::Util::pixmap("sampler", QSize(16, 16)))
    {}
};

ArtistModel::ArtistModel(QObject* parent, AbstractLibrary* library) :
    ItemModel(parent, library)
{
	m = Pimpl::make<ArtistModel::Private>();
}

ArtistModel::~ArtistModel() {}

int ArtistModel::id_by_index(int row)
{
    const ArtistList& artists = library()->artists();

    if(row < 0 || row >= artists.count()){
		return -1;
	}

	else {
        return artists[row].id;
	}
}

QString ArtistModel::get_string(int row) const
{
    const ArtistList& artists = library()->artists();

    if(row < 0 || row >= artists.count()){
		return QString();
	}

	else {
		return artists[row].name();
	}
}


QVariant ArtistModel::data(const QModelIndex & index, int role) const
{
    if (!index.isValid()) {
		return QVariant();
    }

    const ArtistList& artists = library()->artists();
    if (index.row() >= artists.count()) {
		return QVariant();
    }

	int row = index.row();
	int col = index.column();

	ColumnIndex::Artist idx_col = (ColumnIndex::Artist) col;
    const Artist& artist = artists[row];


	if(role == Qt::TextAlignmentRole)
	{
		switch(idx_col) {
			case ColumnIndex::Artist::Name:
				return (int) (Qt::AlignLeft | Qt::AlignVCenter);
			default:
				return (int) (Qt::AlignRight | Qt::AlignVCenter);
		}
	}

	else if(role == Qt::DecorationRole)
	{
        if(idx_col == ColumnIndex::Artist::NumAlbums)
        {
			if(artist.num_albums > 1){
				return m->pm_multi;
			}

			return m->pm_single;
		}
	}

	else if(role == Qt::DisplayRole)
	{
		switch(idx_col)
		{
			case ColumnIndex::Artist::Name:
				if(artist.name().isEmpty()){
					return "None";
				}
				return artist.name();

			case ColumnIndex::Artist::Tracks:
				return QString::number(artist.num_songs) + " " + Lang::get(Lang::Tracks);

			default:
				return QVariant();
		}
	}

    return QVariant();
}

int ArtistModel::rowCount(const QModelIndex&) const
{
    return library()->artists().count();
}


Qt::ItemFlags ArtistModel::flags(const QModelIndex& index) const
{
    if (!index.isValid()){
        return Qt::ItemIsEnabled;
    }

	return QAbstractItemModel::flags(index);
}


Cover::Location ArtistModel::cover(const IndexSet& indexes) const
{
	if(indexes.isEmpty() || indexes.size() > 1){
        return Cover::Location();
	}

    const ArtistList& artists = library()->artists();
	int idx = indexes.first();

    if(idx < 0 || idx > artists.count()){
        return Cover::Location();
	}

    const Artist& artist = artists[idx];
    return Cover::Location::cover_location(artist);
}


int ArtistModel::get_searchable_column() const
{
    return (int) ColumnIndex::Artist::Name;
}


const IndexSet& ArtistModel::selections() const
{
    return library()->selected_artists();
}
