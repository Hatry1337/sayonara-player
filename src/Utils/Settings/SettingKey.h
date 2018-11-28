/* SettingKey.h */

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

 // clazy:excludeall=non-pod-global-static

#ifndef SETTINGKEY_H
#define SETTINGKEY_H

#include "Utils/typedefs.h"
#include "SettingKeyEnum.h"

class QString;
class QStringList;
class QPoint;
class QSize;
class QByteArray;

class EQ_Setting;
struct RawShortcutMap;

namespace Playlist
{
	class Mode;
}

namespace Library
{
	class Sortings;
	class Info;
}

template<typename DataType, SettingKey keyIndex>
class SettingIdentifier
{
	public:
		using Data=DataType;
		const static SettingKey key=keyIndex;

	private:
		SettingIdentifier()=delete;
		~SettingIdentifier();
};


#define INST_ABSTR(ns, type, settingkey) \
	namespace ns {	using settingkey = SettingIdentifier<type, SettingKey:: settingkey>; }

#define INST(type, settingkey)	INST_ABSTR(Set, type, settingkey)
#define INST_NO_DB(type, settingkey) INST_ABSTR(SetNoDB, type, settingkey)


/**
 * @brief Set namespace defines the setting: Which key and which type
 * @ingroup Settings
 */
	//typedef SettingKey<bool, SK::LFM_Active> LFM_Active_t; const LFM_Active_t LFM_Active
	INST(bool,				LFM_Active)				/* is lastFM active? */
	INST(int,				LFM_ScrobbleTimeSec)	/* time in sec when to scrobble */
	INST(StringPair,		LFM_Login)				/* deprecated: 2-Tupel, username, password */
	INST(QString,			LFM_Username)			/* username*/
	INST(QString,			LFM_Password)			/* encrypted password */

	INST(bool,				LFM_Corrections)		/* propose lfm corrections */
	INST(bool,				LFM_ShowErrors)			/* get error message, if there are lfm problems */
	INST(QString,			LFM_SessionKey)			/* lfm session key */

	INST(int,				Eq_Last)				/* last equalizer index */
	INST(QList<EQ_Setting>,		Eq_List)			/* All equalizers */
	INST(bool,				Eq_Gauss)				/* do curve, when changing eq setting */

	INST(bool,				Lib_Show)				/* show library */
	INST(QString,			Lib_Path)					// deprecated
	INST(BoolList,			Lib_ColsTitle)			/* shown columns tracks */
	INST(BoolList,			Lib_ColsArtist)			/* shown columns artist */
	INST(BoolList,			Lib_ColsAlbum)				/* shown columns albums */
	INST(bool,				Lib_LiveSearch)			/* library live search */
	INST(::Library::Sortings,		Lib_Sorting)		/* how to sort in lib */
	INST(QString,			Lib_CurPlugin)				/* Current shown library plugin */
	INST(QByteArray,		Lib_SplitterStateArtist)	/* Splitter state between artists and albums */
	INST(QByteArray,		Lib_SplitterStateTrack)	/* Splitter state between artists and tracks */
	INST(QByteArray,		Lib_SplitterStateGenre)	/* Splitter state between tracks and genres */
	INST(QByteArray,		Lib_SplitterStateDate)		/* Splitter state between tracks and genres */
	INST(int,				Lib_OldWidth)				/* Old library width when hiding library */
	INST(bool,				Lib_DC_DoNothing)			/* when double clicked, create playlist and do nothing*/
	INST(bool,				Lib_DC_PlayIfStopped)		/* when double clicked, play if stopped */
	INST(bool,				Lib_DC_PlayImmediately)	/* when double clicked, play immediately */
	INST(bool,				Lib_DD_DoNothing)			/* when drag dropped, insert tracks and do nothing */
	INST(bool,				Lib_DD_PlayIfStoppedAndEmpty)	/* when drag dropped, play if playlist is empty and stopped */
	INST(int,				Lib_FontSize)				/* current library font size */
	INST(bool,				Lib_FontBold)				/* current library font weight */
	INST(int,				Lib_SearchMode)			/* Search mode in library. See */
	INST(bool,				Lib_AutoUpdate)			/* Automatic update of library */
	INST(bool,				Lib_ShowAlbumArtists)		/* Show album artists instead of artists */
	INST(bool,				Lib_ShowAlbumCovers)		/* Show album cover view */
	INST(int,				Lib_CoverZoom)				/* Zoom of album cover view */
	INST(bool,				Lib_CoverShowUtils)		/* Show utils bar in cover view */
	INST(bool,				Lib_CoverShowArtist)	/* Show artist name in cover view */
	INST(bool,				Lib_GenreTree)				/* Show tree view of genres */
	INST(QList<::Library::Info>, Lib_AllLibraries)		// deprecated
	INST(int,				Lib_LastIndex)				/* Last selected library */
	INST(bool,				Lib_UseViewClearButton)	/* Show clear button in single view */
	INST(bool,				Lib_ShowFilterExtBar) /* Show the file extension filter bar in track view */

	INST(bool,				Dir_ShowTracks)			/* show tracks panel in directory view */
	INST(QByteArray,		Dir_SplitterDirFile)		/* Splitter state between dirs and files */
	INST(QByteArray,		Dir_SplitterTracks)		/* Splitter between upper and track view */


	INST(QString,			Player_Version)			/* Version string of player */
	INST(QString,			Player_Language)			/* language of player */
	INST(int,				Player_Style)				/* dark or native: native = 0, dark = 1 */
	INST(int,				Player_ControlStyle)		/* Big cover or not */
	INST(QString,			Player_FontName)			/* current font name */
	INST(int,				Player_FontSize)			/* current font size */
	INST(QSize,				Player_Size)				/* player size */
	INST(QPoint,			Player_Pos)				/* player position */
	INST(bool,				Player_Fullscreen)			/* player fullscreen */
	INST(bool,				Player_Maximized)			/* player maximized */
	INST(QString,			Player_ShownPlugin)			/* current shown plugin in player, empty if none */
	INST(bool,				Player_OneInstance)			/* only one Sayonara instance is allowed */
	INST(bool,				Player_Min2Tray)			/* minimize Sayonara to tray */
	INST(bool,				Player_ShowTrayIcon)			/* Show/hide the tray icon */
	INST(bool,				Player_StartInTray)			/* start in tray */
	INST(bool,				Player_NotifyNewVersion)		/* check for new version on startup */
	INST(QByteArray,		Player_SplitterState)			/* spliter state between playlist and library */
	INST(RawShortcutMap,	Player_Shortcuts)			/* player shortcuts */
	INST(QByteArray,		Player_SplitterControls)	/* Splitter state between controls and playlist */
	INST(QByteArray,		Player_PrivId)				/* Unique identifier */
	INST(QByteArray,		Player_PublicId)				/* Unique identifier */

	INST(QStringList,		PL_Playlist)				/* old playlist: list of integers in case of library tracks, if no library track, filepath */
	INST(bool,				PL_LoadSavedPlaylists)			/* load saved playlists on startup */
	INST(bool,				PL_LoadTemporaryPlaylists)		/* load temporary playlists on startup */
	INST(bool,				PL_LoadLastTrack)			/* load last track on startup */
	INST(bool,				PL_RememberTime)			/* remember time of last track */
	INST(bool,				PL_StartPlaying)			/* start playing immediately when opening Sayonara */
	INST(int,				PL_LastTrack)				/* last track idx in playlist */
	INST(int,				PL_LastTrackBeforeStop)		/* last track before stop */
	INST(int,				PL_LastPlaylist)			/* last Playlist id, where LastTrack has been played */
	INST(QString,			PL_EntryLook)				/* formatting of playlist entry */
	INST(int,				PL_FontSize)				/* current playlist font size */
	INST(bool,				PL_ShowClearButton)		/* show clear button in playlist */
	INST(Playlist::Mode,	PL_Mode)					/* playlist mode: rep1, repAll, shuffle... */
	INST(bool,				PL_ShowNumbers)			/* show numbers in playlist */
	INST(bool,				PL_RememberTrackAfterStop)	/* when stop button is pressed, remember last track index */
	INST(bool,				PL_ShowCovers)				/* Show covers in Playlist */
	INST(bool,				PL_ShowRating)				/* Show rating in playlist */

	INST(bool,				Notification_Show)			/* show notifications */
	INST(int,				Notification_Timeout)		/* notification timeout */
	INST(QString,			Notification_Name)			/* type of notifications: libnotify or empty for native baloons :( */

	INST(QString,			Engine_Name)				/* Deprecated: Engine name */
	INST(int,				Engine_Vol)				/* Volume */
	INST(bool,				Engine_Mute)				/* Muted/unmuted */
	INST(int,				Engine_CurTrackPos_s)			/* position of track (used to load old position) */
	INST(int,				Engine_ConvertNumberThreads)	/* Number of threads */
	INST(QString,			Engine_ConvertPreferredConverter)		/* Preferred Converter: ogg, lame cbr, lame vbr */
	INST(int,				Engine_ConvertQualityLameVBR)	/* Lame Quality for variable bitrate 1-10 */
	INST(int,				Engine_ConvertQualityLameCBR)	/* 64 - 320 */
	INST(int,				Engine_ConvertQualityOgg)		/* 1 - 10 */
	INST(QString,			Engine_CovertTargetPath)		/* last convert path */
	INST(int,				Engine_SpectrumBins)			/* number of spectrum bins */
	INST(bool,				Engine_ShowSpectrum)			/* show spectrum */
	INST(bool,				Engine_ShowLevel)			/* show level */
	INST(bool,				Engine_CrossFaderActive)		/* crossfader, but not gapless active */
	INST(int,				Engine_CrossFaderTime)			/* crossfader overlap time */
	INST(int, 				Engine_Pitch)				/* hertz of a */
	INST(bool, 				Engine_SpeedActive)			/* is speed control active? */
	INST(float,				Engine_Speed)				/* if yes, set speed */
	INST(bool, 				Engine_PreservePitch)			/* if yes, should pitch be preserved? */
	INST(QString,			Engine_Sink)				/* Alsa, pulseaudio */

	INST(bool,				Engine_SR_Active)			/* Streamripper active */
	INST(bool,				Engine_SR_Warning)			/* streamripper warnings */
	INST(QString,			Engine_SR_Path)			/* streamripper paths */
	INST(bool,				Engine_SR_SessionPath)			/* create streamripper session path? */
	INST(QString,			Engine_SR_SessionPathTemplate)	/* streamripper session path template*/
	INST(bool,				Engine_SR_AutoRecord)			/* streamripper automatic recording */

	INST(int,				Spectrum_Style)			/* index of spectrum style */
	INST(int,				Level_Style)				/* index of level style */
	INST(bool,				Broadcast_Active)			/* is broadcast active? */
	INST(bool,				Broadcast_Prompt)			/* prompt when new connection arrives? */
	INST(int,				Broadcast_Port)			/* broadcast port */

	INST(bool,				Remote_Active)				/* Remote control activated */
	INST(int,				Remote_Port)				/* Remote control port */

	INST(bool,				Stream_NewTab)				/* Open Streams in new tab */
	INST(bool,				Stream_ShowHistory)		/* Show history when playing streams */

	INST(int,				Lyrics_Zoom)				/* Zoom factor in lyrics window */
	INST(QString,			Lyrics_Server)				/* Lyrics server */

	INST(QStringList,		Cover_Server)				/* Cover server */
	INST(bool,				Cover_FetchFromWWW)			/* Fetch covers from www */
	INST(bool,				Cover_StartSearch)		/* start alternative cover search automatically */
	INST(QString,			Icon_Theme)				/* Current icon theme */
	INST(bool,				Icon_ForceInDarkTheme)		/* Current icon theme */

	INST(bool,				Proxy_Active)				/* Is proxy server active */
	INST(QString,			Proxy_Username)			/* Proxy Username */
	INST(QString,			Proxy_Password)			/* Proxy Password */
	INST(QString,			Proxy_Hostname)			/* Proxy Hostname/IP Address */
	INST(int,				Proxy_Port)				/* Proxy Port 3128 */
	INST(bool,				Proxy_SavePw)				/* Should password be saved */

	INST(int,				Settings_Revision)		/* Version number of settings */

	INST(int,				Logger_Level)				/* Also log development: */

	INST_NO_DB(bool,				MP3enc_found)
	INST_NO_DB(bool,				Pitch_found)
	INST_NO_DB(bool,				Player_Quit)

#endif // SETTINGKEY_H
