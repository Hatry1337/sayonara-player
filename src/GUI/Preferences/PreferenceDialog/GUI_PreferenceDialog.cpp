/* GUI_PreferenceDialog.cpp */

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

#include "GUI_PreferenceDialog.h"
#include "GUI/Preferences/ui_GUI_PreferenceDialog.h"

#include "GUI/Helper/Delegates/ComboBoxDelegate.cpp"
#include "Interfaces/PreferenceDialog/PreferenceWidgetInterface.h"
#include "Helper/globals.h"

#include <QLayout>
#include <QItemDelegate>


GUI_PreferenceDialog::GUI_PreferenceDialog(QWidget *parent) :
	PreferenceDialogInterface(parent) {}

GUI_PreferenceDialog::~GUI_PreferenceDialog()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

void GUI_PreferenceDialog::register_preference_dialog(PreferenceWidgetInterface* dialog)
{
	_dialogs << dialog;
}


void GUI_PreferenceDialog::retranslate_ui()
{
	ui->retranslateUi(this);

	// TODO: Init should not be so deep in language changed
	bool is_empty = (ui->list_preferences->count() == 0);

	int i=0;
	for(PreferenceWidgetInterface* dialog : _dialogs){
		QListWidgetItem* item;
		if(is_empty){
			item = new QListWidgetItem(dialog->get_action_name());
			item->setSizeHint(QSize(item->sizeHint().width(), 20));
			ui->list_preferences->addItem(item);
		}
		else{
			item = ui->list_preferences->item(i);
			item->setText(dialog->get_action_name());
		}

		i++;
	}
}


QString GUI_PreferenceDialog::get_action_name() const
{
	return tr("Preferences");
}


void GUI_PreferenceDialog::commit_and_close()
{
	commit();
	close();
}


void GUI_PreferenceDialog::commit()
{
	for(PreferenceWidgetInterface* iface : _dialogs){
		if(iface->is_ui_initialized()){
			iface->commit();
		}
	}
}


void GUI_PreferenceDialog::revert()
{
	for(PreferenceWidgetInterface* iface : _dialogs){
		if(iface->is_ui_initialized()){
			iface->revert();
		}
	}

	close();
}


void GUI_PreferenceDialog::row_changed(int row)
{
	if(!between(row, _dialogs)){
		return;
	}

	hide_all();

	PreferenceWidgetInterface* widget = _dialogs[row];

	QLayout* layout = ui->widget_preferences->layout();
	layout->setContentsMargins(0,0,0,0);

	if(layout){
		layout->addWidget(widget);
		layout->setAlignment(Qt::AlignTop);
	}

	ui->lab_pref_title->setText(widget->get_action_name());

	widget->show();
}


void GUI_PreferenceDialog::hide_all()
{
	for(PreferenceWidgetInterface* iface : _dialogs){
		iface->setParent(nullptr);
		iface->hide();
	}
}

// TODO: This seems strange and useless
void GUI_PreferenceDialog::showEvent(QShowEvent* e)
{
	init_ui();
	QDialog::showEvent(e);
}


void GUI_PreferenceDialog::init_ui()
{
	if(is_ui_initialized()){
		return;
	}

	setup_parent(this, &ui);

	ui->list_preferences->setMouseTracking(false);
	ui->list_preferences->setItemDelegate(new QItemDelegate(ui->list_preferences));

	connect(ui->list_preferences, &QListWidget::currentRowChanged, this, &GUI_PreferenceDialog::row_changed);
	connect(ui->btn_apply, &QPushButton::clicked, this, &GUI_PreferenceDialog::commit);
	connect(ui->btn_ok, &QPushButton::clicked, this, &GUI_PreferenceDialog::commit_and_close);
	connect(ui->btn_cancel, &QPushButton::clicked, this, &GUI_PreferenceDialog::revert);
}
