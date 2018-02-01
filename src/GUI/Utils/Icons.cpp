// clazy:excludeall=non-pod-global-static

/* Icons.cpp */

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

#include "Icons.h"

#include "Components/Directories/DirectoryReader.h"
#include "GUI/Utils/GuiUtils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include <QIcon>
#include <QMap>
#include <QPair>

using namespace Gui;

static char* s_standard_theme=nullptr;
static bool s_force_standard_icons=false;

using P=QPair<QString, QString>;

static const QMap<Icons::IconName, QPair<QString, QString>> s_icon_map =
{
	{Icons::AudioFile,	P("audio-x-generic", "cd.png")},
	{Icons::Append,		P("list-add", "append")},
	{Icons::Backward,	P("media-skip-backward", "bwd")},
	{Icons::Clear,		P("edit-clear", "broom.png")},
	{Icons::Close,		P("window-close", "close")},
	{Icons::Delete,		P("edit-delete", "delete")},
	{Icons::Dynamic,	P("dynamic", "dynamic")},
	{Icons::Edit,		P("accessories-text-editor", "edit")},
	{Icons::Exit,		P("application-exit", "power_on")},
	{Icons::File,		P("text-x-generic", "file")},
	{Icons::FileManager, P("system-file-manager", "folder")},
	{Icons::Folder,		P("folder", "folder")},
	{Icons::FolderOpen,	P("folder-open", "folder_open")},
	{Icons::FolderVisiting,P("folder-visiting", "folder_open")},
	{Icons::Forward,	P("media-skip-forward", "fwd")},
	{Icons::Gapless,	P("gapless", "gapless")},
	{Icons::Image,		P("image-x-generic", "file")},
	{Icons::ImageFile,	P("image-x-generic", "file")},
	{Icons::Info,		P("dialog-information", "info")},
	{Icons::LocalLibrary, P("audio-x-generic", "append")},
	{Icons::Lyrics,		P("format-justify-left", "lyrics")},
	{Icons::New,		P("document-new", "new")},
	{Icons::Next,		P("media-skip-forward", "fwd")},
	{Icons::Open,		P("document-open", "open")},
	{Icons::Pause,		P("media-playback-pause", "pause")},
	{Icons::Play,		P("media-playback-start", "play")},
	{Icons::PlaySmall,	P("media-playback-start", "play_small")},
	{Icons::PlayBorder,	P("media-playback-start", "play_bordered")},
	{Icons::PlaylistFile, P("text-x-generic", "playlistfile")},
	{Icons::Preferences, P("applications-system", "")},
	{Icons::Previous,	P("media-skip-backward", "bwd")},
	{Icons::Record,		P("media-record", "rec")},
	{Icons::Refresh,	P("view-refresh", "undo")},
	{Icons::Remove,		P("list-remove", "remove")},
	{Icons::Rename,		P("accessories-text-editor", "edit")},
	{Icons::Repeat1,	P("rep_1", "rep_1")},
	{Icons::RepeatAll,	P("rep_all", "rep_all")},
	{Icons::Save,		P("document-save", "save")},
	{Icons::SaveAs,		P("document-save-as", "save_as")},
	{Icons::Search,		P("edit-find", "lupe")},
	{Icons::Shuffle,	P("shuffle", "shuffle")},
	{Icons::Shutdown,	P("system-shutdown", "power_on")},
	{Icons::Star,		P("rating", "star.png")},
	{Icons::StarDisabled, P("star_disabled.png", "star_disabled.png")},
	{Icons::Stop,		P("media-playback-stop", "stop")},
	{Icons::Table,		P("format-justify-fill", "")},
	{Icons::Undo,		P("edit-undo", "undo")},
	{Icons::Vol1,		P("audio-volume-low", "vol_1_dark")},
	{Icons::Vol2,		P("audio-volume-medium", "vol_2_dark")},
	{Icons::Vol3,		P("audio-volume-high", "vol_3_dark")},
	{Icons::VolMute,	P("audio-volume-muted", "vol_mute_dark")},
};


#ifdef Q_OS_WIN
QString get_win_icon_name(const QString& name)
{
	QString icon_name = QString(":/IconsWindows/") + name + ".png";
	return icon_name;
}
#endif

QIcon Icons::icon(Icons::IconName spec, Icons::IconMode mode)
{
	bool dark = (Settings::instance()->get<Set::Player_Style>() == 1);
	QString std_name = s_icon_map[spec].first;
	QString dark_name = s_icon_map[spec].second;

	QIcon icon;

	if(mode == IconMode::ForceSayonaraIcon){
		icon = Gui::Util::icon(dark_name);
	}

	else if(mode == IconMode::ForceStdIcon){
		icon = QIcon::fromTheme(std_name);
	}

	if(icon.isNull())
	{
		if(!dark)
		{
	#ifdef Q_OS_WIN
			icon = QIcon(get_win_icon_name(std_name));
	#else

			icon = QIcon::fromTheme(std_name);
	#endif
		}

		else
		{
	#ifdef Q_OS_WIN
			icon = QIcon(get_win_icon_name(std_name));
	#else
			icon = Gui::Util::icon(dark_name);
	#endif
		}
	}

	if(!icon.isNull())
	{
		return icon;
	}

	return Gui::Util::icon(dark_name);
}


QIcon Icons::icon(IconName spec)
{
	if(s_force_standard_icons){
		return icon(spec, IconMode::ForceStdIcon);
	}

	else return icon(spec, IconMode::Automatic);
}


void Icons::change_theme()
{
	Settings* s = Settings::instance();
	QString theme = s->get<Set::Icon_Theme>();

	QIcon::setThemeName(theme);
}


QPixmap Icons::pixmap(Icons::IconName spec)
{
	if(s_force_standard_icons){
		return pixmap(spec, IconMode::ForceStdIcon);
	}

	return pixmap(spec, IconMode::Automatic);
}

QPixmap Icons::pixmap(Icons::IconName spec, Icons::IconMode mode)
{
	bool dark = (Settings::instance()->get<Set::Player_Style>() == 1);
	QString std_name = s_icon_map[spec].first;
	QString dark_name = s_icon_map[spec].second;

	QPixmap pm;

	if(mode == IconMode::ForceSayonaraIcon){
		pm = Gui::Util::pixmap(dark_name);
	}

	else if(mode == IconMode::ForceStdIcon){
		pm = QIcon::fromTheme(std_name).pixmap(QSize(32,32));
	}

	if(pm.isNull())
	{
		if(!dark)
		{
	#ifdef Q_OS_WIN
			pm = QIcon(get_win_icon_name(std_name)).pixmap(QSize(32,32));
	#else

			pm = QIcon::fromTheme(std_name).pixmap(QSize(32,32));
	#endif
		}

		else
		{
	#ifdef Q_OS_WIN
			pm = QIcon(get_win_icon_name(std_name)).pixmap(QSize(32,32));
	#else
			pm = Gui::Util::pixmap(dark_name);
	#endif
		}
	}

	if(!pm.isNull())
	{
		return pm;
	}

	return Gui::Util::pixmap(dark_name);
}

void Icons::set_standard_theme(const QString& name)
{
	s_standard_theme = strdup(name.toLocal8Bit().data());
}

QString Icons::standard_theme()
{
	return QString(s_standard_theme);
}

void Icons::force_standard_icons(bool b)
{
	s_force_standard_icons = b;
}
