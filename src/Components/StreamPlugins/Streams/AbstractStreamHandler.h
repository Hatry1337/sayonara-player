/* AbstractStreamHandler.h */

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


#ifndef AbstractStreamHandler_H
#define AbstractStreamHandler_H

#include <QMap>
#include <QString>
#include "Helper/MetaData/MetaDataList.h"


typedef QMap<QString, QString> StreamMap;


class PlaylistHandler;
class AsyncWebAccess;
class DatabaseConnector;

/**
 * @brief Used to interprete website data as streams. Some methods have to be overridden,
 * to map their functions to their specific database functions.
 * The track list is held in a map, which is accessible through its station name. It can be
 * accessed via the get_tracks() method.
 * @ingroup Streams
 */
class AbstractStreamHandler : public QObject
{
	Q_OBJECT
public:
	explicit AbstractStreamHandler(QObject *parent=nullptr);

signals:
	void sig_error();
	void sig_data_available();

public:
	/**
	 * @brief Retrieves data from the station and tries to interprete it via the parse_content() method.
	 * @param url url to retrieve the data from
	 * @param station_name the station name
	 * @return true, if no other station is parsed atm, false else
	 */
	bool parse_station(const QString& url, const QString& station_name);

	/**
	 * @brief clears all station contents
	 */
	void clear();

	/**
	 * @brief get_tracks
	 * @param station_name
	 * @return
	 */
	MetaDataList get_tracks(const QString& station_name);

	/**
	 * @brief Saves the station. Calls the add_stream() method.
	 * @param station_name The station name.
	 * @param url the station url.
	 */
	void save(const QString& station_name, const QString& url);

	/**
	 * @brief This method should return all stations in database
	 * @param streams target StreamMap
	 * @return true if successful, false else
	 */
	virtual bool get_all_streams(StreamMap& streams)=0;

	/**
	 * @brief This method should add a new station to database. If the station
	 * already exists, there should be a corresponding error handling.
	 * @param station_name station name
	 * @param url url
	 * @return true if successful, false else
	 */
	virtual bool add_stream(const QString& station_name, const QString& url)=0;

	/**
	 * @brief Delete a station from the database.
	 * @param station_name the station to be deleted
	 * @return true if successful, false else
	 */
	virtual bool delete_stream(const QString& station_name)=0;

	/**
	 * @brief Update the url of a station
	 * @param station_name the station to be updated
	 * @param url the new url
	 * @return true if successful, false else
	 */
	virtual bool update_url(const QString& station_name, const QString& url)=0;

	/**
	 * @brief Rename the station
	 * @param station_name new name of the station
	 * @param url old URL of the station
	 * @return true if successful, false else
	 */
	virtual bool rename_stream(const QString& station_name, const QString& url)=0;


protected:
	DatabaseConnector*				_db=nullptr;
	PlaylistHandler*				_playlist=nullptr;


	QMap<QString, MetaDataList>		_station_contents;
	QString							_url;
	QString							_station_name;
	bool							_blocked;

	QStringList						_stream_buffer;


private:

	/**
	 * @brief Writes a temporary playlist file into the file system which is parsed later
	 * @param data Raw data extracted from the website
	 * @return filename where the playlist has been written at
	 */
	QString write_playlist_file(const QByteArray& data);

	/**
	 * @brief Search for a playlist file in website data
	 * @param data website data
	 * @return list of playlist files found in website data
	 */
	QStringList search_for_playlist_files(const QByteArray& data);

	/**
	 * @brief Sset up missing fields in metadata: album, artist, title and filepath\n
	 * @param md reference to a MetaData structure
	 * @param stream_url url used to fill album/artist/filepath
	 */
	void finalize_metadata(MetaData& md, const QString& stream_url);

	/**
	 * @brief Parse content out of website data.
	 * First, check if the data is podcast data.\n
	 * Second, check if the data is a playlist file\n
	 * Else, search for playlist files within the content.
	 *
	 * @param data Raw website data
	 * @return list of tracks found in the website data
	 */
	MetaDataList parse_content(const QByteArray& data);


private slots:
	void awa_finished(bool success);

};

#endif // AbstractStreamHandler_H
