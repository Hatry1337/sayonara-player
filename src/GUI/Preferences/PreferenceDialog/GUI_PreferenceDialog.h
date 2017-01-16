/* GUI_PreferenceDialog.h */

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

#ifndef GUI_PreferenceDialog_H
#define GUI_PreferenceDialog_H

#include "Interfaces/PreferenceDialog/PreferenceDialogInterface.h"

#include <QList>

namespace Ui
{
	class GUI_PreferenceDialog;
}

class PreferenceWidgetInterface;

/**
 * @brief The Preference Dialog. Register new Preference dialogs with the register_preference_dialog() method.
 * @ingroup Preferences
 */
class GUI_PreferenceDialog :
		public PreferenceDialogInterface
{
	Q_OBJECT

public:
	explicit GUI_PreferenceDialog(QWidget *parent = 0);
	~GUI_PreferenceDialog();

	void commit() override;
	void revert() override;

	QString get_action_name() const override;

	void register_preference_dialog(PreferenceWidgetInterface* dialog);

protected slots:
	void commit_and_close();
	void row_changed(int row);

protected:
	void init_ui() override;
	void retranslate_ui() override;
	void showEvent(QShowEvent *e) override;

	void hide_all();

private:
	Ui::GUI_PreferenceDialog* ui=nullptr;
	QList<PreferenceWidgetInterface*> _dialogs;
};

#endif // GUI_PreferenceDialog_H
