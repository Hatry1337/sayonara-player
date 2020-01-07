/* ImportCache.cpp */

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

#include "ImportCache.h"
#include "Components/LibraryManagement/LibraryManager.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Tagging/Tagging.h"

#include <QHash>
#include <QString>
#include <QStringList>

using Library::ImportCache;

struct ImportCache::Private
{
	QString					library_path;
	MetaDataList			v_md;
	QHash<QString, MetaData> src_md_map;
	QHash<QString, QString>	src_dst_map;
	QStringList				files;

	Private(const QString& library_path) :
		library_path(library_path)
	{}

	Private(const Private& other) :
		CASSIGN(library_path),
		CASSIGN(v_md),
		CASSIGN(src_md_map),
		CASSIGN(src_dst_map),
		CASSIGN(files)
	{}

	Private& operator=(const Private& other)
	{
		ASSIGN(library_path);
		ASSIGN(v_md);
		ASSIGN(src_md_map);
		ASSIGN(src_dst_map);
		ASSIGN(files);

		return *this;
	}
};

ImportCache::ImportCache(const QString& library_path)
{
	m = Pimpl::make<Private>(library_path);
}

ImportCache::ImportCache(const ImportCache& other)
{
	m = Pimpl::make<Private>(*(other.m));
}

ImportCache::~ImportCache() = default;

ImportCache& ImportCache::operator=(const ImportCache& other)
{
	*m = *(other.m);

	return *this;
}

void ImportCache::clear()
{
	m->files.clear();
	m->v_md.clear();
	m->src_dst_map.clear();
}

void ImportCache::add_soundfile(const QString& filename)
{
	MetaData md(filename);
	bool success = Tagging::Utils::getMetaDataOfFile(md);
	if(success)
	{
		m->v_md << md;
		m->src_md_map[md.filepath()] = md;
	}
}

void ImportCache::add_file(const QString& filename)
{
	add_file(filename, QString());
}

void ImportCache::add_file(const QString& filename, const QString& parent_dir)
{
	const QString abs_filename = Util::File::clean_filename(filename);
	if(abs_filename.isEmpty()) {
		return;
	}

	const QString abs_parent_dir = Util::File::clean_filename(parent_dir);
	if(abs_parent_dir.isEmpty()){
		return;
	}

	if(!abs_filename.startsWith(abs_parent_dir) || (abs_filename == abs_parent_dir)) {
		return;
	}

	m->files << abs_filename;
	if(Util::File::is_soundfile(abs_filename)) {
		add_soundfile(abs_filename);
	}

	const QStringList splitted_parent_dir = Util::File::split_directories(abs_parent_dir);
	const QStringList splitted_filename = Util::File::split_directories(abs_filename);

	QStringList remainder_parts;
	for(int i=splitted_parent_dir.size(); i<splitted_filename.size(); i++)
	{
		remainder_parts << splitted_filename[i];
	}

	if(remainder_parts.isEmpty())
	{
		return;
	}

	m->src_dst_map[filename] = remainder_parts.join("/");
}

QStringList ImportCache::files() const
{
	return m->files;
}

MetaDataList ImportCache::soundfiles() const
{
	return m->v_md;
}

QString ImportCache::target_filename(const QString& src_filename, const QString& target_directory) const
{
	if(m->library_path.isEmpty()){
		return QString();
	}

	QString original_path = Util::File::clean_filename(src_filename);

	QString path = QString("%1/%2/%3")
				.arg(m->library_path)
				.arg(target_directory)
				.arg(m->src_dst_map[original_path]);

	return Util::File::clean_filename(path);
}

MetaData ImportCache::metadata(const QString& filename) const
{
	return m->src_md_map[filename];
}

void ImportCache::change_metadata(const MetaDataList& v_md_old, const MetaDataList& v_md_new)
{
	Q_UNUSED(v_md_old)

	m->v_md = v_md_new;

	for(const MetaData& md : v_md_new)
	{
		m->src_md_map[md.filepath()] = md;
	}
}
