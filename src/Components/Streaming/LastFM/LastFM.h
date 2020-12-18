/* LastFM.h */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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
 * LastFM.h
 *
 *  Created on: Apr 19, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef LASTFM_H_
#define LASTFM_H_

#include "Utils/Pimpl.h"

#include <QObject>

// singleton base LastFM API class
// signals and slots are handled by the adapter class
namespace LastFM
{
	class Base :
			public QObject
	{
		Q_OBJECT
		PIMPL(Base)

		signals:
			void sigLoggedIn(bool);

		public:
			Base();
			virtual ~Base();

			void login(const QString& username, const QString& password);
			bool isLoggedIn();

		private:
			bool initTrackChangeThread();
			void getSimilarArtists(const QString& artist);
			bool updateTrack(const MetaData& md);

		private slots:
			void activeChanged();
			void loginThreadFinished(bool success);
			//void similarArtistsFetched(const IdList& playlistTrack);
			void currentTrackChanged(const MetaData& md);

			void scrobble();
			void scrobbleResponseReceived(const QByteArray& data);
			void scrobbleErrorReceived(const QString& str);

			void trackChangedTimerTimedOut();
	};
}

#endif /* LASTFM_H_ */
