/* MetaDataInfo.h */

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

#ifndef METADATAINFO_H
#define METADATAINFO_H

#include "Utils/Settings/SayonaraClass.h"
#include "Components/Covers/CoverLocation.h"

#include <QMap>
#include <QObject>
#include "Utils/Pimpl.h"

class QStringList;
class MetaDataList;
class LibraryDatabase;

/**
 * @brief The InfoStrings enum
 * @ingroup MetaDataHelper
 */
enum class InfoStrings : uint8_t
{
	nTracks=0,		// set by MetaDataInfo
	nAlbums,		// set by ArtistInfo, AlbumInfo
	nArtists,		// set by ArtistInfo, AlbumInfo
	Filesize,		// set by MetaDataInfo
	PlayingTime,	// set by MetaDataInfo
	Year,			// set by MetaDataInfo
	Sampler,		// set by AlbumInfo
	Bitrate,		// set by MetaDataInfo
	Genre			// set by MetaDataInfo
};


/**
 * @brief The MetaDataInfo class
 * @ingroup MetaDataHelper
 */
class MetaDataInfo :
	public QObject,
	public SayonaraClass
{
	PIMPL(MetaDataInfo)

	protected:
		QString						_header;
		QString						_subheader;
		QMap<InfoStrings, QString>	_info;
		QMap<QString, QString>		_additional_info;


		QString calc_tracknum_str( uint16_t tracknum );
		QString calc_artist_str() const;
		QString calc_album_str();

		virtual void calc_cover_location();
		virtual void calc_subheader();
		virtual void calc_header();

		void insert_playing_time(uint64_t ms);
		void insert_genre(const QStringList& lst);
		void insert_filesize(uint64_t filesize);

		void insert_interval_info_field(InfoStrings key, int min, int max);
		void insert_numeric_info_field(InfoStrings key, int number);


	public:
		explicit MetaDataInfo(const MetaDataList& v_md);
		virtual ~MetaDataInfo();

		virtual QString header() const;
		virtual QString subheader() const;
		virtual QString infostring() const;
		virtual QString additional_infostring() const;

		virtual Cover::Location cover_location() const;

		const SP::Set<QString>& albums() const;
		const SP::Set<QString>& artists() const;
		const SP::Set<QString>& album_artists() const;

		const SP::Set<AlbumId>& album_ids() const;
		const SP::Set<ArtistId>& artist_ids() const;
		const SP::Set<ArtistId>& album_artist_ids() const;

		QString pathsstring() const;


	private:
		void calc_cover_location(const MetaDataList& lst);
		void calc_subheader(quint16 tracknum);
		void calc_header(const MetaDataList& lst);

		QString get_info_string(InfoStrings idx) const;
};

#endif // METADATAINFO_H
