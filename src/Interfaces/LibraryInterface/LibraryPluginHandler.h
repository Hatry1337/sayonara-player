/* LibraryPluginLoader.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#ifndef LIBRARYPLUGINLOADER_H
#define LIBRARYPLUGINLOADER_H

#include "Utils/Singleton.h"
#include "Utils/Pimpl.h"

#include <QObject>

class QMenu;
namespace Library
{
	class Info;
	class Container;

	/**
	 * @brief Library Plugin Manager
	 * @ingroup LibraryPlugins
	 */
	class PluginHandler :
			public QObject
	{
		Q_OBJECT
		PIMPL(PluginHandler)
		SINGLETON(PluginHandler)

	signals:
		void sig_current_library_changed(const QString& name);
		void sig_libraries_changed();

	private:
		/**
		 * @brief Init a library. This is used at startup for the current library
		 * or when the index has changed
		 * @param idx
		 */
		void init_library(Container* container);


	public:
		/**
		 * @brief Search for plugins and add some predefined plugins
		 * @param containers Some predefined plugins
		 */
		void init(const QList<Container*>& containers);


		/**
		 * @brief Get a list for all found plugins. The ui is not necessarily initialized
		 * @return list for all found library plugins
		 */
		QList<Container*> get_libraries() const;

		void local_library_added(LibraryId id);
		void local_library_renamed(LibraryId id);
		void local_library_removed(LibraryId id);
		void local_library_moved(LibraryId id, int from, int to);

		Container* current_library() const;
		QMenu* current_library_menu() const;

	private slots:
		void current_library_changed(int library_idx);

	public slots:
		void set_current_library(const QString& name);
		void set_current_library(Container* container);
	};
}

#endif // LIBRARYPLUGINLOADER_H
