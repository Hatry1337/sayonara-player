/* SomaFMLibraryContainer.h */

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


/* SomaFMLibraryContainer.h */

#ifndef GUI_SOMAFMLIBRARY_CONTAINER_H
#define GUI_SOMAFMLIBRARY_CONTAINER_H

#include "Interfaces/LibraryInterface/LibraryContainer/LibraryContainer.h"
#include <QtGlobal>
#include <QtPlugin>

namespace SomaFM
{
	class GUI_SomaFM;

	class LibraryContainer :
		public LibraryContainerInterface
	{
		Q_OBJECT

		Q_PLUGIN_METADATA(IID "com.sayonara-player.somafm_library")
		Q_INTERFACES(LibraryContainerInterface)

	private:
		GUI_SomaFM*	ui=nullptr;

	public:

		explicit LibraryContainer(QObject* parent=nullptr);
		~LibraryContainer();

		// override from LibraryViewInterface
		QString			name() const override;
		QString			display_name() const override;
		QWidget*		widget() const override;
		QPixmap			icon() const override;
		QMenu*			menu() override;
		QFrame*			header() const override;
		void			init_ui() override;
	};
}
#endif
