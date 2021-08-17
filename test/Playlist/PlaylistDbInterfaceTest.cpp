/*
 * Copyright (C) 2011-2021 Michael Lugmair
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

#include "SayonaraTest.h"
#include "PlayManagerMock.h"
#include "Database/TestTracks.h"

#include "Components/Playlist/Playlist.h"
#include "Components/Playlist/PlaylistDBWrapper.h"
#include "Components/Playlist/PlaylistChangeNotifier.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/MetaData/MetaDataList.h"

#include "Utils/Algorithm.h"

#include <QSignalSpy>

// access working directory with Test::Base::tempPath("somefile.txt");

using namespace ::Playlist;

namespace
{
	::Playlist::Playlist createPlaylist(int index, const QString& name)
	{
		auto* playManager = new PlayManagerMock();
		return ::Playlist::Playlist(index, name, playManager);
	}

	template<typename A, typename B>
	bool isEqual(const A& a, const B& b)
	{
		const auto tracks1 = a.tracks();
		const auto tracks2 = b.tracks();

		if(tracks1.count() != tracks2.count())
		{
			return false;
		}

		const auto tracksEqual =
			std::equal(tracks1.begin(), tracks1.end(), tracks2.begin(), [](const auto& track1, const auto& track2){
			return (track1.isEqual(track2));
		});

		return (a.id() == b.id()) &&
		       (a.name() == b.name()) &&
		       (a.isTemporary() == b.isTemporary()) &&
		       tracksEqual;
	}
}

namespace P = ::Playlist;
class PlaylistDbInterfaceTest :
	public Test::Base
{
	Q_OBJECT

	public:
		PlaylistDbInterfaceTest() :
			Test::Base("PlaylistDbInterfaceTest")
		{
			SetSetting(Set::PL_LoadSavedPlaylists, true);
			SetSetting(Set::PL_LoadTemporaryPlaylists, true);
		}

	private slots:
		void testInsertAndRename();
		void testInsertAndSaveAs();
		void testInsertAndSaveAsInvalidName();
		void testDeletion();
		void testPlaylistChangeNotifier();
		void testWithTracks();
};

void PlaylistDbInterfaceTest::testInsertAndRename()
{
	const auto originalName = QStringLiteral("one");
	auto playlist = createPlaylist(0, originalName);

	QVERIFY(playlist.id() < 0);
	QVERIFY(playlist.name() == originalName);
	QVERIFY(playlist.isTemporary());
	QVERIFY(playlist.isSaveAsPossible());

	{ // save
		const auto answer = playlist.save();
		QVERIFY(answer == Util::SaveAsAnswer::Success);
		QVERIFY(playlist.id() >= 0);
		QVERIFY(playlist.isTemporary());
		QVERIFY(playlist.isSaveAsPossible());

		const auto dbPlaylist = DBWrapper::getPlaylistById(playlist.id(), false);
		QVERIFY(isEqual(playlist, dbPlaylist));
	}

	{ // rename
		const auto newName = QStringLiteral("one renamed");
		const auto answer = playlist.rename(newName);
		QVERIFY(answer == Util::SaveAsAnswer::Success);
		QVERIFY(playlist.name() == newName);
		QVERIFY(playlist.isTemporary());
		QVERIFY(playlist.isSaveAsPossible());

		const auto dbPlaylist = DBWrapper::getPlaylistById(playlist.id(), false);
		QVERIFY(isEqual(playlist, dbPlaylist));
	}

	{ // rename other playlist to same name
		const auto otherPlaylistName = QStringLiteral("one bla 2");
		auto otherPlaylist = createPlaylist(0, otherPlaylistName);
		QVERIFY(otherPlaylist.save() == Util::SaveAsAnswer::Success);

		const auto answer = otherPlaylist.rename(playlist.name());
		QVERIFY(answer == Util::SaveAsAnswer::NameAlreadyThere);
		QVERIFY(otherPlaylist.name() == otherPlaylistName);

		const auto dbPlaylist = DBWrapper::getPlaylistById(otherPlaylist.id(), false);
		QVERIFY(isEqual(otherPlaylist, dbPlaylist));
	}

	{ // rename playlist which isn't saved yet
		const auto otherPlaylistName = QStringLiteral("one bla 3");
		auto otherPlaylist = createPlaylist(0, otherPlaylistName);
		const auto answer = otherPlaylist.rename("some name");
		QVERIFY(answer == Util::SaveAsAnswer::OtherError);
		QVERIFY(otherPlaylist.name() == otherPlaylistName);
	}
}

void PlaylistDbInterfaceTest::testInsertAndSaveAs()
{
	const auto name = QStringLiteral("two");
	const auto newName = QStringLiteral("two renamed");
	auto playlist = createPlaylist(2, name);

	{ // try to save playlist
		const auto answer = playlist.saveAs(newName);
		QVERIFY(answer == Util::SaveAsAnswer::Success);
		QVERIFY(playlist.id() >= 0);
		QVERIFY(playlist.name() == newName);
		QVERIFY(!playlist.isTemporary());
		QVERIFY(!playlist.isSaveAsPossible());

		const auto dbPlaylist = DBWrapper::getPlaylistById(playlist.id(), false);
		QVERIFY(isEqual(playlist, dbPlaylist));
	}

	{ // try to call saveAs() again
		const auto oldName = playlist.name();
		const auto answer = playlist.saveAs(QStringLiteral("two renamed again"));
		QVERIFY(answer == Util::SaveAsAnswer::OtherError);
		QVERIFY(playlist.name() == oldName);

		const auto dbPlaylist = DBWrapper::getPlaylistById(playlist.id(), false);
		QVERIFY(isEqual(playlist, dbPlaylist));
	}
}

void PlaylistDbInterfaceTest::testInsertAndSaveAsInvalidName()
{
	auto playlist = createPlaylist(3, "three");
	auto otherPlaylist = createPlaylist(4, "four");
	{ // save playlist
		const auto answer = playlist.save();
		QVERIFY(answer == Util::SaveAsAnswer::Success);
	}

	{ // save other playlist
		const auto answer = otherPlaylist.save();
		QVERIFY(answer == Util::SaveAsAnswer::Success);
	}

	{ // try to save playlist as other playlist's name
		const auto oldName = playlist.name();
		const auto newName = otherPlaylist.name();
		const auto answer = playlist.saveAs(newName);
		QVERIFY(answer == Util::SaveAsAnswer::NameAlreadyThere);
		QVERIFY(playlist.name() == oldName);

		const auto dbPlaylist = DBWrapper::getPlaylistById(playlist.id(), false);
		QVERIFY(isEqual(playlist, dbPlaylist));
	}

	{ // try to save as empty name
		const auto answer = playlist.saveAs(QString());
		QVERIFY(answer == Util::SaveAsAnswer::InvalidName);

		const auto dbPlaylist = DBWrapper::getPlaylistById(playlist.id(), false);
		QVERIFY(isEqual(playlist, dbPlaylist));
	}
}

void PlaylistDbInterfaceTest::testDeletion()
{
	auto playlist = createPlaylist(5, "five");
	{ // save playlist
		const auto answer = playlist.save();
		QVERIFY(answer == Util::SaveAsAnswer::Success);
		QVERIFY(playlist.id() >= 0);
	}

	{
		const auto success = playlist.deletePlaylist();
		QVERIFY(success);
		QVERIFY(playlist.id() < 0);
	}
}

void PlaylistDbInterfaceTest::testPlaylistChangeNotifier()
{
	auto playlist = createPlaylist(6, "six");
	{ // save temporary playlist
		auto* changeNotifier = PlaylistChangeNotifier::instance();
		auto spy = QSignalSpy(changeNotifier, &PlaylistChangeNotifier::sigPlaylistAdded);

		const auto answer = playlist.save();
		QVERIFY(answer == Util::SaveAsAnswer::Success);
		QVERIFY(spy.count() == 0);
	}

	{ // rename temporary playlist
		const auto oldName = playlist.name();
		auto* changeNotifier = PlaylistChangeNotifier::instance();
		auto spy = QSignalSpy(changeNotifier, &PlaylistChangeNotifier::sigPlaylistRenamed);

		const auto answer = playlist.rename("six new");
		QVERIFY(answer == Util::SaveAsAnswer::Success);
		QVERIFY(spy.count() == 1);
		QVERIFY(spy.value(0)[0].toInt() == playlist.id());
		QVERIFY(spy.value(0)[1].toString() == oldName);
		QVERIFY(spy.value(0)[2].toString() == playlist.name());
	}

	{ // delete temporary playlist
		const auto id = playlist.id();
		auto* changeNotifier = PlaylistChangeNotifier::instance();
		auto spy = QSignalSpy(changeNotifier, &PlaylistChangeNotifier::sigPlaylistDeleted);

		const auto success = playlist.deletePlaylist();
		QVERIFY(success);
		QVERIFY(spy.count() == 1);
		QVERIFY(spy.value(0).first().toInt() == id);
	}

	{ // save permanent playlist
		auto* changeNotifier = PlaylistChangeNotifier::instance();
		auto spy = QSignalSpy(changeNotifier, &PlaylistChangeNotifier::sigPlaylistAdded);

		const auto answer = playlist.saveAs(playlist.name());
		QVERIFY(answer == Util::SaveAsAnswer::Success);
		QVERIFY(spy.count() == 1);
		QVERIFY(spy.value(0)[0] == playlist.id());
		QVERIFY(spy.value(0)[1] == playlist.name());
	}

	{ // rename permanent playlist
		const auto oldName = playlist.name();
		auto* changeNotifier = PlaylistChangeNotifier::instance();
		auto spy = QSignalSpy(changeNotifier, &PlaylistChangeNotifier::sigPlaylistRenamed);

		const auto answer = playlist.rename("six new");
		QVERIFY(answer == Util::SaveAsAnswer::Success);
		QVERIFY(spy.count() == 1);
		QVERIFY(spy.value(0)[0].toInt() == playlist.id());
		QVERIFY(spy.value(0)[1].toString() == oldName);
		QVERIFY(spy.value(0)[2].toString() == playlist.name());
	}

	{ // delete permanent playlist
		const auto id = playlist.id();
		auto* changeNotifier = PlaylistChangeNotifier::instance();
		auto spy = QSignalSpy(changeNotifier, &PlaylistChangeNotifier::sigPlaylistDeleted);

		const auto success = playlist.deletePlaylist();
		QVERIFY(success);
		QVERIFY(spy.count() == 1);
		QVERIFY(spy.value(0).first().toInt() == id);
	}
}

void PlaylistDbInterfaceTest::testWithTracks()
{
	const auto tracks = Test::createTracks();
	auto playlist = createPlaylist(7, "seven");
	playlist.createPlaylist(tracks);

	{
		QVERIFY(playlist.wasChanged());

		playlist.save();

		const auto dbPlaylist = DBWrapper::getPlaylistById(playlist.id(), true);
		QVERIFY(isEqual(playlist, dbPlaylist));
		QVERIFY(!playlist.wasChanged());
	}

	{
		const auto track = Test::createTrack(0, "title", "artist", "album");
		playlist.appendTracks(MetaDataList{track});

		const auto dbPlaylist = DBWrapper::getPlaylistById(playlist.id(), true);
		QVERIFY(!isEqual(playlist, dbPlaylist));
		QVERIFY(playlist.wasChanged());
	}

	{
		playlist.save();

		const auto dbPlaylist = DBWrapper::getPlaylistById(playlist.id(), true);
		QVERIFY(!isEqual(playlist, dbPlaylist));
		QVERIFY(!playlist.wasChanged());
	}
}

QTEST_GUILESS_MAIN(PlaylistDbInterfaceTest)

#include "PlaylistDbInterfaceTest.moc"