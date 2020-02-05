/* MetaData.h */

/* Copyright (C) 2011-2020 Lucio Carreras
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

/*
 * MetaData.h
 *
 *  Created on: Mar 10, 2011
 *      Author: Lucio Carreras
 */

#ifndef METADATA_H_
#define METADATA_H_

#include "Utils/MetaData/LibraryItem.h"
#include "Utils/MetaData/RadioMode.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Library/Sortorder.h"
#include "Utils/Pimpl.h"

#include <QMetaType>
#include <QString>

class QDateTime;
/**
 * @brief The MetaData class
 * @ingroup MetaDataHelper
 */
class MetaData :
	public LibraryItem
{
	PIMPL(MetaData)

public:
	MetaData();
	explicit MetaData (const QString& path);
	MetaData(const MetaData& other);
	MetaData(MetaData&& other) noexcept;
	MetaData& operator=(const MetaData& md);
	MetaData& operator=(MetaData&& md) noexcept;

	~MetaData();

	QString title() const;
	void set_title(const QString& title);

	QString artist() const;
	void set_artist(const QString& artist);
	ArtistId artist_id() const;
	void set_artist_id(ArtistId id);

	QString album() const;
	void set_album(const QString& album);
	AlbumId album_id() const;
	void set_album_id(AlbumId id);

	const QString& comment() const;
	void set_comment(const QString& comment);

	QString filepath() const;
	QString set_filepath(QString filepath, RadioMode mode=RadioMode::Undefined);

	ArtistId album_artist_id() const;
	QString album_artist() const;
	bool has_album_artist() const;

	void set_album_artist(const QString& album_artist, ArtistId id=-1);
	void set_album_artist_id(ArtistId id);

	void set_radio_station(const QString& name);
	QString radio_station() const;

	RadioMode radio_mode() const;
	void change_radio_mode(RadioMode mode);

	bool is_valid() const;

	bool operator==(const MetaData& md) const;
	bool operator!=(const MetaData& md) const;
	bool is_equal(const MetaData& md) const;
	bool is_equal_deep(const MetaData& md) const;

	const Util::Set<GenreID>& genre_ids() const;
	Util::Set<Genre> genres() const;
	bool has_genre(const Genre& genre) const;
	bool remove_genre(const Genre& genre);
	bool add_genre(const Genre& genre);
	void set_genres(const Util::Set<Genre>& genres);
	void set_genres(const QStringList& genres);

	void set_createdate(uint64_t t);
	uint64_t createdate() const;
	QDateTime createdate_datetime() const;

	void set_modifydate(uint64_t t);
	uint64_t modifydate() const;
	QDateTime modifydate_datetime() const;

	QString genres_to_string() const;
	QStringList genres_to_list() const;

	QString to_string() const;

	static QVariant toVariant(const MetaData& md);
	static bool fromVariant(const QVariant& v, MetaData& md);

	Disc discnumber() const;
	void set_discnumber(const Disc& value);

	Disc disc_count() const;
	void set_disc_count(const Disc& value);

	Bitrate bitrate() const;
	void set_bitrate(const Bitrate& value);

	TrackNum track_number() const;
	void set_track_number(const uint16_t& value);

	Year year() const;
	void set_year(const uint16_t& value);

	Filesize filesize() const;
	void set_filesize(const Filesize& value);

	Rating rating() const;
	void set_rating(const Rating& value);

	MilliSeconds duration_ms() const;
	void set_duration_ms(const MilliSeconds& value);

	bool is_extern() const;
	void set_extern(bool value);

	bool is_disabled() const;
	void set_disabled(bool value);

	LibraryId library_id() const;
	void set_library_id(const LibraryId& value);

	TrackID id() const;
	void set_id(const TrackID& value);

private:
	QHash<GenreID, Genre>& genre_pool() const;
};

#ifndef MetaDataDeclared
Q_DECLARE_METATYPE(MetaData)
	#define MetaDataDeclared
#endif

#endif /* METADATA_H_ */
