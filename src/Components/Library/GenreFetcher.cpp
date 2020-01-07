/* GenreFetcher.cpp */

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

#include "GenreFetcher.h"

#include "Components/Library/LocalLibrary.h"
#include "Components/Tagging/ChangeNotifier.h"
#include "Components/Tagging/UserTaggingOperations.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Set.h"

#include <QTimer>

struct GenreFetcher::Private
{
	LocalLibrary*					local_library=nullptr;
	Util::Set<Genre>				genres;
	Util::Set<Genre>				additional_genres; // empty genres that are inserted
	Tagging::UserOperations*		uto=nullptr;
};

GenreFetcher::GenreFetcher(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	Tagging::ChangeNotifier* mcn = Tagging::ChangeNotifier::instance();

	connect(mcn, &Tagging::ChangeNotifier::sig_metadata_changed, this, &GenreFetcher::reload_genres);
	connect(mcn, &Tagging::ChangeNotifier::sig_metadata_deleted, this, &GenreFetcher::reload_genres);
}

Tagging::UserOperations* GenreFetcher::init_tagging()
{
	if(!m->uto)
	{
		m->uto = new Tagging::UserOperations(-1, this);
		connect(m->uto, &Tagging::UserOperations::sig_progress, this, &GenreFetcher::sig_progress);
		connect(m->uto, &Tagging::UserOperations::sig_finished, this, &GenreFetcher::sig_finished);
	}

	return m->uto;
}

GenreFetcher::~GenreFetcher() = default;

void GenreFetcher::reload_genres()
{
	if(!m->local_library){
		return;
	}

	LibraryId library_id = m->local_library->id();

	DB::LibraryDatabase* lib_db = DB::Connector::instance()->library_db(library_id, 0);
	m->genres = lib_db->getAllGenres();

	emit sig_genres_fetched();
}

Util::Set<Genre> GenreFetcher::genres() const
{
	Util::Set<Genre> genres(m->genres);
	for(const Genre& genre : m->additional_genres)
	{
		genres.insert(genre);
	}

	return genres;
}


void GenreFetcher::create_genre(const Genre& genre)
{
	m->additional_genres << genre;
	emit sig_genres_fetched();
}

void GenreFetcher::add_genre_to_md(const MetaDataList& v_md, const Genre& genre)
{
	Tagging::UserOperations* uto = init_tagging();
	uto->add_genre_to_md(v_md, genre);
}

void GenreFetcher::delete_genre(const Genre& genre)
{
	Tagging::UserOperations* uto = init_tagging();
	uto->delete_genre(genre);
}

void GenreFetcher::delete_genres(const Util::Set<Genre>& genres)
{
	for(const Genre& genre : genres)
	{
		delete_genre(genre);
	}
}

void GenreFetcher::rename_genre(const Genre& old_genre, const Genre& new_genre)
{
	Tagging::UserOperations* uto = init_tagging();
	uto->rename_genre(old_genre, new_genre);
}

void GenreFetcher::set_local_library(LocalLibrary* local_library)
{
	m->local_library = local_library;
	connect(m->local_library, &LocalLibrary::sig_reloading_library_finished,
			this, &GenreFetcher::reload_genres);

	QTimer::singleShot(200, this, SLOT(reload_genres()));
}
