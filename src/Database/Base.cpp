/* AbstractDatabase.cpp */

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

#include "Database/Base.h"
#include "Database/Module.h"
#include "Database/Query.h"

#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/FileUtils.h"

#include <QFile>
#include <QDir>

using DB::Base;
using DB::Query;

namespace FileUtils=::Util::File;

struct Base::Private
{
	QString source_dir;
	QString filename;		// player.db
	QString connection_name;		// /home/user/.Sayonara/player.db
	DbId	db_id;

	bool initialized;

	Private(DbId db_id, const QString& source_dir, const QString& target_dir, const QString& filename) :
		source_dir(source_dir),
		filename(filename),
		db_id(db_id)
	{
		connection_name = target_dir + "/" +filename;
	}
};


Base::Base(DbId db_id, const QString& source_dir, const QString& target_dir, const QString& filename, QObject* parent) :
	QObject(parent),
	DB::Module(target_dir + "/" + filename, db_id)
{
	m = Pimpl::make<Private>(db_id, source_dir, target_dir, filename);
	bool success = FileUtils::exists(m->connection_name);

	if(!success)
	{
		sp_log(Log::Info, this) << "Database not existent. Creating database...";
		success = create_db();
	}

	m->initialized = success && this->db().isOpen();

	if(!m->initialized) {
		sp_log(Log::Error, this) << "Database is not open";
	}
}


DB::Base::~Base() = default;


bool Base::is_initialized()
{
	return m->initialized;
}

bool Base::close_db()
{
	if(!QSqlDatabase::isDriverAvailable("QSQLITE")){
		return false;
	}

	QStringList connection_names = QSqlDatabase::connectionNames();
	if(!connection_names.contains(m->connection_name)){
		return false;
	}

	sp_log(Log::Info, this) << "close database " << m->filename << "...";

	QSqlDatabase database = db();
	if(database.isOpen()){
		database.close();
	}

	return true;
}


bool Base::create_db()
{
	bool success;
	QDir dir = QDir::homePath();

	QString sayonara_path = Util::sayonara_path();
	if(!FileUtils::exists(sayonara_path))
	{
		success = dir.mkdir(".Sayonara");
		if(!success) {
			sp_log(Log::Error, this) << "Could not create .Sayonara dir";
			return false;
		}

		else{
			sp_log(Log::Info, this) << "Successfully created .Sayonara dir";
		}
	}

	success = dir.cd(sayonara_path);

	//if ret is still not true we are not able to create the directory
	if(!success) {
		sp_log(Log::Error, this) << "Could not change to .Sayonara dir";
		return false;
	}

	QString source_db_file = QDir(m->source_dir).absoluteFilePath(m->filename);

	success = FileUtils::exists(m->connection_name);

	if(success) {
		return true;
	}

	if(!success)
	{
		sp_log(Log::Info, this) << "Database " << m->connection_name << " not existent yet";
		sp_log(Log::Info, this) << "Copy " <<  source_db_file << " to " << m->connection_name;

		success = QFile::copy(source_db_file, m->connection_name);

		if(success)
		{
			QFile f(m->connection_name);
			f.setPermissions
			(
				f.permissions() |
				QFile::Permission::WriteOwner | QFile::Permission::WriteUser |
				QFile::Permission::ReadOwner | QFile::Permission::ReadUser
			);

			sp_log(Log::Info, this) << "DB file has been copied to " <<   m->connection_name;
		}

		else
		{
			sp_log(Log::Error, this) << "Fatal Error: could not copy DB file to " << m->connection_name;
		}
	}

	return success;
}


void Base::transaction()
{
	db().transaction();
}

void Base::commit()
{
	db().commit();
}

void Base::rollback()
{
	db().rollback();
}



bool Base::check_and_drop_table(const QString& tablename)
{
	Query q(this);
	QString querytext = "DROP TABLE IF EXISTS " +  tablename + ";";
	q.prepare(querytext);

	if(!q.exec()){
		q.show_error(QString("Cannot drop table ") + tablename);
		return false;
	}

	return true;
}


bool DB::Base::check_and_insert_column(const QString& tablename, const QString& column, const QString& sqltype)
{
    return check_and_insert_column(tablename, column, sqltype, QString());
}

bool Base::check_and_insert_column(const QString& tablename, const QString& column, const QString& sqltype, const QString& default_value)
{
	Query q(this);
	QString querytext = "SELECT " + column + " FROM " + tablename + ";";
	q.prepare(querytext);

	if(!q.exec())
	{
		Query q2 (this);
		querytext = "ALTER TABLE " + tablename + " ADD COLUMN " + column + " " + sqltype;
		if(!default_value.isEmpty()){
			querytext += " DEFAULT " + default_value;
		}

		querytext += ";";

		q2.prepare(querytext);

		if(!q2.exec())
		{
			q.show_error(QString("Cannot insert column ") + column + " into " + tablename);
			return false;
		}

		return true;
	}

	return true;
}

bool Base::check_and_create_table(const QString& tablename, const QString& sql_create_str)
{
	Query q(this);
	QString querytext = "SELECT * FROM " + tablename + ";";
	q.prepare(querytext);

	if(!q.exec())
	{
		Query q2(this);
		q2.prepare(sql_create_str);

		if(!q2.exec()){
			q.show_error(QString("Cannot create table ") + tablename);
			return false;
		}
	}

	return true;
}
