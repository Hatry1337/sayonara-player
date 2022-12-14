/* Query.h */

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

#ifndef Query_H
#define Query_H

#include "Utils/Pimpl.h"

#include <QString>
#include <QSqlQuery>
#include <QVariant>
#include <QSqlDatabase>
#include <QSqlError>

namespace DB
{
	class Module;
	class Query :
			public QSqlQuery
	{
		PIMPL(Query)

		private:
			explicit Query(QSqlDatabase db);

		public:
			explicit Query(const Module* module);

			Query(const QString& connection_name, DbId databaseId);
			Query(const Query& other);

            explicit Query(QSqlResult* result) = delete;
            explicit Query(const QString& query=QString(), const QSqlDatabase& db = QSqlDatabase()) = delete;

			Query& operator=(const Query& other);

			virtual ~Query();

			bool prepare(const QString& query);
			void bindValue(const QString&  placeholder, const QVariant & val, QSql::ParamType paramType = QSql::In);
			bool exec();
			void setError(bool b);
			bool hasError() const;

			QString getQueryString() const;
			void showQuery() const;
			void showError(const QString& err_msg) const;


			size_t fetchedRows();
	};
}

#endif // Query_H
