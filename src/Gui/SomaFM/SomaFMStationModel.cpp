/* SomaFMStationModel.cpp */

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


/* SomaFMStationModel.cpp */

#include "SomaFMStationModel.h"

#include "Components/Streaming/SomaFM/SomaFMStation.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverFetchManager.h"
#include "Components/Covers/Fetcher/CoverFetcherUrl.h"
#include "Components/Streaming/SomaFM/SomaFMLibrary.h"

#include "Utils/globals.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/MetaData/MetaDataList.h"

#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/MimeDataUtils.h"
#include "Gui/Utils/CustomMimeData.h"
#include "Gui/Utils/Icons.h"

#include <QUrl>
#include <QIcon>

struct SomaFM::StationModel::Private
{
	QList<SomaFM::Station>			stations;
	SomaFM::StationModel::Status	status;
};

SomaFM::StationModel::StationModel(QObject* parent) :
	SearchableTableModel(parent)
{
	m = Pimpl::make<SomaFM::StationModel::Private>();
	m->status = Status::Waiting;
}

SomaFM::StationModel::~StationModel() {}


int SomaFM::StationModel::rowCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)

	return std::max(1, m->stations.size());
}

int SomaFM::StationModel::columnCount(const QModelIndex& parent) const
{
	Q_UNUSED(parent)
	return 2;
}


QVariant SomaFM::StationModel::data(const QModelIndex& index, int role) const
{
	int row = index.row();
	int col = index.column();

	if(!index.isValid())
	{
		spLog(Log::Debug, this) << "Index not valid";
		return QVariant();
	}

	if(role == Qt::TextAlignmentRole)
	{
		return int(Qt::AlignVCenter| Qt::AlignLeft);
	}

	if(row < 0 || row >= rowCount())
	{
		return QVariant();
	}

	if(role == Qt::DecorationRole)
	{
		if(m->status == Status::Waiting){
			return QVariant();
		}

		if(col == 1){
			return QVariant();
		}

		if(m->status == Status::Error){
			return Gui::Icons::icon(Gui::Icons::Undo);
		}

		if(m->stations[row].isLoved()){
			return Gui::Icons::icon(Gui::Icons::Star, Gui::Icons::IconMode::ForceSayonaraIcon);
		}

		return Gui::Icons::icon(Gui::Icons::StarDisabled);
	}

	else if(role == Qt::DisplayRole && col == 1)
	{
		if(m->stations.isEmpty())
		{
			if(m->status == Status::Waiting){
				return Lang::get(Lang::LoadingArg).arg("SomaFM");
			}

			else if(m->status == Status::Error){
				return tr("Cannot fetch stations");
			}

			return QVariant();
		}

		return m->stations[row].name();
	}

	else if(role == Qt::WhatsThisRole)
	{
		if(m->stations.isEmpty()){
			return QVariant();
		}

		return m->stations[row].name();
	}

	else if(role == Qt::SizeHintRole && col == 0)
	{
		return QSize(30, 30);
	}

	return QVariant();
}

QModelIndexList SomaFM::StationModel::searchResults(const QString& substr)
{
	QModelIndexList ret;

	int i = 0;
	for(const SomaFM::Station& station : m->stations)
	{
		QString name = station.name();
		QString desc = station.description();

		QString str = name + desc;

		if(str.contains(substr, Qt::CaseInsensitive)){
			ret << this->index(i, 0);
		}
	}

	return ret;
}


void SomaFM::StationModel::setStations(const QList<SomaFM::Station>& stations)
{
	int n_stations = stations.size();

	if(n_stations == 0){
		m->status = Status::Error;
		emit dataChanged( index(0,0), index(0, 1) );
		return;
	}

	m->status = Status::OK;

	beginRemoveRows(QModelIndex(), 0, rowCount() - 1);
	this->removeRows(0, this->rowCount());
	endRemoveRows();

	this->insertRows(0, n_stations);

	beginInsertRows(QModelIndex(), 0, n_stations - 1);
	m->stations = stations;
	endInsertRows();

	emit dataChanged( index(0, 0), index(n_stations - 1, 1));
}

void SomaFM::StationModel::replaceStation(const SomaFM::Station& station)
{
	for(int i=0; i<m->stations.size(); i++)
	{
		if(station.name() == m->stations[i].name()){
			m->stations[i] = station;

			emit dataChanged(this->index(i, 0), this->index(i, 1));
			return;
		}
	}
}

bool SomaFM::StationModel::hasStations() const
{
	return (m->stations.size() > 0);
}

void SomaFM::StationModel::setWaiting()
{
	m->status = Status::Waiting;
	emit dataChanged( index(0,0), index(0, 1) );
}

QMimeData* SomaFM::StationModel::mimeData(const QModelIndexList& indexes) const
{
	QList<QUrl> urls;
	QString coverUrl, coverHash;

	for(const QModelIndex& idx : indexes)
	{
		if(idx.column() == 0){
			continue;
		}

		int row = idx.row();
		if(!Util::between(row, m->stations)){
			continue;
		}

		const SomaFM::Station station = m->stations[row];
		const Cover::Location cl = station.coverLocation();
		coverHash = cl.hash();

		const QStringList playlistUrls = station.playlists();

		for(const QString& playlistUrl : playlistUrls)
		{
			urls << QUrl(playlistUrl);

			auto searchUrls = cl.searchUrls();

			if(!searchUrls.isEmpty())
			{
				coverUrl = searchUrls.first().url();
			}
		}

		break;
	}

	auto* mimeData = new Gui::CustomMimeData(this);

	mimeData->setCoverUrl(coverUrl);
	mimeData->setUrls(urls);

	return mimeData;
}


Qt::ItemFlags SomaFM::StationModel::flags(const QModelIndex& index) const
{
	switch(m->status)
	{
		case Status::Waiting:
			return (Qt::NoItemFlags);
		case Status::Error:
			if(index.column() == 0){
				return Qt::ItemIsEnabled;
			}
			return (Qt::NoItemFlags);
		default:
			return (Qt::ItemIsSelectable | Qt::ItemIsDragEnabled | Qt::ItemIsEnabled);
	}
}
