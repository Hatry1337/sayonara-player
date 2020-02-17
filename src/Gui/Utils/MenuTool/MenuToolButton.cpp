/* MenuTool.cpp */

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

#include "MenuToolButton.h"
#include "Gui/Utils/PreferenceAction.h"

using Gui::MenuToolButton;

struct MenuToolButton::Private
{
	ContextMenu* menu=nullptr;

	Private(MenuToolButton* parent) :
		menu(new ContextMenu(parent))
	{}
};


MenuToolButton::MenuToolButton(QWidget* parent) :
	MenuButton(parent)
{
	m = Pimpl::make<Private>(this);

	connect(m->menu, &ContextMenu::sigOpen, this,  &MenuToolButton::sigOpen);
	connect(m->menu, &ContextMenu::sigNew, this, &MenuToolButton::sigNew);
	connect(m->menu, &ContextMenu::sigUndo, this, &MenuToolButton::sigUndo);
	connect(m->menu, &ContextMenu::sigDefault, this, &MenuToolButton::sigDefault);
	connect(m->menu, &ContextMenu::sigSave, this, &MenuToolButton::sigSave);
	connect(m->menu, &ContextMenu::sigSaveAs, this, &MenuToolButton::sigSaveAs);
	connect(m->menu, &ContextMenu::sigRename, this, &MenuToolButton::sigRename);
	connect(m->menu, &ContextMenu::sigDelete, this, &MenuToolButton::sigDelete);
	connect(m->menu, &ContextMenu::sigEdit, this, &MenuToolButton::sigEdit);

	proveEnabled();
}

MenuToolButton::~MenuToolButton() = default;

void MenuToolButton::registerAction(QAction *action)
{
	m->menu->registerAction(action);
	proveEnabled();
}

void MenuToolButton::registerPreferenceAction(PreferenceAction* action)
{
	m->menu->addPreferenceAction(action);
}

void MenuToolButton::showMenu(QPoint pos)
{
	MenuButton::showMenu(pos);

	this->setDisabled(true);
	m->menu->popup(pos);
	this->setEnabled(true);
}


bool MenuToolButton::proveEnabled()
{
	bool enabled = m->menu->hasActions();
	this->setEnabled(enabled);
	return enabled;
}

void MenuToolButton::showAll()
{
	m->menu->showAll();
	proveEnabled();
}

void MenuToolButton::showAction(ContextMenu::Entry entry, bool visible)
{
	m->menu->showAction(entry, visible);
	proveEnabled();
}

void MenuToolButton::showActions(ContextMenuEntries entries)
{
	m->menu->showActions(entries);
	proveEnabled();
}

Gui::ContextMenuEntries MenuToolButton::entries() const
{
	return m->menu->entries();
}
