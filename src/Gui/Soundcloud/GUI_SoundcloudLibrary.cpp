/* GUI_SoundCloudLibrary.cpp */

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

#include "GUI_SoundcloudLibrary.h"
#include "Gui/Soundcloud/ui_GUI_SoundcloudLibrary.h"
#include "Gui/Soundcloud/GUI_SoundcloudArtistSearch.h"
#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"
#include "Components/Streaming/Soundcloud/SoundcloudLibrary.h"

#include "Utils/Settings/Settings.h"

#include <QShortcut>
#include <QMenu>
#include <QAction>

using SC::GUI_ArtistSearch;

struct SC::GUI_Library::Private
{
	GUI_ArtistSearch*	artistSearch=nullptr;
	QMenu*              libraryMenu=nullptr;
};

using SC::GUI_Library;

GUI_Library::GUI_Library(Library* library, QWidget* parent) :
	GUI_AbstractLibrary(library, parent)
{
	setupParent(this, &ui);
	setAcceptDrops(false);

	this->setFocusProxy(ui->le_search);

	m = Pimpl::make<GUI_Library::Private>();

	m->artistSearch = new GUI_ArtistSearch(library, this);
	m->libraryMenu = new QMenu(this);

	QAction* action_add_artist = m->libraryMenu->addAction(tr("Add artist"));
	connect(action_add_artist, &QAction::triggered, this, &GUI_Library::btnAddClicked);

	library->load();
}

GUI_Library::~GUI_Library()
{
	if(ui)
	{
		delete ui; ui = nullptr;
	}
}


QMenu* GUI_Library::getMenu() const
{
	return m->libraryMenu;
}

QFrame* GUI_Library::headerFrame() const
{
	return ui->header_frame;
}

QList<::Library::Filter::Mode> GUI_Library::searchOptions() const
{
	return {::Library::Filter::Fulltext};
}

Library::TrackDeletionMode GUI_Library::showDeleteDialog(int n_tracks)
{
	Q_UNUSED(n_tracks)
	return ::Library::TrackDeletionMode::OnlyLibrary;
}

void GUI_Library::btnAddClicked()
{
	m->artistSearch->show();
}

Library::TableView* GUI_Library::lvArtist() const
{
	return ui->tv_artists;
}

Library::TableView* GUI_Library::lvAlbum() const
{
	return ui->tv_albums;
}

Library::TableView* GUI_Library::lvTracks() const
{
	return ui->tv_tracks;
}

Library::SearchBar* GUI_Library::leSearch() const
{
	return ui->le_search;
}


void GUI_Library::showEvent(QShowEvent* e)
{
	GUI_AbstractLibrary::showEvent(e);

	this->lvAlbum()->resizeRowsToContents();
	this->lvArtist()->resizeRowsToContents();
	this->lvTracks()->resizeRowsToContents();

	ui->splitter_artists->restoreState(GetSetting(Set::Lib_SplitterStateArtist));
	ui->splitter_tracks->restoreState(GetSetting(Set::Lib_SplitterStateTrack));
}

