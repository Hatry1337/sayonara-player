
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

#include "DirectoryModel.h"

#include "GUI/Utils/SearchableWidget/MiniSearcher.h"

#include "Utils/Set.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Library/SearchMode.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Library/Filter.h"
#include "Utils/MetaData/MetaDataList.h"

#include "Components/Library/LibraryManager.h"
#include "Components/Library/LocalLibrary.h"

#include "Database/LibraryDatabase.h"
#include "Database/DatabaseConnector.h"

#include <QDirIterator>
#include <QPair>


using StringPair=QPair<QString, QString>;

struct DirectoryModel::Private
{
	QStringList	found_strings;
	int			cur_idx;
	bool		search_only_dirs;

	QList<StringPair> all_dirs;
	QList<StringPair> all_files;

	Private()
	{
		search_only_dirs = false;
		cur_idx = -1;
	}
};

DirectoryModel::DirectoryModel(QObject* parent) :
	SearchableModel<QFileSystemModel>(parent)
{
	m = Pimpl::make<Private>();
}

DirectoryModel::~DirectoryModel() {}

void DirectoryModel::search_only_dirs(bool b)
{
	if(b != m->search_only_dirs){
		m->cur_idx = 0;
	}

	m->search_only_dirs = b;
}

void DirectoryModel::create_file_list(const QString& substr)
{
	m->all_files.clear();
	m->all_dirs.clear();

	Library::Filter filter;
	filter.set_filtertext(substr, search_mode());
	filter.set_mode(Library::Filter::Mode::Filename);

	DB::Connector* db = DB::Connector::instance();
	DB::LibraryDatabases library_dbs = db->library_dbs();

	for(DB::LibraryDatabase* lib_db : library_dbs)
	{
		LibraryId lib_id = lib_db->library_id();
		if(lib_id < 0){
			continue;
		}

		Library::Info info = Library::Manager::instance()->library_info(lib_id);

		MetaDataList v_md;
		lib_db->getAllTracksBySearchString(filter, v_md);

		for(const MetaData& md : v_md)
		{
			QString sym_filepath = md.filepath();
			sym_filepath.replace(info.path(), info.symlink_path());

			m->all_files << StringPair(
								sym_filepath,
								QString(sym_filepath).remove(info.symlink_path() + "/")
							);

			QString parent_dir, sym_parent_dir, pure_filename;

			Util::File::split_filename(md.filepath(), parent_dir, pure_filename);
			Util::File::split_filename(sym_filepath, sym_parent_dir, pure_filename);

			bool contains = Util::contains(m->all_dirs, [&sym_parent_dir](const StringPair& sp){
				return (sym_parent_dir.compare(sp.first) == 0);
			});

			if(!contains)
			{
				while(sym_parent_dir != info.symlink_path())
				{
					m->all_dirs << StringPair(
									   sym_parent_dir,
									   QString(sym_parent_dir).remove(info.symlink_path() + "/")
									);

					Util::File::split_filename(QString(sym_parent_dir), sym_parent_dir, pure_filename);
				}
			}
		}
	}

	std::sort(m->all_dirs.begin(), m->all_dirs.end(), [](const StringPair& sp1, const StringPair& sp2){
		return (sp1.first.toLower() < sp2.first.toLower());
	});
}

QModelIndex DirectoryModel::getFirstRowIndexOf(const QString& substr)
{
	m->found_strings.clear();
	m->cur_idx = -1;

	create_file_list(substr);
	if(m->all_files.isEmpty()){
		return QModelIndex();
	}

	QString cvt_search_string = Library::Util::convert_search_string(substr, search_mode());

	auto it_all_dirs = m->all_dirs.begin();

	while(it_all_dirs != m->all_dirs.end())
	{
		QString dir_cvt = Library::Util::convert_search_string(it_all_dirs->second, search_mode());

		if(dir_cvt.contains(cvt_search_string))
		{
			m->found_strings << it_all_dirs->first;
		}

		it_all_dirs++;
	}

	if(!m->search_only_dirs)
	{
		auto it_all_files = m->all_files.begin();

		while(it_all_files != m->all_files.end())
		{
			QString file_cvt = Library::Util::convert_search_string(it_all_files->second, search_mode());
			if(file_cvt.contains(cvt_search_string))
			{
				QString f, d;
				Util::File::split_filename(it_all_files->first, d, f);
				m->found_strings << d;
			}

			it_all_files++;
		}
	}

	QString str;
	if(m->found_strings.size() > 0)
	{
		m->found_strings.removeDuplicates();

		std::sort(m->found_strings.begin(), m->found_strings.end(), [](const QString& str1 , const QString& str2){
			return (str1.toLower() < str2.toLower());
		});

		str = m->found_strings.first();
		m->cur_idx = 0;
	}

	QModelIndex idx = index(str);
	if(canFetchMore(idx)){
		fetchMore(idx);
	}

	return idx;
}


