/* Sorting.h */

/* Copyright (C) 2011-2016  Lucio Carreras
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

#ifndef SORTING_H
#define SORTING_H

#include <QStringList>
namespace Library
{
    enum class SortOrder : quint8
    {
	    NoSorting=0,
	    ArtistNameAsc,
	    ArtistNameDesc,
	    ArtistTrackcountAsc,
	    ArtistTrackcountDesc,
	    AlbumNameAsc,
	    AlbumNameDesc,
	    AlbumYearAsc,
	    AlbumYearDesc,
	    AlbumTracksAsc,
	    AlbumTracksDesc,
	    AlbumDurationAsc,
	    AlbumDurationDesc,
	    AlbumRatingAsc,
	    AlbumRatingDesc,
	    TrackNumAsc,
	    TrackNumDesc,
	    TrackTitleAsc,
	    TrackTitleDesc,
	    TrackAlbumAsc,
	    TrackAlbumDesc,
	    TrackArtistAsc,
	    TrackArtistDesc,
	    TrackYearAsc,
	    TrackYearDesc,
	    TrackLenghtAsc,
	    TrackLengthDesc,
	    TrackBitrateAsc,
	    TrackBitrateDesc,
	    TrackSizeAsc,
	    TrackSizeDesc,
	    TrackDiscnumberAsc,
	    TrackDiscnumberDesc,
	    TrackRatingAsc,
	    TrackRatingDesc
    };


    class Sortings {

    public:
	Library::SortOrder so_albums;
	Library::SortOrder so_artists;
	Library::SortOrder so_tracks;

	Sortings(){
	    so_artists = Library::SortOrder::ArtistNameAsc;
	    so_albums = Library::SortOrder::AlbumNameAsc;
	    so_tracks = Library::SortOrder::TrackAlbumAsc;
	}

	Sortings(const Sortings& so){
	    so_albums = so.so_albums;
	    so_artists = so.so_artists;
	    so_tracks = so.so_tracks;
	}

	~Sortings(){}

	bool operator==(Library::Sortings so){
	    return  (so.so_albums == so_albums) &&
		    (so.so_artists == so_artists) &&
		    (so.so_tracks == so_tracks);
	}

	QString toString() const{
	    return QString::number((int) so_albums) + "," +
		    QString::number((int) so_artists) + "," +
		    QString::number((int) so_tracks);
	}

	static Library::Sortings fromString(const QString& str){

	    Library::Sortings so;
	    QStringList lst = str.split(",");
		    so.so_albums = (Library::SortOrder) lst[0].toInt();
		    so.so_artists = (Library::SortOrder) lst[1].toInt();
		    so.so_tracks = (Library::SortOrder) lst[2].toInt();
	    return so;
	}
    };
}

#endif // SORTING_H
