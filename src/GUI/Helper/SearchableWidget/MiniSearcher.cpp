/* MiniSearcher.cpp */

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

#include "MiniSearcher.h"

#include "SearchableTableView.h"
#include "SearchableListView.h"

#include "GUI/Helper/GUI_Helper.h"

#include <QScrollBar>
#include <QShortcut>
#include <QKeySequence>

MiniSearcherLineEdit::MiniSearcherLineEdit(QWidget* parent) :
	QLineEdit(parent) {}

MiniSearcherLineEdit::~MiniSearcherLineEdit() {}


void MiniSearcherLineEdit::focusOutEvent(QFocusEvent* e)
{
	emit sig_le_focus_lost();
	QLineEdit::focusOutEvent(e);
}


MiniSearcher::MiniSearcher(QAbstractItemView* parent, MiniSearcherButtons b) :
	QFrame(parent)
{
    _parent = parent;

	init_layout(b);
}


void MiniSearcher::init_layout(MiniSearcherButtons b)
{
    bool left=false;
    bool right=false;

	_line_edit = new MiniSearcherLineEdit(this);
	_line_edit->setMaximumWidth(100);

	connect(_line_edit, &MiniSearcherLineEdit::textChanged, this, &MiniSearcher::line_edit_text_changed);
	connect(_line_edit, &MiniSearcherLineEdit::sig_tab_pressed, this, &MiniSearcher::right_clicked);
	connect(_line_edit, &MiniSearcherLineEdit::sig_le_focus_lost, this, &MiniSearcher::line_edit_focus_lost);
	connect(_line_edit, &MiniSearcherLineEdit::sig_esc_pressed, this, &MiniSearcher::close);

	_layout = new QBoxLayout(QBoxLayout::LeftToRight, this);
	_layout->setContentsMargins(4, 4, 4, 4);
	_layout->setSpacing(3);
	_layout->addWidget(_line_edit);

	switch(b)
	{
		case MiniSearcherButtons::BothButtons:
			left = true; right = true;
			break;

		case MiniSearcherButtons::BwdButton:
			left = true;
			break;

		case MiniSearcherButtons::FwdButton:
			right = true;

		case MiniSearcherButtons::NoButton:

		default:
		break;
	}

	if(left) {
		_left_button = new QPushButton(this);
		_left_button->setIcon(GUI::get_icon("bwd"));
		_left_button->setVisible(true);
		_left_button->setFlat(true);
		_left_button->setFocusPolicy(Qt::ClickFocus);

		connect(_left_button, &QPushButton::clicked, this, &MiniSearcher::left_clicked);

		_layout->addWidget(_left_button);
	}

	if(right) {
		_right_button = new QPushButton(this);
		_right_button->setIcon(GUI::get_icon("fwd"));
		_right_button->setVisible(true);
		_right_button->setFlat(true);
		_right_button->setFocusPolicy(Qt::ClickFocus);

		connect(_right_button, &QPushButton::clicked, this, &MiniSearcher::right_clicked);

		_layout->addWidget(_right_button);
	}

	this->hide();
}


bool MiniSearcher::is_initiator(QKeyEvent* event)
{
    QString text = event->text();

	if(event->modifiers() & Qt::ControlModifier){
		return false;
	}

	if(text.isEmpty()){
		return false;
	}

	if(text[0].isLetterOrNumber()){
		return true;
	}

	if(_triggers.contains(text[0]) ){
		return true;
	}

	return false;
}


void MiniSearcher::keyPressEvent(QKeyEvent* event)
{
	int key = event->key();

	switch(key)
	{
		case Qt::Key_Enter:
		case Qt::Key_Return:
			if(this->isVisible()) {
				reset();
			}

			break;

		case Qt::Key_Down:
			right_clicked();
			break;

		case Qt::Key_Up:
			left_clicked();
			break;

		default:
			QFrame::keyPressEvent(event);
			break;
	}
}


void MiniSearcher::showEvent(QShowEvent* e)
{
	if(!_esc_shortcut){
		_esc_shortcut = new QShortcut(QKeySequence(Qt::Key_Escape),
									  this,
									  SLOT(reset()),
									  SLOT(reset()),
									  Qt::WidgetWithChildrenShortcut);
	}

	_esc_shortcut->setEnabled(true);

	QFrame::showEvent(e);
}

void MiniSearcher::hideEvent(QHideEvent* e)
{
	_esc_shortcut->setEnabled(false);
	_parent->setFocus();

	QFrame::hideEvent(e);
}


void MiniSearcher::focusOutEvent(QFocusEvent* e)
{
	this->hide();

	QFrame::focusOutEvent(e);
}


void MiniSearcher::line_edit_text_changed(const QString& str)
{
	emit sig_text_changed(str);
}


void MiniSearcher::line_edit_focus_lost()
{
	if(	_left_button->hasFocus() ||
		_right_button->hasFocus() ||
		this->hasFocus())
	{
		return;
	}

	reset();
}


void MiniSearcher::left_clicked()
{
	emit sig_find_prev_row();
	this->_line_edit->setFocus();
}


void MiniSearcher::right_clicked()
{
	emit sig_find_next_row();
	this->_line_edit->setFocus();
}


void MiniSearcher::init(QString text)
{
    int sb_width = _parent->verticalScrollBar()->width();
    int sb_height = _parent->horizontalScrollBar()->height();
    int par_width = _parent->width();
    int par_height = _parent->height();
    int new_width, new_height;

    if(!_parent->verticalScrollBar()->isVisible()) sb_width = 0;
    if(!_parent->horizontalScrollBar()->isVisible()) sb_height = 0;

	new_width = par_width - (sb_width + 135);
	new_height = par_height - (sb_height + 40);

	this->setGeometry(new_width, new_height, 130, 35);

	this->_line_edit->setFocus();
	this->_line_edit->setText(text);

	this->show();
}


void MiniSearcher::reset()
{
	this->_line_edit->setText("");
	if(this->isVisible()){
		_parent->setFocus();
	}

	this->hide();
}


bool MiniSearcher::check_and_init(QKeyEvent *event)
{
	if(!is_initiator(event)) {
		return false;
	}

    if(!this->isVisible()) {
        init(event->text());
        return true;
    }

	return false;
}


void MiniSearcher::set_extra_triggers(const QMap<QChar, QString>& triggers)
{
	_triggers = triggers;
	QString tooltip;

	for(const QChar& key : triggers.keys()) {
		tooltip += QString(key) + " = " + triggers.value(key) + "\n";
	}

	tooltip.remove(tooltip.size() -1, 1);

	this->setToolTip(tooltip);
}


QString MiniSearcher::get_current_text()
{
	return _line_edit->text();
}


