/* DirectoryTreeView.cpp */

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

#include "DirectoryTreeView.h"
#include "DirectoryDelegate.h"
#include "DirectoryIconProvider.h"
#include "DirectoryModel.h"

#include "Components/DirectoryReader/DirectoryReader.h"

#include "GUI/Utils/PreferenceAction.h"
#include "GUI/Utils/ContextMenu/LibraryContextMenu.h"
#include "GUI/Utils/CustomMimeData.h"
#include "GUI/Utils/MimeDataUtils.h"
#include "GUI/Utils/InputDialog/LineInputDialog.h"
#include "GUI/Utils/Icons.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Library/LibraryInfo.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Language.h"

#include <QDir>
#include <QMouseEvent>
#include <QDrag>
#include <QTimer>
#include <QAction>


struct DirectoryTreeView::Private
{
	QString				last_search_term;

	LibraryContextMenu*	context_menu=nullptr;
	DirectoryModel*		model = nullptr;
	IconProvider*		icon_provider = nullptr;
	QModelIndex			drag_move_index;
	QTimer*				drag_move_timer=nullptr;
	QAction*			action_create_dir=nullptr;
	QAction*			action_rename_dir=nullptr;
	QAction*			action_collapse_all=nullptr;


	Private(QObject* parent)
	{
		icon_provider = new IconProvider();
		drag_move_timer = new QTimer(parent);
		drag_move_timer->setInterval(750);
		drag_move_timer->setSingleShot(true);
	}

	~Private()
	{
		delete icon_provider; icon_provider = nullptr;
	}
};


DirectoryTreeView::DirectoryTreeView(QWidget *parent) :
	Gui::WidgetTemplate<SearchableTreeView>(parent),
	Dragable(this)
{
	m = Pimpl::make<Private>(this);

	QString root_path = Util::sayonara_path("Libraries");

	m->model = new DirectoryModel(this);
	m->model->setFilter(QDir::NoDotAndDotDot | QDir::Dirs);
	m->model->setIconProvider(m->icon_provider);
	m->model->setRootPath(root_path);

	this->setModel(m->model);
	this->setSearchModel(m->model);
	this->setItemDelegate(new DirectoryDelegate(this));

	for(int i=1; i<4; i++){
		this->hideColumn(i);
	}

	this->setRootIndex(m->model->index(root_path));
	//connect(m->model, &DirectoryModel::directoryLoaded, this, &DirectoryTreeView::directory_loaded);
	connect(m->drag_move_timer, &QTimer::timeout, this, &DirectoryTreeView::drag_move_timer_finished);
}

DirectoryTreeView::~DirectoryTreeView() {}

LibraryId DirectoryTreeView::library_id(const QModelIndex& index) const
{
	return m->model->library_id(index);
}


void DirectoryTreeView::language_changed()
{
	if(m && m->action_rename_dir){
		m->action_rename_dir->setText(Lang::get(Lang::Rename));
		m->action_create_dir->setText(tr("Create directory"));
		m->action_collapse_all->setText(tr("Collapse all"));
	}
}


void DirectoryTreeView::skin_changed()
{
	using namespace Gui;

	if(!m){
		return;
	}

	if(m->model){
		m->model->setIconProvider(m->icon_provider);
	}

	if(m->action_rename_dir){
		m->action_rename_dir->setIcon(Icons::icon(Icons::Rename));
		m->action_create_dir->setIcon(Icons::icon(Icons::New));
	}

}

void DirectoryTreeView::keyPressEvent(QKeyEvent* event)
{
	event->setAccepted(false);

	switch(event->key())
	{
		case Qt::Key_Enter:
		case Qt::Key_Return:
			emit sig_enter_pressed();

			return;

		default: break;
	}

	m->model->search_only_dirs(true);

	SearchableTreeView::keyPressEvent(event);
}

