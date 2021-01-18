/* Playlist.h */

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

#ifndef PLAYLIST_H
#define PLAYLIST_H

#include "PlaylistDBInterface.h"
#include "PlaylistStopBehavior.h"
#include "Utils/Playlist/PlaylistFwd.h"
#include "Utils/Playlist/PlaylistMode.h"
#include "Utils/Pimpl.h"

#include <QObject>

class PlayManager;

namespace Playlist
{
	/**
	 * @brief The Playlist class
	 * @ingroup Playlists
	 */
	class Playlist :
		public QObject,
		public DBInterface,
		protected StopBehavior
	{
		Q_OBJECT
		PIMPL(Playlist)

			friend class Handler;

		signals:
			void sigItemsChanged(int index);
			void sigTrackChanged();
			void sigStopped();
			void sigFindTrack(TrackID trackId);
			void sigBusyChanged(bool b);
			void sigCurrentScannedFileChanged(const QString& currentFile);

		public:
			explicit Playlist(int idx, const QString& name, PlayManager* playManager);
			~Playlist() override;

			int createPlaylist(const MetaDataList& tracks);

			int currentTrackIndex() const;
			bool currentTrack(MetaData& metadata) const;

			int index() const;
			void setIndex(int idx);

			Mode mode() const;
			void setMode(const Mode& mode);

			MilliSeconds runningTime() const;
			int count() const override;

			void enableAll();

			void play();
			void stop();
			void fwd();
			void bwd();
			void next();
			bool wakeUp();

			void setBusy(bool b);
			bool isBusy() const;

			void reverse();

			const MetaData& track(int idx) const override;
			const MetaDataList& tracks() const override;

			void insertTracks(const MetaDataList& lst, int tgt);
			void appendTracks(const MetaDataList& lst);
			void removeTracks(const IndexSet& indexes);
			void replaceTrack(int idx, const MetaData& track);
			void clear();

			IndexSet moveTracks(const IndexSet& indexes, int tgt);
			IndexSet copyTracks(const IndexSet& indexes, int tgt);

			void findTrack(int idx);
			bool changeTrack(int index, MilliSeconds positionMs=0);
			bool wasChanged() const override;

			void reloadFromDatabase();

		public slots:
			void metadataDeleted();
			void metadataChanged();
			void currentMetadataChanged();
			void durationChanged();

		private slots:
			void settingPlaylistModeChanged();

		private:
			int calcShuffleTrack();
			void setCurrentTrack(int idx);
			void setChanged(bool b) override;
	};
}
#endif // PLAYLIST_H
