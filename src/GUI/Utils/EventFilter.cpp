/* EventFilter.cpp */

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



#include "EventFilter.h"

#include <QAction>
#include <QContextMenuEvent>
#include <QKeyEvent>

KeyPressFilter::KeyPressFilter(QObject *parent) :
	QObject(parent)
{}


bool KeyPressFilter::eventFilter(QObject *o, QEvent *e)
{
	if(e->type() == QEvent::KeyPress)
	{
		QKeyEvent* ke = static_cast<QKeyEvent*>(e);
		if(ke->key() == Qt::Key_Escape)
		{
			e->accept();
			emit sig_esc_pressed();
		}
	}

	return QObject::eventFilter(o, e);
}



ContextMenuFilter::ContextMenuFilter(QObject* parent) :
	QObject(parent)
{}

bool ContextMenuFilter::eventFilter(QObject *o, QEvent *e)
{
	if(e->type() == QEvent::ContextMenu)
	{
		e->accept();
		QContextMenuEvent* cme = static_cast<QContextMenuEvent*>(e);

		emit sig_context_menu(cme->globalPos(), nullptr);
	}

	return QObject::eventFilter(o, e);
}


MouseMoveFilter::MouseMoveFilter(QObject* parent) :
	QObject(parent)
{}

bool MouseMoveFilter::eventFilter(QObject *o, QEvent *e)
{
	if(e->type() == QEvent::MouseMove)
	{
		e->accept();
		QMouseEvent* me = static_cast<QMouseEvent*>(e);

		emit sig_mouse_moved(me->pos());
	}

	return QObject::eventFilter(o, e);
}



MouseEnterFilter::MouseEnterFilter(QObject *parent) :
	QObject(parent)
{}

bool MouseEnterFilter::eventFilter(QObject *o, QEvent *e)
{
	if(e->type() == QEvent::Enter)
	{
		e->accept();

		emit sig_mouse_entered();
	}

	return QObject::eventFilter(o, e);
}


MouseLeaveFilter::MouseLeaveFilter(QObject *parent) :
	QObject(parent)
{}

bool MouseLeaveFilter::eventFilter(QObject *o, QEvent *e)
{
	if(e->type() == QEvent::Leave)
	{
		e->accept();
		emit sig_mouse_left();
	}

	return QObject::eventFilter(o, e);
}

