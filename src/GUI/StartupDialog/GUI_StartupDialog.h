/* GUI_StartupDialog.h */

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



#ifndef GUI_StartupDialog_H
#define GUI_StartupDialog_H

#include "GUI/StartupDialog/ui_GUI_StartupDialog.h"


#include "Interfaces/PreferenceDialog/PreferenceDialogInterface.h"

class GUI_StartupDialog :
		public PreferenceDialogInterface,
		private Ui::GUI_StartupDialog
{

    Q_OBJECT

	friend class PreferenceDialogInterface;

public:
    GUI_StartupDialog(QWidget *parent=nullptr);
    virtual ~GUI_StartupDialog();
    
protected:
	void init_ui() override;
	void language_changed() override;
	QString get_action_name() const override;
	QLabel* get_title_label() override;


private slots:
    void cb_toggled(bool);
    void ok_clicked();


};



#endif // GUI_StartupDialog_H
