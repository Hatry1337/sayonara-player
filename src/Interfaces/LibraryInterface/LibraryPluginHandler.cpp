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
#include "GUI/Helper/GUI_Helper.h"
#include "GUI/Helper/Delegates/ComboBoxDelegate.h"
#include "GUI/Helper/SayonaraWidget/SayonaraWidgetTemplate.h"
#include "LibraryContainer/LibraryContainer.h"
#include "Components/Library/LibraryManager.h"
#include "Helper/Library/LibraryInfo.h"
#include "GUI/Library/LocalLibraryContainer.h"
#include "GUI/Library/EmptyLibraryContainer.h"
#include "LibraryPluginMenu.h"

#include "Helper/globals.h"
#include "Helper/Helper.h"
#include "Helper/Settings/Settings.h"
#include "Helper/Logger/Logger.h"

#include <QAction>
#include <QDir>
#include <QIcon>
#include <QPair>
#include <QLayout>
#include <QComboBox>
#include <QPluginLoader>
#include <QLabel>
#include <QPainter>
#include <QFrame>
#include <QHBoxLayout>
#include <QScreen>
#include <QGuiApplication>

struct LibraryPluginHandler::Private
{
	LibraryContainerInterface*			current_library;
	QList<LibraryContainerInterface*>	libraries;
	QList<QPair<QString, QIcon>>		library_entries;
	QWidget*							library_parent=nullptr;
	LibraryPluginHandler*				plugin_handler=nullptr;

	Private(LibraryPluginHandler* plugin_handler) :
		plugin_handler(plugin_handler)
	{

	}


	void insert_local_libraries()
	{
		QList<LibraryInfo> library_infos = LibraryManager::getInstance()->get_all_libraries();
		for(const LibraryInfo& library_info : library_infos)
		{
			if(library_info.id() < 0){
				continue;
			}

			sp_log(Log::Debug, plugin_handler) << "Add local library "
									 << library_info.name() << ": "
									 << library_info.path();

			libraries << new LocalLibraryContainer(library_info);
		}

		if(libraries.isEmpty()){
			libraries << new EmptyLibraryContainer();
		}
	}

	void insert_dll_libraries()
	{
		QDir plugin_dir = QDir(Helper::lib_path());
		QStringList dll_filenames = plugin_dir.entryList(QDir::Files);

		for(const QString& filename : dll_filenames)
		{
			QObject* raw_plugin;
			LibraryContainerInterface* container;

			QPluginLoader loader(plugin_dir.absoluteFilePath(filename));

			raw_plugin = loader.instance();
			if(!raw_plugin) {
				sp_log(Log::Warning, plugin_handler) << "Cannot load plugin: " << filename << ": " << loader.errorString();
				loader.unload();
				continue;
			}

			container = dynamic_cast<LibraryContainerInterface*>(raw_plugin);
			if(!container) {
				loader.unload();
				continue;
			}

			sp_log(Log::Info, plugin_handler) << "Found library plugin " << container->display_name();
			libraries << container;
		}
	}

	void insert_containers(const QList<LibraryContainerInterface*>& containers)
	{
		for(LibraryContainerInterface* container : containers)
		{
			if(!container){
				continue;
			}

			sp_log(Log::Debug, plugin_handler) << "Add plugin " << container->display_name();
			libraries << container;
		}
	}
};

bool LibraryPluginHandler::is_local_library(const LibraryContainerInterface* container)
{
	return ((dynamic_cast<const LocalLibraryContainer*>(container) != nullptr) ||
			(dynamic_cast<const EmptyLibraryContainer*>(container) != nullptr));
}

LibraryPluginHandler::LibraryPluginHandler() :
	QObject(nullptr),
	SayonaraClass()
{
	_m = Pimpl::make<Private>(this);
}

LibraryPluginHandler::~LibraryPluginHandler() {}

void LibraryPluginHandler::init(const QList<LibraryContainerInterface*>& containers)
{
	QString cur_plugin = _settings->get(Set::Lib_CurPlugin);

	_m->insert_local_libraries();
	_m->insert_containers(containers);
	_m->insert_dll_libraries();

	for(LibraryContainerInterface* container : _m->libraries){
		if(container->name().compare(cur_plugin) == 0){
			_m->current_library = container;
			break;
		}
	}

	emit sig_initialized();
}