void DirectoryTreeView::init_context_menu()
{
	if(m->context_menu){
		return;
	}

	m->context_menu = new LibraryContextMenu(this);

	m->action_create_dir = new QAction(m->context_menu);
	m->action_rename_dir = new QAction(m->context_menu);
	m->action_collapse_all = new QAction(m->context_menu);

	LibraryContexMenuEntries entries =
			(LibraryContextMenu::EntryDelete |
			LibraryContextMenu::EntryInfo |
			LibraryContextMenu::EntryEdit |
			LibraryContextMenu::EntryAppend |
			LibraryContextMenu::EntryPlayNext);

	m->context_menu->show_actions(entries);
	QAction* separator = m->context_menu->addSeparator();

	QAction* action	= m->context_menu->get_action(LibraryContextMenu::EntryDelete);
	if(action){
		m->context_menu->insertActions(
			action,
			{separator, m->action_create_dir, m->action_rename_dir}
		);
	}

	else{
		m->context_menu->addActions(
			{separator, m->action_create_dir, m->action_rename_dir}
		);
	}

	connect(m->context_menu, &LibraryContextMenu::sig_info_clicked, this, &DirectoryTreeView::sig_info_clicked);
	connect(m->context_menu, &LibraryContextMenu::sig_lyrics_clicked, this, &DirectoryTreeView::sig_info_clicked);
	connect(m->context_menu, &LibraryContextMenu::sig_edit_clicked, this, &DirectoryTreeView::sig_info_clicked);
	connect(m->context_menu, &LibraryContextMenu::sig_delete_clicked, this, &DirectoryTreeView::sig_delete_clicked);
	connect(m->context_menu, &LibraryContextMenu::sig_play_next_clicked, this, &DirectoryTreeView::sig_play_next_clicked);
	connect(m->context_menu, &LibraryContextMenu::sig_append_clicked, this, &DirectoryTreeView::sig_append_clicked);
	connect(m->action_create_dir, &QAction::triggered, this, &DirectoryTreeView::create_dir_clicked);
	connect(m->action_rename_dir, &QAction::triggered, this, &DirectoryTreeView::rename_dir_clicked);
	connect(m->action_collapse_all, &QAction::triggered, this, &DirectoryTreeView::collapseAll);

	action = m->context_menu->add_preference_action(new LibraryPreferenceAction(m->context_menu));

	separator = m->context_menu->addSeparator();
	m->context_menu->insertActions(
		action,
		{m->action_collapse_all, separator}
	);

	skin_changed();
	language_changed();
}

QString DirectoryTreeView::directory_name(const QModelIndex &index)
{
	return m->model->filePath(index);
}

QString DirectoryTreeView::directory_name_origin(const QModelIndex& index)
{
	return m->model->filepath_origin(index);
}

QModelIndexList DirectoryTreeView::selected_items() const
{
	QItemSelectionModel* selection_model = this->selectionModel();

	return selection_model->selectedRows();
}


MetaDataList DirectoryTreeView::selected_metadata() const
{
	DirectoryReader reader;
	QStringList paths = selected_paths();
	return reader.metadata_from_filelist(paths);
}


QStringList DirectoryTreeView::selected_paths() const
{
	QModelIndexList selections = this->selected_items();
	if(selections.isEmpty()){
		return QStringList();
	}

	QStringList paths;
	for(const QModelIndex& idx : selections)
	{
		paths << m->model->filepath_origin(idx);
	}

	return paths;
}


QModelIndex DirectoryTreeView::search(const QString& search_term)
{
	QModelIndex found_idx;

	m->model->search_only_dirs(false);

	if(m->last_search_term == search_term) {
		found_idx = m->model->getNextRowIndexOf(m->last_search_term, 0, QModelIndex());
	}

	else {
		found_idx = m->model->getFirstRowIndexOf(search_term);
		m->last_search_term = search_term;
	}

	scrollTo(found_idx, QAbstractItemView::PositionAtCenter);
	selectionModel()->select(found_idx, QItemSelectionModel::ClearAndSelect);
	expand(found_idx);

	emit sig_directory_loaded(found_idx);

	return found_idx;
}

