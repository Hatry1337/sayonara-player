/* PlaylistContextMenu.cpp */

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

#include "PlaylistContextMenu.h"
#include "PlaylistBookmarksMenu.h"

#include "Gui/Playlist/PlaylistActionMenu.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Widgets/RatingLabel.h"
#include "Utils/Language/Language.h"
#include "Utils/MetaData/MetaData.h"

using Playlist::ContextMenu;
using Playlist::BookmarksMenu;
using Playlist::ActionMenu;

struct ContextMenu::Private
{
	QMap<ContextMenu::Entry, QAction*> entryActionMap;

	BookmarksMenu*	bookmarksMenu=nullptr;
	QMenu*			ratingMenu=nullptr;

	QAction*		playlistModeAction=nullptr;
	QMenu*			playlistModeMenu=nullptr;

	Private(ContextMenu* parent)
	{
		ratingMenu = new QMenu(parent);

		bookmarksMenu = new BookmarksMenu(parent);

		playlistModeMenu = new ActionMenu(parent);
		playlistModeAction = parent->addMenu(playlistModeMenu);

		entryActionMap[EntryRating] = parent->addMenu(ratingMenu);
		entryActionMap[EntryBookmarks] = parent->addMenu(bookmarksMenu);
		entryActionMap[EntryCurrentTrack] = new QAction(parent);
		entryActionMap[EntryFindInLibrary] = new QAction(parent);
		entryActionMap[EntryReverse] = new QAction(parent);

		parent->addActions
		({
			entryActionMap[EntryReverse],
			entryActionMap[EntryCurrentTrack],
			entryActionMap[EntryFindInLibrary],
		});
	}
};

ContextMenu::ContextMenu(QWidget* parent) :
	Library::ContextMenu(parent)
{
	m = Pimpl::make<Private>(this);

	QList<QAction*> ratingActions;
	for(int i=int(Rating::Zero); i != int(Rating::Last); i++)
	{
		ratingActions << initRatingAction(Rating(i), m->ratingMenu);
	}

	m->ratingMenu->addActions(ratingActions);

	connect(m->entryActionMap[EntryCurrentTrack],	&QAction::triggered, this, &ContextMenu::sigJumpToCurrentTrack);
	connect(m->entryActionMap[EntryFindInLibrary],	&QAction::triggered, this, &ContextMenu::sigFindTrackTriggered);
	connect(m->entryActionMap[EntryReverse],			&QAction::triggered, this, &ContextMenu::sigReverseTriggered);

	connect(m->bookmarksMenu, &BookmarksMenu::sigBookmarkPressed, this, &ContextMenu::bookmarkPressed);

	skinChanged();
}

ContextMenu::~ContextMenu() = default;

ContextMenu::Entries ContextMenu::entries() const
{
	ContextMenu::Entries entries = Library::ContextMenu::entries();

	for(auto it=m->entryActionMap.begin(); it != m->entryActionMap.end(); it++)
	{
		if(it.value()->isVisible()) {
			entries |= it.key();
		}
	}

	return entries;
}

void ContextMenu::showActions(ContextMenu::Entries entries)
{
	Library::ContextMenu::showActions(entries);

	for(auto it=m->entryActionMap.begin(); it != m->entryActionMap.end(); it++)
	{
		it.value()->setVisible(entries & it.key());
	}
}

void ContextMenu::setRating(Rating rating)
{
	QList<QAction*> actions = m->ratingMenu->actions();
	for(QAction* action : actions)
	{
		auto data = action->data().value<Rating>();
		action->setChecked(data == rating);
	}

	QString rating_text = Lang::get(Lang::Rating);
	if(rating != Rating::Zero && rating != Rating::Last)
	{
		QString text = QString("%1 (%2)")
							.arg(rating_text)
							.arg(int(rating));

		m->entryActionMap[EntryRating]->setText(text);
	}

	else {
		m->entryActionMap[EntryRating]->setText(rating_text);
	}
}

void ContextMenu::setMetadata(const MetaData& md)
{
	m->bookmarksMenu->setMetadata(md);
}

QAction* ContextMenu::initRatingAction(Rating rating, QObject* parent)
{
	auto* action = new QAction
	(
		QString::number(int(rating)),
		parent
	);

	action->setData(QVariant::fromValue(rating));
	action->setCheckable(true);

	connect(action, &QAction::triggered, this, [=](bool b)
	{
		Q_UNUSED(b)
		emit sigRatingChanged(rating);
	});

	return action;
}

void ContextMenu::languageChanged()
{
	Library::ContextMenu::languageChanged();

	m->entryActionMap[EntryRating]-> setText(Lang::get(Lang::Rating));
	m->entryActionMap[EntryBookmarks]-> setText(Lang::get(Lang::Bookmarks));
	m->entryActionMap[EntryCurrentTrack]->setText(tr("Jump to current track"));
	m->entryActionMap[EntryFindInLibrary]->setText(tr("Show track in library"));
	m->entryActionMap[EntryReverse]->setText(Lang::get(Lang::ReverseOrder));

	m->playlistModeAction->setText(tr("Playlist mode"));
}

void ContextMenu::skinChanged()
{
	Library::ContextMenu::skinChanged();

	using namespace Gui;
	m->entryActionMap[EntryRating]->setIcon(Icons::icon(Icons::Star));
	m->entryActionMap[EntryFindInLibrary]->setIcon(Icons::icon(Icons::Search));
}

void ContextMenu::bookmarkPressed(Seconds timestamp)
{
   emit sigBookmarkPressed(timestamp);
}
