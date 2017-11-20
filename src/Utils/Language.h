/* Language.h */

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

#ifndef LANGUAGE_H
#define LANGUAGE_H

#include <QObject>

class LanguageString : public QString
{
public:
	LanguageString(const QString& str);
	LanguageString toFirstUpper() const;
	LanguageString space() const;
	LanguageString question() const;
	LanguageString triplePt() const;
};

class Lang :
		public QObject
{
	Q_OBJECT

public:
	enum Term
	{
		About=0,
		Action,
		Actions,
		Activate,
		Active,
		AddTab,
		Album,
		AlbumArtists,
		Albums,
		All,
		Append,
		Application,
		Apply,
		Artist,
		Artists,
		Ascending,
		Bitrate,
		Bookmarks,
		Broadcast,
		By,
		Cancel,
		CannotFindLame,
		Continue,
		Covers,
		Clear,
		Close,
		CloseOthers,
		CloseTab,
		Date,
		Days,
		DaysShort,
		Default,
		Delete,
		Descending,
		Directory,
		Directories,
		Duration,
		DurationShort,
		DynamicPlayback,
		Edit,
		EmptyInput,
		EnterName,
		EnterUrl,
		Entries,
		Entry,
		Error,
		Fast,
		File,
		Filename,
		Files,
		Filesize,
		First,
		Font,
		Fonts,
		Fulltext,
		GaplessPlayback,
		Genre,
		Genres,
		Hours,
		HoursShort,
		ImportDir,
		ImportFiles,
		Inactive,
		Info,
		InvalidChars,
		Library,
		LibraryPath,
		Listen,
		Loading,
		LoadingArg,
		Logger,
		Lyrics,
		Menu,
		Minutes,
		MinutesShort,
		Missing,
		Months,
		MuteOn,
		MuteOff,
		Name,
		New,
		NextTrack,
		No,
		None,
		NumTracks,
		OK,
		On,
		Open,
		OpenDir,
		OpenFile,
		Or,
		Overwrite,
		Pause,
		Play,
		PlayPause,
		PlayingTime,
		Playlist,
		Playlists,
		PlayNext,
		Preferences,
		PreviousTrack,
		Radio,
		RadioStation,
		Rating,
		Really,
		Refresh,
		ReloadLibrary,
		Remove,
		Rename,
		Repeat1,
		RepeatAll,
		Reset,
		Retry,
		Sampler,
		Save,
		SaveAs,
		SaveToFile,
		SearchNoun,
		SearchVerb,
		SearchNext,
		Second,
		Seconds,
		SecondsShort,
		SeekForward,
		SeekBackward,
		Show,
		ShowAlbumArtists,
		Shuffle,
		Shutdown,
		SimilarArtists,
		SortBy,
		Stop,
		StreamUrl,
		Success,
		Th,
		Third,
		Title,
		Track,
		TrackOn,
		TrackNo,
		Tracks,
		Tree,
		Undo,
		UnknownPlaceholder,
		Various,
		VariousAlbums,
		VariousArtists,
		VariousTracks,
		Version,
		VolumeDown,
		VolumeUp,
		Warning,
		Weeks,
		Year,
		Years,
		Yes,
		Zoom,
		NUMBER_OF_LANGUAGE_KEYS
	};

public:
	Lang();
	~Lang();

	static LanguageString get(Lang::Term term, bool* ok=nullptr);
};

#endif // LANGUAGE_H