void DirectoryTreeView::select_match(const QString& str, SearchDirection direction)
{
	QModelIndex idx;

	if(direction == SearchDirection::First){
		idx = m->model->getFirstRowIndexOf(str);
	}

	else if(direction == SearchDirection::Next){
		idx = m->model->getNextRowIndexOf(str, 0);
	}

	else {
		idx = m->model->getPrevRowIndexOf(str, 0);
	}

	if(!idx.isValid()){
		return;
	}

	expand(idx);
	scrollTo(idx, QAbstractItemView::PositionAtCenter);
	this->clearSelection();
	selectionModel()->clearSelection();
	selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);
	setCurrentIndex(idx);

	if(m->model->canFetchMore(idx)){
		m->model->fetchMore(idx);
	}
}

bool DirectoryTreeView::has_drag_label() const
{
	return (!this->selected_paths().isEmpty());
}

QString DirectoryTreeView::drag_label() const
{
	QStringList paths = selected_paths();
	for(QString& path : paths)
	{
		QString d, f;
		Util::File::split_filename(path, d, f);
		path = f;
	}
	return paths.join(", ");
}

void DirectoryTreeView::mousePressEvent(QMouseEvent* event)
{
	QTreeView::mousePressEvent(event);

	if(event->buttons() & Qt::LeftButton)
	{
		Dragable::drag_pressed( event->pos() );
	}

	if(event->button() & Qt::RightButton){
		QPoint pos = QWidget::mapToGlobal( event->pos() );

		if(!m->context_menu){
			init_context_menu();
		}

		m->context_menu->exec(pos);
	}
}


void DirectoryTreeView::mouseMoveEvent(QMouseEvent* e)
{
	QDrag* drag = Dragable::drag_moving(e->pos());
	if(drag)
	{
		connect(drag, &QDrag::destroyed, [=]() {
			this->drag_released(Dragable::ReleaseReason::Destroyed);
		});
	}
}

void DirectoryTreeView::dragEnterEvent(QDragEnterEvent* event)
{
	const QMimeData* mime_data = event->mimeData();
	if(!mime_data->hasUrls()){
		sp_log(Log::Warning, this) << "DragMove: No Mimedata";
		return;
	}

	event->accept();
}

void DirectoryTreeView::drag_move_timer_finished()
{
	if(m->drag_move_index.isValid())
	{
		this->expand(m->drag_move_index);

		m->drag_move_timer->stop();
		m->drag_move_index = QModelIndex();
	}
}

void DirectoryTreeView::create_dir_clicked()
{
	QModelIndexList indexes = this->selected_items();
	if(indexes.size() != 1){
		return;
	}

	QString dir = m->model->filepath_origin(indexes.first());
	LineInputDialog dialog(Lang::get(Lang::Rename), tr("Enter new name"), this);
	dialog.exec();

	QString new_dir_name = dialog.textValue();

	if(!new_dir_name.isEmpty() && !new_dir_name.contains("/") && !new_dir_name.contains("\\"))
	{
		Util::File::create_dir(QDir(dir).filePath(new_dir_name));
		this->expand(indexes.first());
	}
}

void DirectoryTreeView::rename_dir_clicked()
{
	QModelIndexList indexes = this->selected_items();
	if(indexes.size() != 1){
		return;
	}

	QString dir = m->model->filepath_origin(indexes.first());
	QDir d(dir);
	LineInputDialog dialog(Lang::get(Lang::Rename), tr("Enter new name"), d.dirName(), this);
	dialog.exec();

	QString dir_renamed = dialog.textValue();

	if(!dir_renamed.isEmpty())
	{
		d.cdUp();
		m->model->rename_dir(dir, d.filePath(dir_renamed));
	}
}

void DirectoryTreeView::dragMoveEvent(QDragMoveEvent* event)
{
	event->ignore();

	const QMimeData* mime_data = event->mimeData();
	if(!mime_data){
		return;
	}

	const CustomMimeData* cmd = Gui::MimeData::custom_mimedata(mime_data);
	if(cmd)
	{
		if(cmd->has_source(this)){
			event->accept();
		}
	}

	else {
		if(mime_data->hasUrls()){
			event->accept();
		}
	}

	if(!event->isAccepted()){
		m->drag_move_timer->stop();
		return;
	}

	QModelIndex index = this->indexAt(event->pos());
	selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);

	if(m->drag_move_index != index && index.isValid())
	{
		m->drag_move_timer->start();
	}

	m->drag_move_index = index;

	if(!index.isValid()){
		m->drag_move_timer->stop();
		sp_log(Log::Debug, this) << "Stop timer";
	}
}

