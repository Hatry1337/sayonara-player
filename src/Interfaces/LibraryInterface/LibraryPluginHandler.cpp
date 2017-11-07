/* LibraryPluginHandler.cpp */

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

#include "LibraryPluginHandler.h"
#include "LibraryPluginCombobox.h"

#include "GUI/Utils/GuiUtils.h"
#include "GUI/Utils/Delegates/ComboBoxDelegate.h"
#include "GUI/Utils/Widgets/WidgetTemplate.h"
#include "LibraryContainer/LibraryContainer.h"
#include "Components/Library/LibraryManager.h"
#include "Utils/Library/LibraryInfo.h"
#include "GUI/Library/LocalLibraryContainer.h"
#include "GUI/Library/EmptyLibraryContainer.h"

#include "Utils/globals.h"
#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include <QAction>
#include <QDir>
#include <QFrame>
#include <QIcon>
#include <QLayout>
#include <QMenu>
#include <QPluginLoader>
#include <QSize>
#include <QVBoxLayout>

using Library::PluginHandler;
using Library::Container;

struct PluginHandler::Private
{
	Container*			current_library=nullptr;

	Container*			empty_library=nullptr;
	QList<LocalLibraryContainer*>		local_libraries;
	QList<Container*>	library_containers;
	QList<Container*>	dll_libraries;

	QWidget*							library_parent=nullptr;
	PluginHandler*				plugin_handler=nullptr;

	Private(PluginHandler* plugin_handler) :
		plugin_handler(plugin_handler)
	{}

	void insert_local_libraries()
	{
		QList<Library::Info> library_infos = Library::Manager::instance()->all_libraries();
		for(const Library::Info& library_info : library_infos)
		{
			if(library_info.id() < 0) {
				continue;
			}

			sp_log(Log::Debug, plugin_handler) << "Add local library "
											   << library_info.name() << ": "
											   << library_info.path();

			local_libraries << new LocalLibraryContainer(library_info);
		}

		if(local_libraries.isEmpty()) {
			empty_library = new EmptyLibraryContainer();
		}
	}

	void insert_dll_libraries()
	{
		QDir plugin_dir = QDir(Util::lib_path());
		QStringList dll_filenames = plugin_dir.entryList(QDir::Files);

		for(const QString& filename : dll_filenames)
		{
			QObject* raw_plugin;
			Container* container;

			QPluginLoader loader(plugin_dir.absoluteFilePath(filename));

			raw_plugin = loader.instance();
			if(!raw_plugin) {
				sp_log(Log::Warning, plugin_handler) << "Cannot load plugin: " << filename << ": " << loader.errorString();
				loader.unload();
				continue;
			}

			container = dynamic_cast<Container*>(raw_plugin);
			if(!container) {
				loader.unload();
				continue;
			}

			sp_log(Log::Info, plugin_handler) << "Found library plugin " << container->display_name();
			dll_libraries << container;
		}
	}

	void insert_containers(const QList<Container*>& containers)
	{
		for(Container* container : containers)
		{
			if(!container) {
				continue;
			}

			sp_log(Log::Debug, plugin_handler) << "Add plugin " << container->display_name();
			library_containers << container;
		}
	}

	QList<Container*> all_libraries() const
	{
		QList<Container*> container;
		if(empty_library) {
			container << empty_library;
		}

		for(LocalLibraryContainer* llc : local_libraries) {
			container << static_cast<Container*>(llc);
		}

		container << library_containers;
		container << dll_libraries;

		return container;
	}

	Container* library_by_name(const QString& name)
	{
		for(Container* container : all_libraries())
		{
			if(container->name() == name) {
				return container;
			}
		}

		return nullptr;
	}
};


/*************************************************************************/


PluginHandler::PluginHandler() :
	QObject(nullptr),
	SayonaraClass()
{
	m = Pimpl::make<Private>(this);
}

PluginHandler::~PluginHandler() {}


void PluginHandler::init(const QList<Container*>& containers)
{
	QString cur_plugin = _settings->get(Set::Lib_CurPlugin);

	m->insert_local_libraries();
	m->insert_containers(containers);
	m->insert_dll_libraries();

	Container* container = m->library_by_name(cur_plugin);
	if(container) {
		set_current_library(container);
	}

	else {
		set_current_library( m->all_libraries().first() );
	}

	emit sig_initialized();
}


