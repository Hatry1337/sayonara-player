/* GUI_LanguageChooser.h */

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

#ifndef GUI_LANGUAGECHOOSER_H
#define GUI_LANGUAGECHOOSER_H

#include "Interfaces/PreferenceDialog/PreferenceWidgetInterface.h"

#include <QMap>

UI_FWD(GUI_LanguageChooser)

class GUI_LanguageChooser :
		public PreferenceWidgetInterface
{
    Q_OBJECT
	UI_CLASS(GUI_LanguageChooser)

public:
    explicit GUI_LanguageChooser(QWidget *parent=nullptr);
    virtual ~GUI_LanguageChooser();

	void commit() override;
	void revert() override;

	QString get_action_name() const override;

protected:
	void init_ui() override;
	void retranslate_ui() override;
	void showEvent(QShowEvent*) override;

private:
	// todo: pimpl
	QMap<QString, QString>		_map;

private:
	void renew_combo();
};

#endif // GUI_LANGUAGECHOOSER_H
