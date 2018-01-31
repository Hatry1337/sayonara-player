/* DirectoryReader.cpp */

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

#include "DirectoryReader.h"

#include "Utils/FileUtils.h"
#include "Utils/Utils.h"
#include "Utils/Tagging/Tagging.h"
#include "Utils/Parser/PlaylistParser.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Database/DatabaseConnector.h"
#include "Database/LibraryDatabase.h"

#include <QDirIterator>
#include <QFileInfo>
#include <QStringList>
#include <QDir>

struct DirectoryReader::Private
{
	QStringList		name_filters;

	Private()
	{
		name_filters = Util::soundfile_extensions();
	}
};

DirectoryReader::DirectoryReader ()
{
	m = Pimpl::make<Private>();
}

DirectoryReader::~DirectoryReader () {}

void DirectoryReader::set_filter(const QStringList & filter)
{
	m->name_filters = filter;
}

void DirectoryReader::set_filter(const QString& filter)
{
	m->name_filters.clear();
	m->name_filters << filter;
}


void DirectoryReader::files_in_directory_recursive(const QDir& base_dir_orig, QStringList& files) const
{
	QDir base_dir(base_dir_orig);

	QStringList tmp_files = base_dir.entryList(m->name_filters,
											   (QDir::Filters)(QDir::Files | QDir::NoDotAndDotDot));

	QStringList dirs = base_dir.entryList((QDir::Filters)(QDir::Dirs | QDir::NoDotAndDotDot));

	for(const QString& dir : dirs)
	{
		base_dir.cd(dir);
		files_in_directory_recursive(base_dir, files);
		base_dir.cdUp();
	}

	for(const QString& filename : tmp_files){
		files << base_dir.absoluteFilePath(filename);
	}
}

void DirectoryReader::files_in_directory(const QDir& base_dir, QStringList& files) const
{
	QStringList tmp_files = base_dir.entryList(m->name_filters,
											   (QDir::Filters)(QDir::Files | QDir::NoDotAndDotDot));

	for(const QString& filename : tmp_files){
		files << base_dir.absoluteFilePath(filename);
	}
}


MetaDataList DirectoryReader::metadata_from_filelist(const QStringList& lst)
{
	MetaDataList v_md;
	QStringList sound_files, playlist_files;

	// fetch sound and playlist files
	QStringList filter;
	filter << Util::soundfile_extensions();
	filter << Util::playlist_extensions();

	set_filter(filter);

	for( const QString& str : lst)
	{
		if(!QFile::exists(str)) {
			continue;
		}

		if(Util::File::is_dir(str))
		{
			QStringList files;
			QDir dir(str);
			dir.cd(str);

			files_in_directory_recursive(dir, files);
			for(const QString& file : ::Util::AsConst(files)){
				if(Util::File::is_soundfile(file)){
					sound_files << file;
				}
			}
		}

		else if(Util::File::is_soundfile(str)){
			sound_files << str;
		}

		else if(Util::File::is_playlistfile(str)) {
			playlist_files << str;
		}
	}

	DB::Connector* db = DB::Connector::instance();
	DB::LibraryDatabase* lib_db = db->library_db(-1, 0);

	lib_db->getMultipleTracksByPath(sound_files, v_md);

	auto it=v_md.begin();
	while(it != v_md.end())
	{
		if( it->id < 0 )
		{
			if(!Tagging::Util::getMetaDataOfFile(*it)) {
				it = v_md.erase(it);
				continue;
			}

			it->is_extern = true;
		}

		it++;
	}

	for(const QString& playlist_file : ::Util::AsConst(playlist_files))
	{
		v_md << PlaylistParser::parse_playlist(playlist_file);
	}

	return v_md;
}


QStringList DirectoryReader::find_files_rec(const QDir& dir_orig, const QString& filename)
{
	if(dir_orig.canonicalPath().isEmpty()){
		return QStringList();
	}

	if(filename.isEmpty()){
		return QStringList();
	}

	QDir dir(dir_orig);
	QStringList ret;
	QStringList dirs = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
	QStringList files = dir.entryList(QDir::Files);

	for(const QString& d : dirs)
	{
		if(d.isEmpty()){
			continue;
		}

		if(dir.cd(d)) {
			ret += find_files_rec(dir, filename);
			dir.cdUp();
		}
	}

	for(const QString& file : files)
	{
		if(file.contains(filename)){
			ret += dir.absoluteFilePath(file);
		}
	}

	return ret;
}
