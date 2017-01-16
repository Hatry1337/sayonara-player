/* AbstractPlaylistParser.cpp */

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

#include "AbstractPlaylistParser.h"
#include "PlaylistParser.h"
#include "Helper/FileHelper.h"
#include "Helper/WebAccess/AsyncWebAccess.h"
#include "Helper/MetaData/MetaData.h"
#include "Helper/MetaData/MetaDataList.h"

#include <QDir>

struct AbstractPlaylistParser::Private
{
	MetaDataList		v_md;
	QString				file_content;
	QString				directory;
	bool				parsed;

	Private()
	{
		parsed = false;
	}
};


AbstractPlaylistParser::AbstractPlaylistParser(const QString& filename)
{
	_m = Pimpl::make<AbstractPlaylistParser::Private>();

	QString pure_file;

	Helper::File::split_filename(filename, _m->directory, pure_file);
	Helper::File::read_file_into_str(filename, _m->file_content);

}

AbstractPlaylistParser::~AbstractPlaylistParser() {}

MetaDataList AbstractPlaylistParser::tracks(bool force_parse)
{
	if(force_parse){
		_m->parsed = false;
	}

	if(!_m->parsed){
		_m->v_md.clear();
		parse();
	}

	_m->parsed = true;


	return _m->v_md;
}

void AbstractPlaylistParser::add_track(const MetaData& md)
{
	_m->v_md << md;
}

void AbstractPlaylistParser::add_tracks(const MetaDataList& v_md)
{
	_m->v_md << v_md;
}

const QString& AbstractPlaylistParser::content() const
{
	return _m->file_content;
}


QString AbstractPlaylistParser::get_absolute_filename(const QString& filename) const
{
	QString ret;

	if(filename.isEmpty()){
		return "";
	}

	if(Helper::File::is_www(filename)){
		return filename;
	}

	if(!Helper::File::is_absolute(filename)){
		ret = _m->directory + "/" + filename;
	}
	else{
		ret = filename;
	}

	if(!QFile::exists(ret)){
		ret = "";
	}

	return Helper::File::clean_filename(ret);
}
