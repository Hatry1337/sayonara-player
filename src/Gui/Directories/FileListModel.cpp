/* FileListModel.cpp */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#include "FileListModel.h"
#include "Components/Directories/DirectoryReader.h"
#include "Components/LibraryManagement/LibraryManager.h"

#include "Utils/Utils.h"
#include "Utils/Algorithm.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Library/LibraryInfo.h"

#include "Gui/Utils/CustomMimeData.h"
#include "Gui/Utils/MimeDataUtils.h"
#include "Gui/Utils/Icons.h"

#include <QVariant>
#include <QModelIndex>
#include <QMimeData>
#include <QUrl>
#include <QIcon>
#include <QDir>

namespace Algorithm=Util::Algorithm;

struct FileListModel::Private
{
	QString		parent_directory;

	LibraryId	library_id;
	QStringList files;

	Private() :
		library_id(-1)
	{}
};


FileListModel::FileListModel(QObject* parent) :
	SearchableListModel(parent)
{
	m = Pimpl::make<Private>();
}

FileListModel::~FileListModel() = default;

void FileListModel::set_parent_directory(LibraryId id, const QString& dir)
{
	int old_rowcount = rowCount();

	m->files.clear();
	m->library_id = id;
	m->parent_directory = dir;

	QStringList extensions;
	extensions << Util::soundfile_extensions();
	extensions << Util::playlist_extensions();
	extensions << "*";

	DirectoryReader reader;
	reader.set_filter(extensions);
	reader.scan_files(QDir(dir), m->files);

	if(m->files.size() > old_rowcount)
	{
		beginInsertRows(QModelIndex(), old_rowcount, m->files.size());
		endInsertRows();
	}

	else if(m->files.size() < old_rowcount)
	{
		beginRemoveRows(QModelIndex(), m->files.size(), old_rowcount);
		endRemoveRows();
	}

	Algorithm::sort(m->files, [](const QString& f1, const QString& f2)
	{
		bool is_soundfile1 = Util::File::is_soundfile(f1);
		bool is_soundfile2 = Util::File::is_soundfile(f2);

		bool is_playlistfile1 = Util::File::is_playlistfile(f1);
		bool is_playlistfile2 = Util::File::is_playlistfile(f2);

		bool is_imagefile1 = Util::File::is_imagefile(f1);
		bool is_imagefile2 = Util::File::is_imagefile(f2);

		if(is_soundfile1 && is_soundfile2){
			return (f1.toLower() < f2.toLower());
		}

		if(is_soundfile1 && !is_soundfile2){
			return true;
		}

		if(!is_soundfile1 && is_soundfile2){
			return false;
		}

		if(is_playlistfile1 && is_playlistfile2){
			return (f1.toLower() < f2.toLower());
		}

		if(is_playlistfile1 && !is_playlistfile2){
			return true;
		}

		if(!is_playlistfile1 && is_playlistfile2){
			return false;
		}

		if(is_imagefile1 && is_imagefile2){
			return (f1.toLower() < f2.toLower());
		}

		if(is_imagefile1 && !is_imagefile2){
			return true;
		}

		if(!is_imagefile1 && is_imagefile2){
			return false;
		}

		return (f1.toLower() < f2.toLower());

	});

	emit dataChanged(
			index(0,0),
			index(m->files.size() - 1, 0)
	);
}

QString FileListModel::parent_directory() const
{
	return m->parent_directory;
}


QStringList FileListModel::files() const
{
	return m->files;
}

QModelIndexList FileListModel::search_results(const QString& substr)
{
	QModelIndexList ret;

	for(int i=0; i<m->files.size(); i++)
	{

		if(check_row_for_searchstring(i, substr)){
			ret << index(i, 0);
		}
	}

	return ret;
}


LibraryId FileListModel::library_id() const
{
	return m->library_id;
}

int FileListModel::rowCount(const QModelIndex &parent) const
{
	Q_UNUSED(parent)

	return m->files.size();
}

QVariant FileListModel::data(const QModelIndex &index, int role) const
{
	int row = index.row();
	if(!Util::between(row, m->files)) {
		return QVariant();
	}

	QString filename = m->files[row];

	using namespace Util;

	switch(role)
	{
		case Qt::DisplayRole:
			return File::get_filename_of_path(filename);
		case Qt::DecorationRole:
			if(File::is_soundfile(filename))
			{
				return Gui::Icons::icon(Gui::Icons::AudioFile);
			}

			if(File::is_playlistfile(filename)){
				return Gui::Icons::icon(Gui::Icons::PlaylistFile);
			}

			if(File::is_imagefile(filename))
			{
				return Gui::Icons::icon(Gui::Icons::ImageFile);
			}

			return QIcon();

		case Qt::UserRole:
			return filename;

		default:
			return QVariant();
	}
}

bool FileListModel::check_row_for_searchstring(int row, const QString& substr) const
{
	QString converted_string = Library::Utils::convert_search_string(substr, search_mode());

	QString dirname, filename;
	Util::File::split_filename(m->files[row], dirname, filename);

	QString converted_filepath = Library::Utils::convert_search_string(filename, search_mode());
	return converted_filepath.contains(converted_string);
}

QMimeData* FileListModel::mimeData(const QModelIndexList& indexes) const
{
	QList<QUrl> urls;

	for(const QModelIndex& idx : indexes)
	{
		int row = idx.row();
		if(Util::between(row, m->files)) {
			urls << QUrl::fromLocalFile(m->files[row]);
		}
	}

	if(urls.isEmpty()){
		return nullptr;
	}

	auto* data = new Gui::CustomMimeData(this);
	data->setUrls(urls);

	return data;
}


Qt::ItemFlags FileListModel::flags(const QModelIndex& index) const
{
	if(index.isValid())
	{
		return  Qt::ItemFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsDragEnabled);
	}

	return Qt::NoItemFlags;

}
