/* ComboBox.cpp */

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



#include "ComboBox.h"
#include "GUI/Utils/Delegates/ComboBoxDelegate.h"
#include "GUI/Utils/Delegates/StyledItemDelegate.h"
#include "Utils/Settings/Settings.h"

#include <QAbstractItemView>
#include <QEvent>

using Gui::ComboBox;
using Gui::WidgetTemplate;

ComboBox::ComboBox(QWidget* parent) :
	WidgetTemplate<QComboBox>(nullptr)
{
	Q_UNUSED(parent)

	this->setItemDelegate(new ComboBoxDelegate(this));
}

ComboBox::~ComboBox() {}

void ComboBox::changeEvent(QEvent* event)
{
	WidgetTemplate<QComboBox>::changeEvent(event);

	if(event->type() != QEvent::StyleChange){
		return;
	}

	QFontMetrics f(this->font());
	int h = f.height();
	h = std::max(h, 16);

	this->setIconSize(QSize(h, h));
	update();
	repaint();
}

