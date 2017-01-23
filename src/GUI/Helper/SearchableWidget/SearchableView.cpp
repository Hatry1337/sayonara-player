/* SearchableView.cpp */

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

#include "SearchableView.h"
#include "AbstractSearchModel.h"
#include "MiniSearcher.h"
#include "Helper/Settings/Settings.h"
#include "Helper/Set.h"

#include <QAbstractItemModel>
#include <QItemSelectionModel>
#include <QKeyEvent>
#include <QMap>
#include <QString>

struct SearchViewFunctionality::Private :
		public QObject
{
	Q_OBJECT

public:
	Settings*					settings=nullptr;
	SearchModelFunctionality*		search_model=nullptr;
	SearchViewFunctionality*	search_view=nullptr;
	QAbstractItemView*			view=nullptr;
	MiniSearcher*				mini_searcher=nullptr;
	int							cur_idx;

private slots:
	void edit_changed(const QString& str);
	void fwd_clicked();
	void bwd_clicked();

public:
	Private(SearchViewFunctionality* parent, QAbstractItemView* v) :
		QObject(v)
	{
		view = v;
		search_view = parent;
		search_model = nullptr;
		cur_idx = -1;
		mini_searcher = new MiniSearcher(v, MiniSearcherButtons::BothButtons);
		settings = Settings::getInstance();

		connect(mini_searcher, &MiniSearcher::sig_text_changed, this, &Private::edit_changed);
		connect(mini_searcher, &MiniSearcher::sig_find_next_row, this, &Private::fwd_clicked);
		connect(mini_searcher, &MiniSearcher::sig_find_prev_row, this, &Private::bwd_clicked);
	}
};


SearchViewFunctionality::SearchViewFunctionality(QAbstractItemView* view) :
	SayonaraSelectionView()
{
	_m = Pimpl::make<Private>(this, view);
}


SearchViewFunctionality::~SearchViewFunctionality() {}


void SearchViewFunctionality::setSearchModel(SearchModelFunctionality* model)
{
	 _m->search_model = model;

	 if(_m->search_model){
		 Library::SearchModeMask search_mode = _m->settings->get(Set::Lib_SearchMode);
		 _m->search_model->set_search_mode(search_mode);
	 }

	 _m->mini_searcher->set_extra_triggers(_m->search_model->getExtraTriggers());
}


int SearchViewFunctionality::get_row_count(const QModelIndex& parent) const
{
	return _m->view->model()->rowCount(parent);
}


int SearchViewFunctionality::get_column_count(const QModelIndex& parent) const
{
	return _m->view->model()->columnCount(parent);
}


QModelIndex SearchViewFunctionality::get_index(int row, int col, const QModelIndex& parent) const
{
	return _m->view->model()->index(row, col, parent);
}


QModelIndex SearchViewFunctionality::get_match_index(const QString& str, SearchDirection direction) const
{
	QModelIndex idx;
	if(str.isEmpty()) {
		return idx;
	}

	if(!_m->search_model) {
		return idx;
	}

	Library::SearchModeMask search_mode = _m->settings->get(Set::Lib_SearchMode);
	QMap<QChar, QString> extra_triggers = _m->search_model->getExtraTriggers();

	QString converted_string = Library::convert_search_string(str, search_mode, extra_triggers.keys());

	switch(direction)
	{
		case SearchDirection::First:
			idx = _m->search_model->getFirstRowIndexOf(converted_string);
			break;
		case SearchDirection::Next:
			idx = _m->search_model->getNextRowIndexOf(converted_string, _m->cur_idx + 1);
			break;
		case SearchDirection::Prev:
			idx = _m->search_model->getPrevRowIndexOf(converted_string, _m->cur_idx - 1);
			break;
	}

	return idx;
}


void SearchViewFunctionality::select_match(const QString &str, SearchDirection direction)
{
	QModelIndex idx = get_match_index(str, direction);
	if(!idx.isValid()){
		_m->cur_idx = -1;
		return;
	}

	_m->cur_idx = get_index_by_model_index(idx);

	SP::Set<int> indexes(_m->cur_idx);

	switch(selection_type()){
		case SayonaraSelectionView::SelectionType::Rows:
			select_rows(indexes);
			break;
		case SayonaraSelectionView::SelectionType::Columns:
			select_columns(indexes);
			break;
		case SayonaraSelectionView::SelectionType::Items:
			select_items(indexes);
			break;
	}

	this->set_current_index(_m->cur_idx);

	_m->view->scrollTo(idx);
}


QItemSelectionModel* SearchViewFunctionality::get_selection_model() const
{
	return _m->view->selectionModel();
}


void SearchViewFunctionality::set_current_index(int idx)
{
	QModelIndex index = get_model_index_by_index(idx);
	_m->view->setCurrentIndex(index);
}


void SearchViewFunctionality::handleKeyPress(QKeyEvent* e)
{
	if(!_m->search_model)
	{
		return;
	}

	Library::SearchModeMask search_mode = _m->settings->get(Set::Lib_SearchMode);
	_m->search_model->set_search_mode(search_mode);


	bool was_initialized = _m->mini_searcher->isVisible();
	bool initialized = _m->mini_searcher->check_and_init(e);

	if(e->key() == Qt::Key_Tab && !was_initialized) {
		return;
	}

	if(initialized || was_initialized) {
		_m->mini_searcher->keyPressEvent(e);
		e->setAccepted(false);
		return;
	}

	e->setAccepted(true);
}


void SearchViewFunctionality::Private::edit_changed(const QString& str)
{
	search_view->select_match(str, SearchDirection::First);
}

void SearchViewFunctionality::Private::fwd_clicked()
{
	QString str = mini_searcher->get_current_text();
	search_view->select_match(str, SearchDirection::Next);
}

void SearchViewFunctionality::Private::bwd_clicked()
{
	QString str = this->mini_searcher->get_current_text();
	search_view->select_match(str, SearchDirection::Prev);
}


#include "GUI/Helper/GUI/Helper/SearchableWidget/SearchableView.moc"