QModelIndex DirectoryModel::getNextRowIndexOf(const QString& substr, int cur_row, const QModelIndex& parent)
{
	Q_UNUSED(substr)
	Q_UNUSED(cur_row)
	Q_UNUSED(parent)

	QString str;

	if(m->cur_idx < 0 || m->found_strings.isEmpty() ){
		return QModelIndex();
	}

	m->cur_idx = (m->cur_idx + 1) % m->found_strings.size();

	str = m->found_strings[m->cur_idx];

	return index(str);
}


QModelIndex DirectoryModel::getPrevRowIndexOf(const QString& substr, int cur_row, const QModelIndex& parent)
{
	Q_UNUSED(substr)
	Q_UNUSED(cur_row)
	Q_UNUSED(parent)

	QString str;

	if(m->cur_idx < 0 ){
		return QModelIndex();
	}

	if(m->cur_idx == 0){
		m->cur_idx = (m->found_strings.size() - 1);
	}

	else {
		m->cur_idx--;
	}

	str = m->found_strings[m->cur_idx];

	return index(str);
}


int DirectoryModel::getNumberResults(const QString& str)
{
	Q_UNUSED(str)
	return m->found_strings.size();
}

Qt::ItemFlags DirectoryModel::flags(const QModelIndex& index) const
{
	if(index.isValid()){
		return (QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled);
	}

	return (QAbstractItemModel::flags(index) & ~Qt::ItemIsDropEnabled);
}

LibraryId DirectoryModel::library_id(const QModelIndex& index) const
{
	QString sympath = filePath(index);
	Library::Info info = Library::Manager::instance()->library_info_by_sympath(sympath);
	return info.id();
}

QString DirectoryModel::filepath_origin(const QModelIndex& index) const
{
	QString sympath = filePath(index);
	Library::Info info = Library::Manager::instance()->library_info_by_sympath(sympath);

	QString ret(sympath);
	ret.replace(info.symlink_path(), info.path());

	return ret;
}

bool DirectoryModel::copy_dirs(const QStringList& source_dirs, const QString& target_dir)
{
	Library::Info target_info = Library::Manager::instance()->library_info_by_path(target_dir);
	LibraryId target_id = target_info.id();

	sp_log(Log::Debug, this) << "Copy files " << source_dirs << " to " << target_dir;
	for(const QString& source_dir : source_dirs)
	{
		Util::File::copy_dir(source_dir, target_dir);
	}

	if(target_id >= 0)
	{
		LocalLibrary* library = Library::Manager::instance()->library_instance(target_id);
		if(library){
			library->reload_library(false, Library::ReloadQuality::Fast);
		}
	}

	return true;
}


bool DirectoryModel::move_dirs(const QStringList& source_dirs, const QString& target_dir)
{
	QString cleaned_target_dir = Util::File::clean_filename(target_dir);

	sp_log(Log::Debug, this) << "Move files " << source_dirs << " to " << cleaned_target_dir;

	MetaDataList v_md, v_md_to_update;
	DB::Connector* db = DB::Connector::instance();
	DB::LibraryDatabase* library_db = db->library_db(-1, db->db_id());
	if(library_db)
	{
		library_db->getAllTracks(v_md);
	}

	for(const QString& source_dir : source_dirs)
	{
		QString cleaned_source_dir = Util::File::clean_filename(source_dir);

		Util::File::move_dir(source_dir, cleaned_target_dir);

		for(MetaData md : v_md)
		{
			QString filepath = Util::File::clean_filename(md.filepath());

			if(filepath.startsWith(cleaned_source_dir + "/"))
			{
				filepath.replace(cleaned_source_dir, cleaned_target_dir);
				md.set_filepath(filepath);
				v_md_to_update << md;
			}
		}
	}

	library_db->updateTracks(v_md_to_update);

	return true;
}


bool DirectoryModel::rename_dir(const QString& source_dir, const QString& target_dir)
{
	QString cleaned_source_dir = Util::File::clean_filename(source_dir);
	QString cleaned_target_dir = Util::File::clean_filename(target_dir);

	bool success = Util::File::rename_dir(source_dir, cleaned_target_dir);

	DB::Connector* db = DB::Connector::instance();
	DB::LibraryDatabase* library_db = db->library_db(-1, db->db_id());
	if(library_db)
	{
		MetaDataList v_md, v_md_to_update;
		library_db->getAllTracks(v_md);
		for(MetaData md : v_md)
		{
			QString filepath = Util::File::clean_filename(md.filepath());

			if(filepath.startsWith(cleaned_source_dir + "/"))
			{
				filepath.replace(cleaned_source_dir, cleaned_target_dir);
				md.set_filepath(filepath);
				v_md_to_update << md;
			}
		}

		library_db->updateTracks(v_md_to_update);
	}

	return success;
}
