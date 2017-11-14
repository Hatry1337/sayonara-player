/* AbstractDatabase.h */

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

#ifndef ABSTRACTDATABASE_H
#define ABSTRACTDATABASE_H

#include "Utils/Pimpl.h"
#include <QObject>

class QSqlDatabase;

namespace DB
{
	class Base : public QObject
	{
		PIMPL(Base)

		public:
			explicit Base(DbId db_id, const QString& db_dir, const QString& db_name, QObject *parent=nullptr);
			virtual ~Base();

			virtual bool close_db();
			virtual bool is_initialized();

			virtual void transaction();
			virtual void commit();
			virtual void rollback();

			DbId db_id() const;


		protected:
			virtual bool exists();
			virtual bool create_db();
			virtual QSqlDatabase open_db();
			virtual bool apply_fixes()=0;

			virtual bool check_and_insert_column(const QString& tablename, const QString& column, const QString& sqltype, const QString& default_value=QString());
			virtual bool check_and_create_table(const QString& tablename, const QString& sql_create_str);
			virtual bool check_and_drop_table(const QString& tablename);


			QSqlDatabase db() const;
	};
}

#endif // ABSTRACTDATABASE_H
