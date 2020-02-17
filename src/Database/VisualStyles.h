/* DatabaseVisualStyles.h */

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

#ifndef DATABASEVISSTYLES_H
#define DATABASEVISSTYLES_H

#include "Database/Module.h"
#include <QList>

struct RawColorStyle;

namespace DB
{
	class VisualStyles : private Module
	{
		public:
			VisualStyles(const QString& connectionName, DbId databaseId);
			~VisualStyles();

			QList<RawColorStyle> getRawColorStyles();
			bool insertRawColorStyle(const RawColorStyle& rcs);
			bool updateRawColorStyle(const RawColorStyle& rcs);
			bool rawColorStyleExists(QString name);
			bool deleteRawColorStyle(QString name);
	};
}

#endif // DATABASEVISSTYLES_H
