/* GUI_PlayerPlugin.h */

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


/* GUI_PlayerPlugin.h */

#ifndef GUI_PLAYERPLUGIN_H
#define GUI_PLAYERPLUGIN_H

#include "GUI/Helper/SayonaraWidget/SayonaraWidget.h"

namespace Ui { class GUI_PlayerPlugin; }

class PlayerPluginInterface;
class GUI_PlayerPlugin :
		public SayonaraWidget

{
	Q_OBJECT

public:
	explicit GUI_PlayerPlugin(QWidget *parent = nullptr);
	virtual ~GUI_PlayerPlugin();

	void set_content(PlayerPluginInterface* player_plugin);
	void show(PlayerPluginInterface* player_plugin);

private:
	PlayerPluginInterface* _current_plugin=nullptr;
	Ui::GUI_PlayerPlugin* ui=nullptr;

private:
	void close_cur_plugin();

protected:
    void language_changed() override;
    void closeEvent(QCloseEvent *e) override;

};

#endif // GUI_PLAYERPLUGIN_H
