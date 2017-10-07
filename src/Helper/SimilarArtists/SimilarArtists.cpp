/* SimilarArtists.cpp */

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

#include "SimilarArtists.h"
#include "Helper/MetaData/Artist.h"
#include "Helper/Helper.h"
#include "Helper/FileHelper.h"
#include "Helper/Compressor/Compressor.h"

#include <QDir>

SimilarArtists::SimilarArtists() {}
SimilarArtists::~SimilarArtists() {}

static QString get_filename(const QString& artist)
{
    QString sayonara_path = Util::sayonara_path() + "/" + "similar_artists/";
    sayonara_path = Util::File::clean_filename(sayonara_path);
    QDir dir(sayonara_path);

    QStringList name_filters;
        name_filters << "*.comp";

	QDir::Filters filters = static_cast<QDir::Filters>(QDir::Files);

    QStringList files = dir.entryList(name_filters, filters);
    if(files.isEmpty()){
        return QString();
    }

	QString target_name = artist + ".comp";
    QString result_filename;
    for(const QString& str : files)
    {
        if(str.compare(target_name, Qt::CaseInsensitive) == 0)
        {
            result_filename = dir.filePath(target_name);
            break;
        }
    }

    return result_filename;
}

QMap<QString, double>
SimilarArtists::get_similar_artists(const QString& artist) const
{
    QMap<QString, double> sim_artist_map;
	QString filename = get_filename(artist);
    if(filename.isEmpty()){
        return sim_artist_map;
    }

    QByteArray content, decomp;
    bool success = Util::File::read_file_into_byte_arr(filename, content);
    if(!success){
        return sim_artist_map;
    }

    decomp = Compressor::decompress(content);
	if(decomp.isEmpty()){
		return sim_artist_map;
	}

	QStringList sim_artists  = QString::fromLocal8Bit(decomp).split("\n");
    for(const QString& sim_artist : sim_artists){
        QStringList lst = sim_artist.split('\t');
        if(lst.size() < 3){
			continue;
		}

		QString match = lst[0];
		QString artist_name = lst[2];

        sim_artist_map[artist_name] = match.toDouble();
    }

    return sim_artist_map;
}

QStringList SimilarArtists::get_similar_artist_names(const QString &artist) const
{
    return QStringList( get_similar_artists(artist).keys() );
}
