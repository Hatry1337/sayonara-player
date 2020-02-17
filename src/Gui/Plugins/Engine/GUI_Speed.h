/* GUI_Speed.h */

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

#ifndef GUI_SPEED_H
#define GUI_SPEED_H

#include "Gui/Plugins/PlayerPluginBase.h"

UI_FWD(GUI_Speed)

class GUI_Speed : public PlayerPlugin::Base
{
	Q_OBJECT
	UI_CLASS(GUI_Speed)

public:
	explicit GUI_Speed(QWidget* parent=nullptr);
	~GUI_Speed() override;

	QString name() const override;
	QString displayName() const override;

private:
	void retranslate() override;
	void initUi() override;

private slots:
	void speedChanged(int value);
	void activeChanged(bool enabled);
	void activeToggled(bool enabled);
	void preservePitchChanged(bool enabled);
	void pitchChanged(int pitch);

	void revertSpeedClicked();
	void revertPitchClicked();

	void pitchHovered(int val);
	void speedHovered(int val);

	void currentTabChanged(int idx);

	void pitchFoundChanged();
};

#endif // GUI_SPEED_H
