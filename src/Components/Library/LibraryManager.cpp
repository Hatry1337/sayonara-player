/* LibraryManager.cpp */

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



#include "LibraryManager.h"
#include "Interfaces/LibraryInterface/LibraryPluginHandler.h"
#include "Helper/Library/LibraryInfo.h"
#include "Helper/Helper.h"
#include "Helper/FileHelper.h"
#include "Helper/Settings/Settings.h"
#include "LocalLibrary.h"

#include <QDir>
#include <QFile>
#include <QMap>
#include <QObject>
#include <QString>


struct LibraryManager::Private
{
	QList<LibraryInfo> all_libs;
	QMap<qint8, LocalLibrary*> lib_map;
	LibraryPluginHandler* lph=nullptr;

	Private()
	{
		lph = LibraryPluginHandler::getInstance();
	}

	bool contains_path(const QString& path) const
	{
		for(const LibraryInfo& info : all_libs){
			//QString info_path = info.path();
			if(path.compare(info.path(), Qt::CaseInsensitive) == 0){
				return true;
			}
		}

		return false;
	}

	void rename_library(qint8 library_id, const QString& name)
	{
		for(int i=0; i<all_libs.size(); i++)
		{
			const LibraryInfo& info = all_libs[i];

			if(info.id() != library_id){
				continue;
			}

			if(info.name() == name){
				break;
			}

			QFile::remove(info.symlink_path());

			all_libs[i] = LibraryInfo(name, info.path(), info.id());
			lib_map[library_id]->set_library_name(name);

			Helper::File::create_symlink(all_libs[i].path(), all_libs[i].symlink_path());
			break;
		}

		lph->rename_local_library(library_id, name);
	}

	void change_library_path(qint8 library_id, const QString& new_path)
	{
		for(int i=0; i<all_libs.size(); i++)
		{
			LibraryInfo info = all_libs[i];

			if(info.id() != library_id){
				continue;
			}

			if(info.path() == new_path){
				break;
			}

			LibraryInfo new_info(info.name(), new_path, info.id());
			all_libs[i] = new_info;

			LocalLibrary* library = lib_map[info.id()];
			library->set_library_path(new_path);

			QFile::remove(info.symlink_path());
			Helper::File::create_symlink(new_info.path(), new_info.symlink_path());

			break;
		}
	}

	int get_next_id() const
	{
		qint8 id=0;
		QList<qint8> ids;
		for(const LibraryInfo& li : all_libs){
			ids << li.id();
		}

		while(ids.contains(id)){
			id++;
		}

		return id;
	}

	LocalLibrary* get_library(qint8 library_id)
	{
		for(const LibraryInfo& li : all_libs)
		{
			if(li.id() == library_id){
				if(lib_map.contains(library_id)){
					return lib_map[library_id];
				}

				else {
					LocalLibrary* lib = new LocalLibrary(library_id, li.name(), li.path());
					lib_map[library_id] = lib;
					return lib;
				}
			}
		}

		return nullptr;
	}

	LibraryInfo get_library_info(qint8 id)
	{
		for(const LibraryInfo& li : all_libs)
		{
			if(li.id() == id){
				return li;
			}
		}

		return LibraryInfo();
	}

	void move_library(int row, int new_row)
	{
		all_libs.move(row, new_row);
	}

	void init_symlinks()
	{

		QString dir = Helper::sayonara_path("Libraries");
		QDir d(dir);

		QFileInfoList symlinks = d.entryInfoList(QDir::NoFilter);
		for(const QFileInfo& symlink : symlinks)
		{
			if(symlink.isSymLink()) {
				QFile::remove(symlink.absoluteFilePath());
			}
		}

		Helper::File::create_directories(dir);

		for(const LibraryInfo& info : all_libs)
		{
			QString target = info.symlink_path();

			if(!(QFile::exists(target))){
				Helper::File::create_symlink(info.path(), target);
			}
		}
	}
};


LibraryManager::LibraryManager() :
	SayonaraClass()
{
	_m = Pimpl::make<Private>();

	revert();
	_m->init_symlinks();
}

LibraryManager::~LibraryManager() {}

qint8 LibraryManager::add_library(const QString& name, const QString& path)
{
	if(path.isEmpty() || name.isEmpty()){
		return -1;
	}

	if(_m->contains_path(path)){
		return -1;
	}

	qint8 id = _m->get_next_id();
	LibraryInfo li(name, path, id);

	_m->all_libs << li;
	_m->lph->add_local_library(li);

	_settings->set(Set::Lib_AllLibraries, _m->all_libs);

	Helper::File::create_symlink(li.path(), li.symlink_path());

	return id;
}

void LibraryManager::rename_library(qint8 id, const QString& new_name)
{
	_m->rename_library(id, new_name);
	_settings->set(Set::Lib_AllLibraries, _m->all_libs);
}

void LibraryManager::remove_library(qint8 id)
{
	for(int i=0; i<_m->all_libs.size(); i++)
	{
		if(_m->all_libs[i].id() != id) {
			continue;
		}

		_m->lph->remove_local_library(id);

		LibraryInfo info = _m->all_libs.takeAt(i);
		QFile::remove(info.symlink_path());


		LocalLibrary* library = _m->lib_map.take(info.id());

		if(!_m->lib_map.contains(info.id())){
			library = nullptr;
		}

		if(library){
			library->clear_library();
			delete library; library=nullptr;
		}

		_settings->set(Set::Lib_AllLibraries, _m->all_libs);

		break;
	}
}

void LibraryManager::move_library(int old_row, int new_row)
{
	_m->move_library(old_row, new_row);

	_m->lph->move_local_library(old_row, new_row);
	_settings->set(Set::Lib_AllLibraries, _m->all_libs);
}

void LibraryManager::change_library_path(qint8 id, const QString& path)
{
	_m->change_library_path(id, path);
	_settings->set(Set::Lib_AllLibraries, _m->all_libs);
}


QString LibraryManager::request_library_name(const QString& path)
{
	QDir d(path);
	return Helper::cvt_str_to_first_upper(d.dirName());
}

QList<LibraryInfo> LibraryManager::all_libraries() const
{
	return _m->all_libs;
}

int LibraryManager::count() const
{
	return _m->all_libs.size();
}

LibraryInfo LibraryManager::library_info(qint8 id) const
{
	return _m->get_library_info(id);
}

LocalLibrary* LibraryManager::library_instance(qint8 id) const
{
	return _m->get_library(id);
}




void LibraryManager::revert()
{
	_m->all_libs = _settings->get(Set::Lib_AllLibraries);
	QList<int> invalid;
	for(int i=_m->all_libs.size() - 1; i>=0; i--){
		if(!_m->all_libs[i].valid()){
			_m->all_libs.removeAt(i);
		}
	}

	if(_m->all_libs.isEmpty()) {
		QString old_path = _settings->get(Set::Lib_Path);
		if(!old_path.isEmpty()) {
			LibraryInfo li("Local Library", old_path, 0);
			_m->all_libs << li;
			_settings->set(Set::Lib_AllLibraries, _m->all_libs);
		}
	}
}

