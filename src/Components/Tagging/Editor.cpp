/* TagEdit.cpp */

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

#include "Editor.h"
#include "ChangeNotifier.h"

#include "Utils/Tagging/Tagging.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/globals.h"
#include "Utils/Logger/Logger.h"
#include "Database/LibraryDatabase.h"
#include "Database/DatabaseConnector.h"

#include <QHash>
#include <algorithm>

using namespace Tagging;

struct Editor::Private
{
	MetaDataList			v_md;			// the current metadata
	MetaDataList			v_md_orig;		// the original metadata

	MetaDataList			v_md_before_change;
	MetaDataList			v_md_after_change;
	BoolList				changed_md;	// indicates if metadata at idx was changed
	QMap<int, QImage>		cover_map;

	QHash<QString, ArtistID> artist_map;
	QHash<QString, AlbumID> album_map;

	DB::LibraryDatabase*	ldb=nullptr;	// database of LocalLibrary
	bool						notify;

	ArtistID get_artist_id(const QString& artist_name)
	{
		if(artist_map.contains(artist_name)){
			return artist_map[artist_name];
		} else {
			ArtistID id = ldb->getArtistID(artist_name);
			if(id < 0){
				id = ldb->insertArtistIntoDatabase(artist_name);
			}
			artist_map[artist_name] = id;
			return id;
		}
	}

	AlbumID get_album_id(const QString& album_name)
	{
		if(album_map.contains(album_name)){
			return album_map[album_name];
		} else {
			AlbumID id = ldb->getAlbumID(album_name);
			if(id < 0){
				id = ldb->insertAlbumIntoDatabase(album_name);
			}
			album_map[album_name] = id;
			return id;
		}
	}
};

Editor::Editor(QObject *parent) :
	QThread(parent)
{
	m = Pimpl::make<Editor::Private>();
	m->ldb = DB::Connector::instance()->library_db(-1, 0);
	m->notify = true;

	connect(this, &QThread::finished, this, &Editor::thread_finished);
}

Editor::Editor(const MetaDataList& v_md, QObject* parent) :
	Editor(parent)
{
	set_metadata(v_md);
}

Editor::~Editor() {}


void Editor::update_track(int idx, const MetaData& md)
{
	bool has_changed = !( md.is_equal_deep( m->v_md_orig[idx]) );
	m->changed_md[idx] = has_changed;
	m->v_md[idx] = md;
}

void Editor::undo(int idx)
{
	m->v_md[idx] = m->v_md_orig[idx];
}


void Editor::undo_all()
{
	m->v_md = m->v_md_orig;
}

const MetaData& Editor::metadata(int idx) const
{
	return m->v_md[idx];
}

const MetaDataList& Editor::metadata() const
{
	return m->v_md;
}


int Editor::count() const
{
	return m->v_md.size();
}

void Editor::add_genre(int idx, const QString &genre)
{
	if(!between(idx, m->v_md)){
		return;
	}

	MetaData& md = m->v_md[idx];

	if(md.add_genre(Genre(genre))){
		m->changed_md[idx] = true;
	}
}

void Editor::delete_genre(int idx, const QString& genre)
{
	if(!between(idx, m->v_md)){
		return;
	}

	MetaData& md = m->v_md[idx];
	if(md.remove_genre(Genre(genre))){
		m->changed_md[idx] = true;
	}
}

void Editor::rename_genre(int idx, const QString& genre, const QString& new_name)
{
	if(!between(idx, m->v_md)){
		return;
	}

	MetaData& md = m->v_md[idx];
	if(md.remove_genre(Genre(genre))){
		m->changed_md[idx] = true;
	}

	if(md.add_genre(Genre(new_name))){
		m->changed_md[idx] = true;
	}
}


void Editor::set_metadata(const MetaDataList& v_md)
{
	m->v_md = v_md;
	m->v_md_orig = v_md;

	m->cover_map.clear();
	m->changed_md.assign(v_md.size(), false);

	if( v_md.size() > 0) {
		m->ldb = DB::Connector::instance()->library_db(v_md.first().library_id, 0);
	}

	emit sig_metadata_received(m->v_md);
}

bool Editor::is_cover_supported(int idx) const
{
	return Util::is_cover_supported( m->v_md[idx].filepath() );
}



void Editor::apply_artists_and_albums_to_md()
{
	for(int i=0; i<m->v_md.count(); i++)
	{
		bool changed = m->changed_md[i];
		if( !changed )
		{
			continue;
		}

		MetaData& md = m->v_md[i];

		ArtistID artist_id = m->get_artist_id(md.artist());
		AlbumID album_id = m->get_album_id(md.album());
		ArtistID album_artist_id = m->get_artist_id(md.album_artist());

		md.album_id = album_id;
		md.artist_id = artist_id;
		md.set_album_artist_id(album_artist_id);
	}
}


void Editor::update_cover(int idx, const QImage& cover)
{
	if(cover.isNull()){
		return;
	}

	if(!between(idx, m->v_md) ){
		return;
	}

	bool cover_supported = is_cover_supported(idx);
	if(!cover_supported) {
		return;
	}

	m->cover_map[idx] = cover;
}


bool Editor::has_cover_replacement(int idx) const
{
	return m->cover_map.contains(idx);
}

void Editor::run()
{
	MetaDataList v_md;
	MetaDataList v_md_orig;
	DB::Connector* db;

	sp_log(Log::Debug, this) << "Apply albums and artists";
	apply_artists_and_albums_to_md();

	sp_log(Log::Debug, this) << "Have to change" <<
						  std::count(m->changed_md.begin(), m->changed_md.end(), true)
					   << " tracks";

	int i=0;
	int n_operations = m->v_md.size() + m->cover_map.size();

	for(i=0; i<m->v_md.count(); i++)
	{
		MetaData md = m->v_md[i];
		emit sig_progress( (i * 100) / n_operations);

		if( m->changed_md[i] == false ) {
			continue;
		}

		bool success = Tagging::Util::setMetaDataOfFile(md);
		if( !success ) {
			continue;
		}

		if( !md.is_extern && md.id >= 0 ){
			success = m->ldb->updateTrack(md);
		}

		if(success){
			v_md << std::move(md);
			v_md_orig.push_back(m->v_md_orig[i]);
		}
	}

	for(int idx : m->cover_map.keys())
	{
		Tagging::Util::write_cover(m->v_md[idx], m->cover_map[idx]);
		emit sig_progress( (i++ * 100) / n_operations);
	}

	m->ldb->create_indexes();

	db = DB::Connector::instance();
	db->clean_up();

	m->v_md_after_change = v_md;
	m->v_md_before_change = v_md_orig;
	m->v_md_orig = m->v_md;

	emit sig_progress(-1);
}

void Editor::thread_finished()
{
	if(m->notify){
		ChangeNotifier::instance()->change_metadata(m->v_md_before_change, m->v_md_after_change);
	}
}

void Editor::commit()
{
	this->start();
}