void LibraryPluginHandler::init_library(LibraryContainerInterface* library)
{
	if(library->is_initialized()){
		return;
	}

	library->init_ui();
	library->set_initialized();

	QWidget* ui = library->widget();
	ui->setParent(_m->library_parent);

	QLayout* layout = ui->layout();
	if(layout){
		layout->setContentsMargins(5, 0, 8, 0);
	}

	QFrame* header_frame = library->header();
	if(header_frame)
	{
		QPixmap pixmap = library->icon().scaled(24, 24, Qt::AspectRatioMode::KeepAspectRatio, Qt::TransformationMode::SmoothTransformation);
		QLabel* lab_text = new QLabel(library->display_name(), header_frame);
		QLabel* lab_image = new QLabel(header_frame);
		QHBoxLayout* layout = new QHBoxLayout(header_frame);

		lab_text->setStyleSheet("font-weight: bold;");
		lab_image->setPixmap(pixmap);

		layout->setContentsMargins(0, 0, 0, 0);
		layout->setSpacing(8);
		layout->addWidget(lab_image);
		layout->addWidget(lab_text);

		header_frame->setLayout(layout);
		header_frame->setFrameStyle(QFrame::NoFrame);
		header_frame->setContextMenuPolicy(Qt::ContextMenuPolicy::CustomContextMenu);

		LibraryPluginMenu* context_menu = new LibraryPluginMenu(header_frame);
		context_menu->setup_actions();
		connect(header_frame, &QWidget::customContextMenuRequested, this, [=](const QPoint& pos){
			QPoint new_pos = header_frame->mapToGlobal(pos);
			int y_difference = GUI::move_widget(new_pos, context_menu);
			new_pos.setY(new_pos.y() - y_difference);
			context_menu->popup(new_pos);
			context_menu->move(context_menu->pos().x(),
							   context_menu->pos().y()-y_difference);
		});
	}
}


void LibraryPluginHandler::set_library_parent(QWidget* parent)
{
	_m->library_parent = parent;

	for(LibraryContainerInterface* container : _m->libraries){
		if(container->is_initialized()){
			container->widget()->setParent(parent);
		}
	}
}

void LibraryPluginHandler::set_current_library(const QString& name)
{
	for(LibraryContainerInterface* container : _m->libraries)
	{
		if(container->name().compare(name) != 0) {
			if(container->is_initialized()){
				container->widget()->setVisible(false);
			}

			continue;
		}

		_m->current_library = container;
		init_library(container);

		QWidget* widget = container->widget();
		QWidget* parent = widget->parentWidget();

		widget->resize(parent->size());
		widget->setVisible(true);
		widget->update();
	}

	_settings->set(Set::Lib_CurPlugin, name);

	emit sig_current_library_changed(name);
}


void LibraryPluginHandler::add_local_library(const LibraryInfo& library)
{
	LocalLibraryContainer* llc = new LocalLibraryContainer(library);

	int idx = 0;
	for(LibraryContainerInterface* container : _m->libraries)
	{
		if(is_local_library(container)){
			idx++;
		}

		if(!container->is_initialized()){
			continue;
		}
	}

	_m->libraries.insert(idx, llc);
}

void LibraryPluginHandler::rename_local_library(qint8 library_id, const QString& new_name)
{
	for(LibraryContainerInterface* container : _m->libraries)
	{
		LocalLibraryContainer* llc = dynamic_cast<LocalLibraryContainer*>(container);
		if(llc){
			if(llc->get_id() == library_id) {
				llc->set_name(new_name);
				break;
			}
		}
	}
}

void LibraryPluginHandler::remove_local_library(qint8 library_id)
{
	int idx = -1;
	int i=0;
	for(LibraryContainerInterface* container : _m->libraries)
	{
		LocalLibraryContainer* llc = dynamic_cast<LocalLibraryContainer*>(container);
		if(llc){
			if(llc->get_id() == library_id){
				idx = i;
				break;
			}

			i++;
		}
	}

	if(idx < 0) {
		return;
	}

	_m->libraries.removeAt(idx);
}

void LibraryPluginHandler::move_local_library(int old_row, int new_row)
{
	_m->libraries.move(old_row, new_row);
}

LibraryContainerInterface*LibraryPluginHandler::current_library() const
{
	return _m->current_library;
}

QMenu* LibraryPluginHandler::current_library_menu() const
{
	if(!_m->current_library){
		return nullptr;
	}

	return _m->current_library->menu();
}


QList<LibraryContainerInterface*> LibraryPluginHandler::get_libraries() const
{
	return _m->libraries;
}

