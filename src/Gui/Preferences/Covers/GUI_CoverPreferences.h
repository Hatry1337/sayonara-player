/* GUI_CoverPreferences.h */

/* Copyright (C) 2011-2020  Lucio Carreras
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

#ifndef GUI_COVERPREFERENCES_H
#define GUI_COVERPREFERENCES_H

#include "Gui/Preferences/PreferenceWidget.h"

UI_FWD(GUI_CoverPreferences)

class GUI_CoverPreferences :
		public Preferences::Base
{
	Q_OBJECT
	UI_CLASS(GUI_CoverPreferences)

public:
	explicit GUI_CoverPreferences(const QString& identifier);
	~GUI_CoverPreferences() override;

	bool commit() override;
	void revert() override;

	QString action_name() const override;

protected:
	void init_ui() override;
	void retranslate_ui() override;
	void skin_changed() override;

private slots:
	void up_clicked();
	void down_clicked();
	void add_clicked();
	void remove_clicked();

	void current_row_changed(int row);
	void delete_covers_from_db();
	void delete_cover_files();
	void fetch_covers_www_triggered(bool b);

	void cb_save_to_library_toggled(bool b);
	void le_cover_template_edited(const QString& text);
};


#endif // GUI_CoverPreferences_H
