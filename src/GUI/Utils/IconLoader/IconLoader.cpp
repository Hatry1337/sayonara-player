/* IconLoader.cpp */

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

#include "IconLoader.h"

#include "Components/DirectoryReader/DirectoryReader.h"
#include "GUI/Utils/GuiUtils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include <QIcon>
#include <QFile>
#include <QRegExp>
#include <QMap>
#include <QIcon>

struct IconLoader::Private
{
	QString 				theme;
	QStringList				theme_paths;
	QMap<QString, QIcon> 	icons;

	Private()
	{
		theme_paths = QIcon::themeSearchPaths();

		init_theme();
	}

	void init_theme()
	{
		QString new_theme;
		icons.clear();

        QString t = Settings::instance()->get(Set::Icon_Theme);

		if(t.isEmpty()) {
			theme = QIcon::themeName();
		}

		else {
			theme = t;
		}

		QIcon::setThemeName(theme);
	}

	void change_theme()
	{
		init_theme();
        Settings::instance()->shout(Set::Player_Style);
	}
};


#ifdef Q_OS_WIN
	QString get_win_icon_name(const QString& name)
	{
		QString icon_name = QString(":/IconsWindows/") + name + ".png";
		return icon_name;
	}
#endif

IconLoader::IconLoader() :
    QObject(),
    SayonaraClass()
{
	m = Pimpl::make<Private>();

#ifndef Q_OS_WIN
	for(const QString& theme_path : m->theme_paths)
	{
		QString full_theme_path = theme_path + "/" + m->theme;
		QString index_path = full_theme_path + "/theme.index";
		if(!QFile::exists(index_path)){
			continue;
		}
	}
#endif

	QIcon::setThemeSearchPaths(m->theme_paths);
	sp_log(Log::Debug, this) << "Theme paths " << m->theme_paths;

	Set::listen(Set::Icon_Theme, this, &IconLoader::icon_theme_changed, false);
}

IconLoader::~IconLoader() {}

void IconLoader::icon_theme_changed()
{
	m->change_theme();
}


void IconLoader::add_icon_names(const QStringList& icon_names)
{
	#ifdef Q_OS_WIN
		return;
	#endif

	DirectoryReader dir_reader;
	dir_reader.set_filter("*.png");

	for(const QString& icon_name : icon_names){
		QIcon icon = QIcon::fromTheme(icon_name);
		if( !icon.isNull() ){
			m->icons[icon_name] = icon;
			//sp_log(Log::Debug, this) << "Could load icon from theme: " << icon_name;
			continue;
		}

		for(const QString& theme_path : m->theme_paths){
			bool found = false;
			QString full_theme_path = theme_path + "/" + m->theme;
			//QString full_theme_path = theme_path;


			QDir full_theme_dir(full_theme_path);
			//sp_log(Log::Debug, this) << full_theme_dir.canonicalPath();


			QStringList files = dir_reader.find_files_rec(full_theme_dir, icon_name);
			//sp_log(Log::Debug, this) << "Search for " << icon_name << " in " << full_theme_path << ": " << files;

			for(const QString& file : files){
				if(file.contains("48")){
					found = true;
				}

				else if(file.contains("32")){
					m->icons[icon_name] = QIcon(file);
					found = true;
				}

				else if(file.contains("24")){
					found = true;
				}

				if(found){
					//sp_log(Log::Debug, this) << "Found icon " << icon_name << " in " << file;
					m->icons[icon_name] = QIcon(file);
					break;
				}
			}

			if(found){
				break;
			}
		}
	}
}

QIcon IconLoader::icon(const QString& name, const QString& dark_name)
{
    bool dark = (_settings->get(Set::Player_Style) == 1);

	if(!dark){
		if(!has_std_icon(name)){
			QStringList lst; lst << name;
			add_icon_names(lst);

			if(has_std_icon(name)){
#ifdef Q_OS_WIN
				return QIcon(get_win_icon_name(name));
#else
				return m->icons[name];
#endif
			}
		}

		else{
#ifdef Q_OS_WIN
			return QIcon(get_win_icon_name(name));
#else
			return m->icons[name];
#endif
		}
	}

	return Gui::Util::icon(dark_name);
}

QIcon IconLoader::icon(const QStringList& names, const QString& dark_name)
{
    bool dark = (_settings->get(Set::Player_Style) == 1);

	if(!dark){
		for(const QString& name : names){
			if(!has_std_icon(name)){
				QStringList lst; lst << name;
				add_icon_names(lst);

				if(has_std_icon(name)){
	#ifdef Q_OS_WIN
					return QIcon(get_win_icon_name(name));
	#else
					return m->icons[name];
	#endif
				}
			}

			else{
	#ifdef Q_OS_WIN
				return QIcon(get_win_icon_name(name));
	#else
				return m->icons[name];
	#endif
			}
		}
	}

	return Gui::Util::icon(dark_name);
}

bool IconLoader::has_std_icon(const QString& name) const
{
#ifdef Q_OS_WIN
	QIcon icon = QIcon( get_win_icon_name(name) );
	if(icon.isNull())
	{
		return false;
	}

	return true;

#endif

	return m->icons.contains(name);
}