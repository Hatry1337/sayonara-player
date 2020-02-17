
/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#ifndef GUI_FONTPREFERENCES_H
#define GUI_FONTPREFERENCES_H

#include "Gui/Utils/Widgets/Widget.h"
#include "Utils/Pimpl.h"

UI_FWD(GUI_FontPreferences)

class QFont;
class QFontDatabase;

class GUI_FontPreferences :
		public Gui::Widget
{
	Q_OBJECT
	UI_CLASS(GUI_FontPreferences)
	PIMPL(GUI_FontPreferences)

public:
	explicit GUI_FontPreferences(QWidget* parent=nullptr);
	~GUI_FontPreferences() override;

	bool commit();
	void revert();

	QString actionName() const;

protected:
	void languageChanged() override;
	void skinChanged() override;
	void showEvent(QShowEvent* e) override;

protected slots:
	void defaultClicked();
	void comboFontsChanged(const QFont& font);

private:
	QStringList availableFontSizes(const QString& fontName, const QString& style=QString());
	QStringList availableFontSizes(const QFont& font);

	void fillSizes(const QStringList& sizes);
	void initUi();
};

#endif // FONTCONFIG_H
