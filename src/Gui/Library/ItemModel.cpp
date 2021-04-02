/* LibraryItemModel.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#include "ItemModel.h"

#include "Components/Library/AbstractLibrary.h"
#include "Components/Covers/CoverLocation.h"
#include "Gui/Library/Header/ColumnHeader.h"
#include "Gui/Utils/MimeData/CustomMimeData.h"

#include "Utils/Algorithm.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"

#include <QUrl>

using namespace Library;

struct ItemModel::Private
{
	AbstractLibrary* library = nullptr;
    QStringList headerNames;
	int oldRowCount;

    Private(int columnCount, AbstractLibrary* library) :
		library(library),
        oldRowCount(0)
    {
        for(int i=0; i<columnCount; i++)
        {
            headerNames << QString();
        }
    }
};

ItemModel::ItemModel(int columnCount, QObject* parent, AbstractLibrary* library) :
	SearchableTableModel(parent)
{
    m = Pimpl::make<ItemModel::Private>(columnCount, library);
}

ItemModel::~ItemModel() = default;

QVariant ItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return ((role == Qt::DisplayRole) && Util::between(section, m->headerNames))
            ? m->headerNames[section]
            : SearchableTableModel::headerData(section, orientation, role);
}

bool ItemModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant& value, int role)
{
    if(role == Qt::DisplayRole && Util::between(section, m->headerNames))
    {
        m->headerNames[section] = value.toString();
        emit headerDataChanged(orientation, section, section);
        return true;
    }

    return QAbstractItemModel::setHeaderData(section, orientation, value, role);
}

int ItemModel::columnCount(const QModelIndex&) const
{
	return m->headerNames.size();
}

bool ItemModel::removeRows(int row, int count, const QModelIndex& index)
{
	Q_UNUSED(index)

	beginRemoveRows(QModelIndex(), row, row + count - 1);
	m->oldRowCount -= count;
	endRemoveRows();

	return true;
}

bool ItemModel::insertRows(int row, int count, const QModelIndex& index)
{
	Q_UNUSED(index)

	beginInsertRows(QModelIndex(), row, row + count - 1);
	m->oldRowCount += count;
	endInsertRows();

	return true;
}

void ItemModel::refreshData(int* rowCountBefore, int* rowCountNew)
{
	int oldSize = m->oldRowCount;
	int newSize = rowCount();

	if(rowCountBefore != nullptr)
	{
		*rowCountBefore = oldSize;
	}

	if(rowCountNew != nullptr)
	{
		*rowCountNew = newSize;
	}

	if(oldSize > newSize)
	{
		removeRows(newSize, oldSize - newSize);
	}

	else if(oldSize < newSize)
	{
		insertRows(oldSize, newSize - oldSize);
	}

	emit dataChanged(index(0, 0), index(rowCount() - 1, columnCount() - 1));
}

QMimeData* ItemModel::mimeData(const QModelIndexList& indexes) const
{
	auto* mimedata = new Gui::CustomMimeData(this);

	const auto tracks = selectedMetadata();
	mimedata->setMetadata(tracks);

	Util::Set<int> rows;
	for(const auto& modelIndex : indexes)
	{
		rows.insert(modelIndex.row());
	}

	const auto coverLocation = this->cover(indexes);
	mimedata->setCoverUrl(coverLocation.preferredPath());

	return mimedata;
}

QModelIndexList ItemModel::searchResults(const QString& substr)
{
	QModelIndexList ret;

	int len = rowCount();
	if(len == 0)
	{
		return QModelIndexList();
	}

	for(int i = 0; i < len; i++)
	{
		QString title = searchableString(i);
		title = Library::Utils::convertSearchstring(title, searchMode());

		if(title.contains(substr))
		{
			ret << this->index(i, searchableColumn());
		}
	}

	return ret;
}

AbstractLibrary* ItemModel::library()
{
	return m->library;
}

const AbstractLibrary* ItemModel::library() const
{
	return m->library;
}
