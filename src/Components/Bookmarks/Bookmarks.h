/* Bookmarks.h */

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

#ifndef BOOKMARKS_H
#define BOOKMARKS_H

#include "BookmarkStorage.h"

#include <QList>
#include <QObject>

class Bookmark;
class MetaData;
class PlayManager;

/**
 * @brief The Bookmarks logic class
 * @ingroup Bookmarks
 */
class Bookmarks :
	public QObject
{
	Q_OBJECT
	PIMPL(Bookmarks)

	signals:
		/**
		 * @brief emitted when bookmarks have changed
		 * @param bookmarks new bookmarks
		 */
		void sigBookmarksChanged();

		/**
		 * @brief previous bookmark has changed
		 * @param bm new bookmark. Check for Bookmark::isValid()
		 */
		void sigPreviousChanged(const Bookmark& bm);

		/**
		 * @brief next bookmark has changed
		 * @param bm new bookmark. Check for Bookmark::isValid()
		 */
		void sigNextChanged(const Bookmark& bm);

	public:
		explicit Bookmarks(PlayManager* playManager, QObject* parent=nullptr);
		~Bookmarks() override;

		/**
		 * @brief Jump to specific bookmark
		 * @param idx bookmark index
		 * @return true if index was valid
		 */
		bool jumpTo(int idx);

		/**
		 * @brief Jump to next bookmark
		 * @return true if successful, false else
		 */
		bool jumpNext();

		/**
		 * @brief Jump to previous bookmark
		 * @return true if successful, false else
		 */
		bool jumpPrevious();

		/**
		 * @brief tries to set the loop between the current two indices
		 * @param b switch loop on or off
		 * @return false if the two current indices are invalid or if b == false. True else
		 */
		bool setLoop(bool b);

		int count() const;

		BookmarkStorage::CreationStatus create();

		bool remove(int index);
		const QList<Bookmark>& bookmarks() const;

		const MetaData& currentTrack() const;

	private slots:
		/**
		 * @brief track position has changed
		 * @param positionMs new position in ms
		 */
		void positionChangedMs(MilliSeconds positionMs);

		/**
		 * @brief current track has changed
		 * @param track new MetaData object
		 */
		void currentTrackChanged(const MetaData& track);

		/**
		 * @brief current playstate has changed
		 * @param state new playstate
		 */
		void playstateChanged(PlayState state);
};

#endif // BOOKMARKS_H
