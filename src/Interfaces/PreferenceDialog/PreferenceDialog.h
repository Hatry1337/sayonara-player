/* PreferenceDialog.h */

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



#ifndef PREFERENCEDIALOG_H
#define PREFERENCEDIALOG_H

#include "GUI/Utils/Widgets/Dialog.h"
#include <QList>

class QString;
class QAction;
class QMenu;

namespace Preferences
{
	class Base;
}

class PreferenceDialog :
	public Gui::Dialog
{

public:
	using Gui::Dialog::Dialog;

	virtual QString action_name() const=0;
	virtual QAction* action()=0;

	virtual void register_preference_dialog(Preferences::Base* dialog)=0;
	virtual void show_preference_dialog(const QString& identifier)=0;

	virtual QList<QAction*> actions()=0;
};

#endif // PREFERENCEDIALOG_H