void DirectoryTreeView::dropEvent(QDropEvent* event)
{
	event->accept();

	QModelIndex index = this->indexAt(event->pos());
	if(!index.isValid()){
		sp_log(Log::Warning, this) << "Drop: Invalid index";
		return;
	}

	const QMimeData* mime_data = event->mimeData();
	if(!mime_data){
		sp_log(Log::Warning, this) << "Drop: No Mimedata";
		return;
	}

	QString target_dir = m->model->filepath_origin(index);
	const CustomMimeData* cmd = Gui::MimeData::custom_mimedata(mime_data);
	if(cmd)
	{
		if(cmd->has_source(this) && mime_data->hasUrls())
		{
			QList<QUrl> urls = mime_data->urls();
			QStringList source_dirs;
			for(const QUrl& url : urls)
			{
				QString source_dir = url.toLocalFile();
				if(!Util::File::can_copy_dir(source_dir, target_dir)){
					continue;
				}

				source_dirs << url.toLocalFile();
			}

			if(source_dirs.isEmpty()){
				return;
			}

			DirectoryTreeView::DropAction drop_action = show_drop_menu(QCursor::pos());
			switch(drop_action)
			{
				case DirectoryTreeView::DropAction::Copy:
					m->model->copy_dirs(source_dirs, target_dir);
					break;
				case DirectoryTreeView::DropAction::Move:
					m->model->move_dirs(source_dirs, target_dir);
					break;
				default:
					break;
			}
		}

		else
		{
			sp_log(Log::Debug, this) << "Drop: Internal player drag";
		}

		return;
	}

	if(!mime_data->hasUrls())
	{
		sp_log(Log::Debug, this) << "Drop: No Urls";
		return;
	}

	LibraryId lib_id = m->model->library_id(index);
	QStringList files;

	for(const QUrl& url : mime_data->urls())
	{
		QString local_file = url.toLocalFile();
		if(!local_file.isEmpty()){
			files << local_file;
		}
	}

	sp_log(Log::Debug, this) << "Drop: " << files.size() << " files into library " << lib_id;

	if(lib_id < 0){
		return;
	}

	emit sig_import_requested(lib_id, files, target_dir);
}

DirectoryTreeView::DropAction DirectoryTreeView::show_drop_menu(const QPoint& pos)
{
	QMenu* menu = new QMenu(this);

	QList<QAction*> actions;
	actions << new QAction(tr("Copy here"), menu);
	actions << new QAction(tr("Move here"), menu);
	actions << menu->addSeparator();
	actions << new QAction(Lang::get(Lang::Cancel), menu);
	menu->addActions(actions);

	QAction* action = menu->exec(pos);
	DirectoryTreeView::DropAction drop_action = DirectoryTreeView::DropAction::Cancel;

	if(action == actions[0]){
		drop_action = DirectoryTreeView::DropAction::Copy;
	}

	else if(action == actions[1]){
		drop_action = DirectoryTreeView::DropAction::Move;
	}

	menu->deleteLater();

	return drop_action;
}

QMimeData* DirectoryTreeView::dragable_mimedata() const
{
	CustomMimeData* cmd	= new CustomMimeData(this);
	MetaDataList v_md = this->selected_metadata();

	cmd->set_metadata(v_md);

	QList<QUrl> urls;
	QModelIndexList selected_items = this->selected_items();
	for(const QModelIndex& index : selected_items){
		urls << QUrl::fromLocalFile(m->model->filepath_origin(index));
		sp_log(Log::Debug, this) << "Dragging " << m->model->filepath_origin(index);
	}

	cmd->setUrls(urls);

	return cmd;
}

int DirectoryTreeView::index_by_model_index(const QModelIndex& idx) const
{
	Q_UNUSED(idx)
	return -1;
}

QModelIndex DirectoryTreeView::model_index_by_index(int idx) const
{
	Q_UNUSED(idx)
	return QModelIndex();
}
