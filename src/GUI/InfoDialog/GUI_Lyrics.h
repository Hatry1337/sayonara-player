/* GUI_Lyrics.h */

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



#ifndef GUI_LYRICS_H
#define GUI_LYRICS_H

#include <QWidget>
#include "Helper/Pimpl.h"
#include "GUI/Helper/SayonaraWidget/SayonaraWidget.h"

class MetaData;
namespace Ui {
	class GUI_Lyrics;
}

class GUI_Lyrics :
		public SayonaraWidget
{
	Q_OBJECT
	PIMPL(GUI_Lyrics)

signals:
	void sig_closed();

public:
	explicit GUI_Lyrics(QWidget *parent = nullptr);
	~GUI_Lyrics();

	void set_metadata(const MetaData& md);

private:
	Ui::GUI_Lyrics *ui=nullptr;


	void init();

	void zoom(qreal font_size);
	void setup_sources();
	void choose_source();
	void show_lyrics(const QString& lyrics, const QString& header, bool rich);
	void show_local_lyrics();
	void set_save_button_text();

private slots:
	void zoom_in();
	void zoom_out();

	void lyrics_fetched();
	void lyric_server_changed(int idx);

	void switch_pressed();
	void prepare_lyrics();
	void save_lyrics_clicked();

protected:
    void language_changed() override;

	void showEvent(QShowEvent* e) override;
	void wheelEvent(QWheelEvent* e) override;
	void keyPressEvent(QKeyEvent* e) override;
};


#endif // GUI_LYRICS_H
