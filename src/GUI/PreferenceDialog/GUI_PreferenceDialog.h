/* GUI_PreferenceDialog.h */

/* Copyright (C) 2011-2016  Lucio Carreras
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



#ifndef GUI_PreferenceDialog_H
#define GUI_PreferenceDialog_H

#include <QDialog>
#include <QVector>
#include "GUI/PreferenceDialog/ui_GUI_PreferenceDialog.h"
#include "GUI/Helper/SayonaraWidget.h"
#include "Interfaces/PreferenceDialog/PreferenceDialogInterface.h"

#include <QSpacerItem>

class PreferenceWidgetInterface;

class GUI_PreferenceDialog :
		public PreferenceDialogInterface,
		private Ui::GUI_PreferenceDialog
{

	friend class PreferenceDialogInterface;
	friend class PreferenceInterface<SayonaraDialog>;

	Q_OBJECT

public:
	explicit GUI_PreferenceDialog(QWidget *parent = 0);
	~GUI_PreferenceDialog();

	QString get_action_name() const override;
	QLabel* get_title_label() override;
	void init_ui() override;

	void register_preference_dialog(PreferenceWidgetInterface* dialog);

	QWidget* get_widget();


protected slots:
	void language_changed() override;

	void commit_and_close();
	void commit() override;
	void revert() override;

	void row_changed(int row);

protected:
	void hide_all();

private:
	QVector<PreferenceWidgetInterface*> _dialogs;
};

#endif // GUI_PreferenceDialog_H
