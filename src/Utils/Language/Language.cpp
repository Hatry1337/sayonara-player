/* Language.cpp */

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

#include "Language.h"
#include "Algorithm.h"
#include "StandardPaths.h"
#include "Utils.h"

#include <QDir>
#include <QRegExp>
#include <QStringList>
#include <QLocale>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunknown-pragmas"

LanguageString::LanguageString(const QString& other) :
	QString(other) {}

LanguageString& LanguageString::operator=(const QString& other)
{
	QString::operator=(other);
	return *this;
}

LanguageString& LanguageString::toFirstUpper()
{
	this->replace(0, 1, this->at(0).toUpper());
	return *this;
}

LanguageString& LanguageString::space()
{
	this->append(' ');
	return *this;
}

LanguageString& LanguageString::question()
{
	this->append('?');
	return *this;
}

LanguageString& LanguageString::triplePt()
{
	this->append("...");
	return *this;
}

Lang::Lang() = default;

Lang::~Lang() = default;

#pragma clang diagnostic push
#pragma ide diagnostic ignored "readability-static-accessed-through-instance"

LanguageString Lang::get(Lang::Term term, bool* ok)
{
	if(ok)
	{
		*ok = true;
	}

	Lang l;
	switch(term)
	{
		case About:
			return l.tr("About");
		case Action:
			return l.tr("Action");
		case Actions:
			return l.tr("Actions");
		case Activate:
			return l.tr("Activate");
		case Active:
			return l.tr("Active");
		case Add:
			return l.tr("Add");
		case AddArtist:
			return l.tr("Add artist");
		case AddTab:
			return l.tr("Add tab");
		case Album:
			return l.tr("Album");
		case AlbumArtist:
			return l.tr("Album artist");
		case AlbumArtists:
			return l.tr("Album artists");
		case Albums:
			return l.tr("Albums");
		case All:
			return l.tr("All");
		case Append:
			return l.tr("Append");
		case Application:
			return l.tr("Application");
		case Apply:
			return l.tr("Apply");
		case Artist:
			return l.tr("Artist");
		case Artists:
			return l.tr("Artists");
		case Ascending:
			return l.tr("Ascending");
		case Automatic:
			return l.tr("Automatic");
		case Bitrate:
			return l.tr("Bitrate");
		case Bookmarks:
			return l.tr("Bookmarks");
		case Broadcast:
			return l.tr("Broadcast");
		case By:
			// "Beat it" by "Michael Jackson"
			return l.tr("by");
		case Cancel:
			return l.tr("Cancel");
		case CannotFindLame:
			return l.tr("Cannot find Lame MP3 encoder");
		case CaseInsensitive:
			return l.tr("Case insensitive");
		case Clear:
			return l.tr("Clear");
		case ClearSelection:
			return l.tr("Clear selection");
		case Close:
			return l.tr("Close");
		case CloseOthers:
			return l.tr("Close others");
		case CloseTab:
			return l.tr("Close tab");
		case Comment:
			return l.tr("Comment");
		case Continue:
			return l.tr("Continue");
		case Covers:
			return l.tr("Covers");
		case Created:
			return l.tr("Created");
		case CreateDirectory:
			return l.tr("Create new directory");
		case CreateNewLibrary:
			return l.tr("Create a new library");
		case DarkMode:
			return l.tr("Dark Mode");
		case Date:
			return l.tr("Date");
		case Days:
			return l.tr("Days");
		case DaysShort:
			// short form of day
			return l.tr("d");
		case Default:
			return l.tr("Default");
		case Delete:
			return l.tr("Delete");
		case Descending:
			return l.tr("Descending");
		case Directory:
			return l.tr("Directory");
		case Directories:
			return l.tr("Directories");
		case Disc:
			return l.tr("Disc");
		case Duration:
			return l.tr("Duration");
		case DurationShort:
			// short form of duration
			return l.tr("Dur.");
		case DynamicPlayback:
			return l.tr("Dynamic playback");
		case Edit:
			return l.tr("Edit");
		case EmptyInput:
			return l.tr("Empty input");
		case EnterName:
			return l.tr("Enter name");
		case EnterNewName:
			return l.tr("Please enter new name");
		case EnterUrl:
			return l.tr("Enter URL");
		case Entry:
			return l.tr("Entry");
		case Entries:
			return l.tr("Entries");
		case Error:
			return l.tr("Error");
		case Fast:
			return l.tr("Fast");
		case File:
			return l.tr("File");
		case Filename:
			return l.tr("Filename");
		case Files:
			return l.tr("Files");
		case Filesize:
			return l.tr("Filesize");
		case Filetype:
			return l.tr("File type");
		case Filter:
			return l.tr("Filter");
		case First:
			return l.tr("1st");
		case Font:
			return l.tr("Font");
		case Fonts:
			return l.tr("Fonts");
		case Fulltext:
			return l.tr("Fulltext");
		case GaplessPlayback:
			return l.tr("Gapless playback");
		case GB:
			return l.tr("GB");
		case Genre:
			return l.tr("Genre");
		case Genres:
			return l.tr("Genres");
		case Hide:
			return l.tr("Hide");
		case Hours:
			return l.tr("Hours");
		case HoursShort:
			// short form of hours
			return l.tr("h");
		case IgnoreAccents:
			return l.tr("Ignore accents");
		case IgnoreSpecialChars:
			return l.tr("Ignore special characters");
		case ImportDir:
			return l.tr("Import directory");
		case ImportFiles:
			return l.tr("Import files");
		case Inactive:
			return l.tr("Inactive");
		case Info:
			return l.tr("Info");
		case Loading:
			return l.tr("Loading");
		case LoadingArg:
			return l.tr("Loading %1");
		case InvalidChars:
			return l.tr("Invalid characters");
		case KB:
			return l.tr("KB");
		case Key_Find:
			return l.tr("Ctrl+f");
		case Key_Delete:
			return l.tr("Delete");
		case Key_Escape:
			return l.tr("Esc");
		case Key_Control:
			return l.tr("Ctrl");
		case Key_Alt:
			return l.tr("Alt");
		case Key_Shift:
			return l.tr("Shift");
		case Key_Backspace:
			return l.tr("Backspace");
		case Key_Tab:
			return l.tr("Tab");
		case Library:
			return l.tr("Library");
		case LibraryPath:
			return l.tr("Library path");
		case LibraryView:
			return l.tr("Library view type");
		case Listen:
			return l.tr("Listen");
		case LiveSearch:
			return l.tr("Live Search");
		case Logger:
			return l.tr("Logger");
		case LogLevel:
			return l.tr("Log level");
		case Lyrics:
			return l.tr("Lyrics");
		case MB:
			return l.tr("MB");
		case Menu:
			return l.tr("Menu");
		case Minimize:
			return l.tr("Minimize");
		case Minutes:
			return l.tr("Minutes");
		case MinutesShort:
			// short form of minutes
			return l.tr("m");
		case Missing:
			return l.tr("Missing");
		case Modified:
			return l.tr("Modified");
		case Months:
			return l.tr("Months");
		case MuteOn:
			return l.tr("Mute");
		case MuteOff:
			return l.tr("Mute off");
		case Name:
			return l.tr("Name");
		case New:
			return l.tr("New");
		case NextPage:
			return l.tr("Next page");
		case NextTrack:
			return l.tr("Next track");
		case No:
			return l.tr("No");
		case NoAlbums:
			return l.tr("No albums");
		case NumTracks:
			return l.tr("Tracks");
		case MoveDown:
			return l.tr("Move down");
		case MoveUp:
			return l.tr("Move up");
		case OK:
			return l.tr("OK");
		case On:
			// 5th track on "Thriller"
			return l.tr("on");
		case Open:
			return l.tr("Open");
		case OpenDir:
			return l.tr("Open directory");
		case OpenFile:
			return l.tr("Open file");
		case Or:
			return l.tr("or");
		case Overwrite:
			return l.tr("Overwrite");
		case Pause:
			return l.tr("Pause");
		case Play:
			return l.tr("Play");
		case PlayingTime:
			return l.tr("Playing time");
		case PlayInNewTab:
			return l.tr("Play in new tab");
		case Playlist:
			return l.tr("Playlist");
		case Playlists:
			return l.tr("Playlists");
		case PlayNext:
			return l.tr("Play next");
		case PlayPause:
			return l.tr("Play/Pause");
		case Plugin:
			return l.tr("Plugin");
		case Podcasts:
			return l.tr("Podcasts");
		case Preferences:
			return l.tr("Preferences");
		case PreviousPage:
			return l.tr("Previous page");
		case PreviousTrack:
			return l.tr("Previous track");
		case PurchaseUrl:
			return l.tr("Purchase Url");
		case Quit:
			return l.tr("Quit");
		case Radio:
			return l.tr("Radio");
		case RadioStation:
			return l.tr("Radio Station");
		case Rating:
			return l.tr("Rating");
		case Really:
			return l.tr("Really");
		case Refresh:
			return l.tr("Refresh");
		case ReloadLibrary:
			return l.tr("Reload Library");
		case Remove:
			return l.tr("Remove");
		case Rename:
			return l.tr("Rename");
		case Repeat1:
			return l.tr("Repeat 1");
		case RepeatAll:
			return l.tr("Repeat all");
		case Replace:
			return l.tr("Replace");
		case Reset:
			return l.tr("Reset");
		case Retry:
			return l.tr("Retry");
		case ReverseOrder:
			return l.tr("Reverse order");
		case Sampler:
			return l.tr("Sampler");
		case Shuffle:
			return l.tr("Shuffle");
		case ShufflePlaylist:
			return l.tr("Shuffle playlist");
		case Shutdown:
			return l.tr("Shutdown");
		case Save:
			return l.tr("Save");
		case SaveAs:
			return l.tr("Save as");
		case SaveToFile:
			return l.tr("Save to file");
		case ScanForFiles:
			return l.tr("Scan for audio files");
		case SearchNoun: // NOLINT(bugprone-branch-clone)
			return l.tr("Search");
		case SearchVerb:
			return l.tr("Search");
		case SearchNext:
			return l.tr("Search next");
		case SearchPrev:
			return l.tr("Search previous");
		case Second:
			return l.tr("2nd");
		case Seconds:
			return l.tr("Seconds");
		case SecondsShort:
			// short form of seconds
			return l.tr("s");
		case SeekBackward:
			return l.tr("Seek backward");
		case SeekForward:
			return l.tr("Seek forward");
		case Show:
			return l.tr("Show");
		case ShowAlbumArtists:
			return l.tr("Show Album Artists");
		case ShowCovers:
			return l.tr("Show Covers");
		case ShowLibrary:
			return l.tr("Show Library");
		case SimilarArtists:
			return l.tr("Similar artists");
		case SmartPlaylists:
			return l.tr("Smart Playlists");
		case SortBy:
			return l.tr("Sort by");            // for example "sort by year"
		case Stop:
			return l.tr("Stop");
		case Streams:
			return l.tr("Streams");
		case StreamUrl:
			return l.tr("Stream URL");
		case Success:
			return l.tr("Success");
		case Th:
			return l.tr("th");
		case Third:
			return l.tr("3rd");
		case Title:
			return l.tr("Title");
		case Track:
			return l.tr("Track");
		case TrackNo:
			return l.tr("Track number");
		case TrackOn:
			return l.tr("track on");
		case Tracks:
			return l.tr("Tracks");
		case Tree:
			return l.tr("Tree");
		case Undo:
			return l.tr("Undo");
		case UnknownAlbum:
			return l.tr("Unknown album");
		case UnknownArtist:
			return l.tr("Unknown artist");
		case UnknownTitle:
			return l.tr("Unknown title");
		case UnknownGenre:
			return l.tr("Unknown genre");
		case UnknownPlaceholder:
			return l.tr("Unknown placeholder");
		case UnknownYear:
		{
			[[maybe_unused]] const auto s = l.tr("Unknown year");
		}
			return {"-"};

		case Various:
			return l.tr("Various");
		case VariousAlbums:
			return l.tr("Various albums");
		case VariousArtists:
			return l.tr("Various artists");
		case VariousTracks:
			return l.tr("Various tracks");
		case Version:
			return l.tr("Version");
		case VolumeDown:
			return l.tr("Volume down");
		case VolumeUp:
			return l.tr("Volume up");
		case Warning:
			return l.tr("Warning");
		case Weeks:
			return l.tr("Weeks");
		case Year:
			return l.tr("Year");
		case Years:
			return l.tr("Years");
		case Yes:
			return l.tr("Yes");
		case Zoom:
			return l.tr("Zoom");
		case NUMBER_OF_LANGUAGE_KEYS:
			[[fallthrough]];
		default:
			if(ok)
			{
				*ok = false;
			}
			return QString();
	}
}

LanguageString Lang::getWithNumber(TermNr term, int param, bool* ok)
{
	if(ok)
	{
		*ok = true;
	}

	Lang l;

	switch(term)
	{
		case NrDirectories:
			if(param == 0)
			{
				return l.tr("No directories");
			}

			return l.tr("%n directory(s)", "", param);

		case NrFiles:
			if(param == 0)
			{
				return l.tr("No files");
			}

			return l.tr("%n file(s)", "", param);

		case NrPlaylists:
			if(param == 0)
			{
				return l.tr("No playlists");
			}

			return l.tr("%n playlist(s)", "", param);

		case NrTracks:
			if(param == 0)
			{
				return l.tr("No tracks");
			}

			return l.tr("%n track(s)", "", param);

		case NrTracksFound:
			if(param == 0)
			{
				return l.tr("No tracks found");
			}

			return l.tr("%n track(s) found", "", param);

		default:
			if(ok)
			{
				*ok = false;
			}

			return QString();
	}
}

#pragma clang diagnostic pop // "readability-static-accessed-through-instance"

#pragma clang diagnostic pop // "-Wunknown-pragmas"