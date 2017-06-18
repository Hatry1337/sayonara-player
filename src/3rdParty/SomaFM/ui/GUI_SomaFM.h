/* GUI_SomaFM.h */

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

/* GUI_SomaFM.h */

#ifndef GUI_SOMAFM_H
#define GUI_SOMAFM_H

#include "GUI/Helper/SayonaraWidget/SayonaraWidget.h"

#include <QItemSelection>


class QComboBox;
class QFrame;

namespace Ui
{
	class GUI_SomaFM;
}

namespace SomaFM
{
	class Station;
	class Library;

	class GUI_SomaFM :
			public SayonaraWidget
	{
		Q_OBJECT

	private:
		SomaFM::Library*		_library=nullptr;
		Ui::GUI_SomaFM*		ui=nullptr;


	public:
		explicit GUI_SomaFM(QWidget *parent = 0);
		~GUI_SomaFM();

		QFrame* header_frame() const;

	private slots:
		void stations_loaded(const QList<SomaFM::Station>& stations);
		void station_changed(const SomaFM::Station& station);

		void station_double_clicked(const QModelIndex& idx);
		void station_clicked(const QModelIndex& idx);
		void station_index_changed(const QModelIndex& idx);
		void playlist_double_clicked(const QModelIndex& idx);
		void cover_found(const QString& cover_path);

		void selection_changed(const QModelIndexList& selected);


	private:
		SomaFM::Station get_station(int row) const;
	};
}

#endif // GUI_SOMAFM_H
