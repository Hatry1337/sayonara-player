/* LibraryItemModelTracks.cpp */

/* Copyright (C) 2011-2016 Lucio Carreras
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
 * LibraryItemModelTracks.cpp
 *
 *  Created on: Apr 24, 2011
 *      Author: Lucio Carreras
 */


#include "LibraryItemModelTracks.h"
#include "GUI/Library/Helper/ColumnHeader.h"

#include "Helper/Helper.h"



LibraryItemModelTracks::LibraryItemModelTracks() :
	LibraryItemModel()
{

}


LibraryItemModelTracks::~LibraryItemModelTracks() {

}


QVariant LibraryItemModelTracks::data(const QModelIndex &index, int role) const{

	int row = index.row();
	int col = index.column();

	if (!index.isValid())
		return QVariant();

	if (row >= _tracks.size())
		return QVariant();

	int idx_col = col;

	if (role == Qt::DisplayRole || role==Qt::EditRole) {

		const MetaData& md = _tracks.at(row);

		switch(idx_col) {
			case COL_TRACK_NUM:
				return QVariant( md.track_num );

			case COL_TITLE:
				return QVariant( md.title );

			case COL_ARTIST:
				return QVariant( md.artist );

			case COL_LENGTH:
				return QVariant( Helper::cvt_ms_to_string(md.length_ms) );

			case COL_ALBUM:
				return QVariant(md.album);

			case COL_YEAR:
				return QVariant(md.year);

			case COL_BITRATE:
				return QVariant(md.bitrate);

			case COL_FILESIZE:
				return QVariant(md.filesize);
			case COL_TRACK_RATING:
				return QVariant(md.rating);
			default:
				return QVariant();
		}
	}

	else if (role == Qt::TextAlignmentRole) {

		if (idx_col == COL_TRACK_NUM || idx_col == COL_BITRATE || idx_col == COL_LENGTH || idx_col == COL_YEAR || idx_col == COL_FILESIZE /*|| idx_col == COL_DISCNUMBER*/)
		{
			return Qt::AlignRight + Qt::AlignVCenter;
		}

		else return Qt::AlignLeft + Qt::AlignVCenter;
	}

	else{
		return QVariant();
	}
}


Qt::ItemFlags LibraryItemModelTracks::flags(const QModelIndex &index = QModelIndex()) const{

	if (!index.isValid())
		return Qt::ItemIsEnabled;

	int idx_column = index.column();
	int shown_col = idx_column;
	if(shown_col == COL_TRACK_RATING) {

		return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
	}

	return QAbstractItemModel::flags(index);
}

bool LibraryItemModelTracks::setData(const QModelIndex &index, const QVariant &value, int role) {

	if(!index.isValid()){
		return false;
	}

	if (role == Qt::DisplayRole || role == Qt::EditRole) {

		int row = index.row();
		int col = index.column();

		if(col == COL_TRACK_RATING) {
			_tracks[row].rating = value.toInt();
		}

		else{

			if(!MetaData::fromVariant(value, _tracks[row])) {
				return false;
			}
		}

		emit dataChanged(index, this->index(row, _header_names.size() - 1));

		return true;
	}

	return false;
}

bool LibraryItemModelTracks::setData(const QModelIndex&index, const MetaDataList&v_md, int role)
{
	if(!index.isValid()){
		return false;
	}

	if (role == Qt::EditRole || role == Qt::DisplayRole) {

		int row = index.row();

		_tracks = v_md;

		emit dataChanged(index, this->index(row + v_md.size() - 1, _header_names.size() - 1));

		return true;
	}

	return false;
}


int LibraryItemModelTracks::get_id_by_row(int row)
{
	if(row < 0 || row >= _tracks.size()){
		return -1;
	}

	else {
		return _tracks[row].id;
	}
}



QModelIndex	LibraryItemModelTracks::getFirstRowIndexOf(QString substr) {
	if(_tracks.isEmpty()) {
		return this->index(-1, -1);
	}

	return getNextRowIndexOf(substr, 0);
}

QModelIndex LibraryItemModelTracks::getNextRowIndexOf(QString substr, int row, const QModelIndex& parent) {

	Q_UNUSED(parent)

	int len = _tracks.size();
	if(_tracks.isEmpty()) {
		return this->index(-1, -1);
	}

	for(int i=0; i< len; i++) {
		int row_idx = (i + row) % len;
		QString title = _tracks[row_idx].title;
		if(title.startsWith(substr, Qt::CaseInsensitive)) {
			return this->index(row_idx, 0);
		}
	}

	return this->index(-1, -1);
}

QModelIndex LibraryItemModelTracks::getPrevRowIndexOf(QString substr, int row, const QModelIndex& parent) {

	Q_UNUSED(parent)

	int len = _tracks.size();
	if(len < row) row = len - 1;
	for(int i=0; i< len; i++) {
		if(row - i < 0) row = len - 1;
		int row_idx = (row - i) % len;
		QString title = _tracks[row_idx].title;
		if(title.startsWith(substr, Qt::CaseInsensitive)) {
			return this->index(row_idx, 0);
		}
	}

	return this->index(-1, -1);
}

