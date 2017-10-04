/* AbstractSearchModel.cpp */

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

#include "AbstractSearchModel.h"

SearchModelFunctionality::SearchModelFunctionality() {}
SearchModelFunctionality::~SearchModelFunctionality() {}

QModelIndex SearchModelFunctionality::getFirstRowIndexOf(const QString &substr)
{
    if(!has_items()){
        return QModelIndex();
    }

    return getNextRowIndexOf(substr, 0, QModelIndex());
}

int SearchModelFunctionality::getNumberResults(const QString& str)
{
    if(str.isEmpty()){
        return -1;
    }

	QModelIndex first_idx = this->getFirstRowIndexOf(str);
	QModelIndex found_idx = first_idx;

	int results=1;
	if(!first_idx.isValid()) {
		return 0;
	}

	while(true)
	{
		int row = found_idx.row();
		found_idx = getNextRowIndexOf(str, row + 1);
		bool same_idx = ((found_idx.row() == first_idx.row()) &&
						(found_idx.column() == first_idx.column()));

		if( same_idx || (!found_idx.isValid())) {
			break;
		}

		else {
			results++;
		}
	}

	return results;
}

void SearchModelFunctionality::set_search_mode(Library::SearchModeMask search_mode)
{
	_search_mode = search_mode;
}

Library::SearchModeMask SearchModelFunctionality::search_mode() const
{
	return _search_mode;
}