void PluginHandler::init_library(Container* library)
{
	if(library->is_initialized()) {
		return;
	}

	library->init_ui();
	library->set_initialized();

	QWidget* ui = library->widget();
	ui->setParent(m->library_parent);

	QLayout* layout = ui->layout();
	if(layout) {
		layout->setContentsMargins(5, 0, 8, 0);
	}

	QFrame* header_frame = library->header();
	if(header_frame)
	{
		PluginCombobox* combo_box = new PluginCombobox(library->display_name(), nullptr);

		QLayout* layout = new QVBoxLayout(header_frame);
		layout->setContentsMargins(0, 0, 0, 0);
		layout->addWidget(combo_box);

		header_frame->setFrameShape(QFrame::NoFrame);
		header_frame->setLayout(layout);

		connect(combo_box, SIGNAL(activated(int)),
				this, SLOT(current_library_changed(int)));
	}
}


void PluginHandler::set_library_parent(QWidget* parent)
{
	m->library_parent = parent;

	for(Container* container : m->all_libraries()) {
		if(container->is_initialized()) {
			container->widget()->setParent(parent);
		}
	}
}


void PluginHandler::current_library_changed(int library_idx)
{
	QList<Container*> libs = m->all_libraries();
	if(between(library_idx, libs)) {
		set_current_library(libs[library_idx]);
	}
}


void PluginHandler::set_current_library(const QString& name)
{
	set_current_library( m->library_by_name(name) );
}


void PluginHandler::set_current_library(Container* cur_library)
{
	if(!cur_library) {
		return;
	}

	for(Container* container : m->all_libraries())
	{
		if(container->name() != cur_library->name()) {
			container->hide();
		}

		else {
			m->current_library = container;
		}
	}

	if(m->current_library) {
	   init_library(m->current_library);
	   m->current_library->show();
	}

	_settings->set(Set::Lib_CurPlugin, cur_library->name() );

	emit sig_current_library_changed( cur_library->name() );
}


void PluginHandler::add_local_library(const Library::Info& library)
{
	LocalLibraryContainer* llc = new LocalLibraryContainer(library);
	m->local_libraries << llc;

	bool empty_library_found = (m->empty_library != nullptr);

	if(empty_library_found)
	{
		m->empty_library->hide();
		m->empty_library->deleteLater();
		m->empty_library = nullptr;
	}

	emit sig_libraries_changed();

	if(empty_library_found) {
		set_current_library(llc);
	}
}


void PluginHandler::rename_local_library(int8_t library_id, const QString& new_name)
{
	for(LocalLibraryContainer* llc : m->local_libraries)
	{
		if(llc->id() == library_id) {
			llc->set_name(new_name);
			set_current_library(llc);

			break;
		}
	}

	emit sig_libraries_changed();
}


void PluginHandler::remove_local_library(int8_t library_id)
{
	int idx = -1;
	int i=0;

	LocalLibraryContainer* removed_llc=nullptr;
	for(LocalLibraryContainer* llc : m->local_libraries)
	{
		if(llc->id() == library_id)
		{
			idx = i;
			removed_llc = llc;

			llc->hide();

			break;
		}

		i++;
	}

	if(idx < 0) {
		return;
	}

	m->local_libraries.removeAt(idx);

	if(m->local_libraries.isEmpty()) {
		m->empty_library = new EmptyLibraryContainer();
	}

	emit sig_libraries_changed();

	if(m->current_library == removed_llc)
	{
		if(m->local_libraries.isEmpty()) {
			set_current_library(m->empty_library);
		}

		else {
			set_current_library(m->local_libraries.first());
		}
	}


	emit sig_libraries_changed();
}


void PluginHandler::move_local_library(int old_row, int new_row)
{
	m->local_libraries.move(old_row, new_row);

	emit sig_libraries_changed();
}


Container* PluginHandler::current_library() const
{
	return m->current_library;
}


QMenu* PluginHandler::current_library_menu() const
{
	if(!m->current_library) {
		return nullptr;
	}

	return m->current_library->menu();
}


QList<Container*> PluginHandler::get_libraries() const
{
	return m->all_libraries();
}

