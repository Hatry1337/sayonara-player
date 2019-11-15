/* ShortcutHandler.h */

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

#ifndef SHORTCUTHANDLER_H
#define SHORTCUTHANDLER_H


#include "ShortcutIdentifier.h"
#include "Utils/Singleton.h"
#include "Utils/Pimpl.h"

#include <QShortcut>
#include <QObject>

class Shortcut;

#define ShortcutPrivate private

/**
 * @brief A singleton class for retrieving shortcuts
 * @ingroup Shortcuts
 */
class ShortcutHandler :
	public QObject
{
	Q_OBJECT
	SINGLETON(ShortcutHandler)
	PIMPL(ShortcutHandler)

	friend class Shortcut;
ShortcutPrivate:
	void qt_shortcuts_added(ShortcutIdentifier db_key, const QList<QShortcut*>& qt_shortcuts);

signals:
	void sig_shortcut_changed(ShortcutIdentifier db_key);

public:
	/**
	 * @brief get a shortcut by its unique identifier
	 * @param identifier the identifier which is used in database
	 * @return a shortcut instance
	 */
	Shortcut shortcut(ShortcutIdentifier db_key) const;

	/**
	 * @brief set the shortcut by its unique identifier
	 * @param identifier  the identifier which is used in database
	 * @param shortcut a shortcut instance
	 */
	void set_shortcut(ShortcutIdentifier db_key, const QStringList& shortcut);

	/**
	 * @brief get all shortcuts
	 * @return
	 */
	QList<ShortcutIdentifier> all_identifiers() const;

	QString db_key(ShortcutIdentifier id) const;
	QString shortcut_text(ShortcutIdentifier id) const;

private slots:
	void qt_shortcut_destroyed();
};

#endif // SHORTCUTHANDLER_H
