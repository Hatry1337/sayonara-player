/* PreferenceInterface.h */

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

#ifndef PREFERENCEINTERFACE_H
#define PREFERENCEINTERFACE_H

#include <QAction>

class QWidget;
class QString;

namespace Preferences
{
	/**
	 * @brief The action, which is used to access the Preference.
	 *
	 * This action is generated by and handled by the PreferenceInterface.
	 * Usually you don't get in touch with that class.
	 * @ingroup Preferences
	 */
	class Action : public QAction
	{
		Q_OBJECT

	public:
		/**
		 * @brief PreferenceAction Create QAction object, which is automatically
		 * connected to the show event of the underlying widget.
		 * @param text text of the action
		 * @param preference_interface Widget, that should appear when action is triggered
		 */
		Action(const QString& text, QWidget* preference_interface);
	};
}

#endif // PREFERENCEINTERFACE_H