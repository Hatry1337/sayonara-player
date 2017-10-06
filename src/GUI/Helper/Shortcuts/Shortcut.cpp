/* Shortcut.cpp */

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

#include "Shortcut.h"
#include "ShortcutHandler.h"
#include "ShortcutWidget.h"
#include "RawShortcutMap.h"
#include "GUI/Helper/SayonaraWidget/SayonaraWidget.h"
#include "Helper/Settings/Settings.h"
#include "Helper/Logger/Logger.h"

#include <QKeySequence>
#include <QWidget>

struct Shortcut::Private
{
	QStringList			default_shortcuts;
	QStringList			shortcuts;
	QString				name;
	QString				identifier;
	QList<QShortcut*>	qt_shortcuts;
	ShortcutWidget*		parent=nullptr;
};

Shortcut::Shortcut() :
	SayonaraClass()
{
	m = Pimpl::make<Private>();
}

Shortcut::Shortcut(ShortcutWidget* parent, const QString& identifier, const QString& name, const QStringList& default_shortcuts) :
	Shortcut()
{
	m->name = name;
	m->identifier = identifier;
	m->parent=parent;

	m->default_shortcuts = default_shortcuts;
	for(QString& str : m->default_shortcuts){
		str.replace(" +", "+");
		str.replace("+ ", "+");
	}

	m->shortcuts = m->default_shortcuts;

	RawShortcutMap rsm = _settings->get(Set::Player_Shortcuts);

	if(rsm.contains(identifier)){
		m->shortcuts = rsm[identifier];
	}

	else{
		m->shortcuts = m->default_shortcuts;
	}
}

Shortcut::Shortcut(ShortcutWidget* parent, const QString& identifier, const QString& name, const QString& default_shortcut) :
	Shortcut(parent, identifier, name, QStringList(default_shortcut)) {}

Shortcut::Shortcut(const Shortcut& other) :
	Shortcut()
{
	m->parent =				other.m->parent;
	m->name =					other.m->name;
	m->identifier =			other.m->identifier;
	m->default_shortcuts =		other.m->default_shortcuts;
	m->shortcuts =				other.m->shortcuts;
	m->qt_shortcuts =			other.m->qt_shortcuts;
}

Shortcut::~Shortcut() {}

Shortcut& Shortcut::operator =(const Shortcut& other)
{
	m->parent =				other.m->parent;
	m->name =					other.m->name;
	m->identifier =			other.m->identifier;
	m->default_shortcuts =		other.m->default_shortcuts;
	m->shortcuts =				other.m->shortcuts;
	m->qt_shortcuts =			other.m->qt_shortcuts;

	return (*this);
}



QString Shortcut::get_name() const
{
	if(m->parent){
		QString name = m->parent->get_shortcut_text(m->identifier);
		if(!name.isEmpty()){
			return name;
		}
	}
	return m->name;
}

QStringList Shortcut::get_default() const
{
	return m->default_shortcuts;
}

QList<QKeySequence> Shortcut::get_sequences() const
{
	QList<QKeySequence> sequences;
	for(const QString& str : get_shortcuts()){
		QKeySequence seq = QKeySequence::fromString(str, QKeySequence::NativeText);
		sequences << seq;
	}

	return sequences;
}

QStringList Shortcut::get_shortcuts() const
{
	return m->shortcuts;
}

QString Shortcut::get_identifier() const
{
	return m->identifier;
}

Shortcut Shortcut::getInvalid()
{
	return Shortcut();
}

bool Shortcut::is_valid() const
{
	return !(m->identifier.isEmpty());
}


void Shortcut::create_qt_shortcut(QWidget* parent, QObject* receiver, const char* slot)
{
	QList<QShortcut*> shortcuts = init_qt_shortcut(parent);
	for(QShortcut* sc : shortcuts){
		parent->connect(sc, SIGNAL(activated()), receiver, slot);
	}
}


QList<QShortcut*> Shortcut::init_qt_shortcut(QWidget* parent)
{
	QList<QShortcut*> lst;
	if(get_sequences().size() > 1){
		sp_log(Log::Debug, this) << "Number of shortcuts: " << get_sequences().size();
	}

	for(const QKeySequence& sequence : get_sequences()){
		QShortcut* shortcut = new QShortcut(parent);

		shortcut->setContext(Qt::WindowShortcut);
		shortcut->setKey(sequence);

		m->qt_shortcuts << shortcut;

		lst << shortcut;
	}

	ShortcutHandler::instance()->set_shortcut(*this);

	return lst;
}


void Shortcut::change_shortcut(const QStringList &shortcuts){
	m->shortcuts = shortcuts;
	for(QString& str : m->shortcuts){
		str.replace(" +", "+");
		str.replace("+ ", "+");
	}

	for(QShortcut* sc : m->qt_shortcuts){
		QList<QKeySequence> sequences = get_sequences();
		for(const QKeySequence& ks : sequences){
			sc->setKey(ks);
		}
	}

	ShortcutHandler::instance()->set_shortcut(*this);
}
