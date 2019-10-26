
/* Copyright (C) 2011-2019  Lucio Carreras
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

#ifndef PLAYLISTBOTTOMBAR_H
#define PLAYLISTBOTTOMBAR_H

#include "Gui/Utils/Widgets/Widget.h"

#include "Components/Shutdown/Shutdown.h"
#include "Utils/Pimpl.h"
#include "Utils/Macros.h"

#ifdef SAYONARA_WITH_SHUTDOWN
	class GUI_Shutdown;
#endif

namespace Playlist
{
	class Mode;

	/**
	 * @brief The GUI_PlaylistBottomBar class
	 * @ingroup GuiPlaylists
	 */
	class BottomBar :
			public Gui::Widget
	{
		Q_OBJECT
		PIMPL(BottomBar)

		signals:
			void sig_show_numbers_changed(bool active);
			void sig_playlist_mode_changed(const ::Playlist::Mode& mode);

		public:
			explicit BottomBar(QWidget *parent=nullptr);
			~BottomBar() override;

			void check_dynamic_play_button();

		protected:
			void language_changed() override;
			void skin_changed() override;
			void showEvent(QShowEvent* e) override;
			void resizeEvent(QResizeEvent* e) override;

		private slots:
			void rep1_checked(bool checked);
			void rep_all_checked(bool checked);
			void shuffle_checked(bool checked);
			void playlist_mode_changed();
			void gapless_clicked();

			void s_playlist_mode_changed();

		#ifdef SAYONARA_WITH_SHUTDOWN
			void shutdown_clicked();
			void shutdown_started(MilliSeconds time2go);
			void shutdown_closed();
		#endif
	};
}

#endif // PLAYLISTBOTTOMBAR_H