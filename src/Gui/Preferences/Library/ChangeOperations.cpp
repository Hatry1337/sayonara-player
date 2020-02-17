/* ChangeOperations.cpp */

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

#include "ChangeOperations.h"
#include "Components/LibraryManagement/LibraryManager.h"

#include <QString>

ChangeOperation::ChangeOperation() = default;

ChangeOperation::~ChangeOperation() = default;

Library::Manager* ChangeOperation::manager() const
{
	return Library::Manager::instance();
}

struct MoveOperation::Private
{
	int from, to;
	Private(int from, int to) :
		from(from),
		to(to)
	{}
};

MoveOperation::MoveOperation(int from, int to)
{
	m = Pimpl::make<Private>(from, to);
}

MoveOperation::~MoveOperation() = default;

bool MoveOperation::exec()
{
	return manager()->moveLibrary(m->from, m->to);
}


struct RenameOperation::Private
{
	LibraryId id;
	QString new_name;

	Private(LibraryId id, const QString& new_name) :
		id(id),
		new_name(new_name)
	{}
};

RenameOperation::RenameOperation(LibraryId id, const QString& new_name)
{
	m = Pimpl::make<Private>(id, new_name);
}

RenameOperation::~RenameOperation() = default;

bool RenameOperation::exec()
{
	return manager()->renameLibrary(m->id, m->new_name);
}

struct RemoveOperation::Private
{
	LibraryId id;

	Private(LibraryId id) :
		id(id)
	{}
};

RemoveOperation::RemoveOperation(LibraryId id)
{
	m = Pimpl::make<Private>(id);
}

RemoveOperation::~RemoveOperation() = default;

bool RemoveOperation::exec()
{
	return manager()->removeLibrary(m->id);
}

struct AddOperation::Private
{
	QString name, path;

	Private(const QString& name, const QString& path) :
		name(name),
		path(path)
	{}
};

AddOperation::AddOperation(const QString& name, const QString& path)
{
	m = Pimpl::make<Private>(name, path);
}

AddOperation::~AddOperation() = default;

bool AddOperation::exec()
{
	return (manager()->addLibrary(m->name, m->path) >= 0);
}

struct ChangePathOperation::Private
{
	LibraryId id;
	QString new_path;

	Private(LibraryId id, const QString& new_path) :
		id(id),
		new_path(new_path)
	{}
};

ChangePathOperation::ChangePathOperation(LibraryId id, const QString& new_path)
{
	m = Pimpl::make<Private>(id, new_path);
}

ChangePathOperation::~ChangePathOperation() = default;

bool ChangePathOperation::exec()
{
	return manager()->changeLibraryPath(m->id, m->new_path);
}
