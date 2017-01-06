/* DateSearchModel.h */

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



#ifndef DATESEARCHMODEL_H
#define DATESEARCHMODEL_H

#include "GUI/Helper/SearchableWidget/AbstractSearchModel.h"

#include "Helper/Pimpl.h"

namespace Library {class DateFilter;}

class DateSearchModel :
	public AbstractSearchListModel
{
    Q_OBJECT

public:
	DateSearchModel(QObject* parent=nullptr);
	~DateSearchModel();

public:
	int rowCount(const QModelIndex& parent=QModelIndex()) const override;
	QVariant data(const QModelIndex& index, int role=Qt::DisplayRole) const override;

	void set_data(const Library::DateFilter filter, int idx);
	void add_data(const Library::DateFilter filter);
	void remove(int idx);

public:
	QModelIndex getFirstRowIndexOf(const QString& substr) override;
	QModelIndex getNextRowIndexOf(const QString& substr, int cur_row, const QModelIndex& parent=QModelIndex()) override;
	QModelIndex getPrevRowIndexOf(const QString& substr, int cur_row, const QModelIndex& parent=QModelIndex()) override;
	QMap<QChar, QString> getExtraTriggers();

	Library::DateFilter get_filter(int row) const;

private:
	PIMPL(DateSearchModel)
};

#endif // DATESEARCHMODEL_H
