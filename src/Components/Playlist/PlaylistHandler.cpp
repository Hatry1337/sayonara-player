/* Playlist.cpp */

/* Copyright (C) 2011-2017 Lucio Carreras
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
 *
 *  Created on: Apr 6, 2011
 *      Author: Lucio Carreras
 */

#include "PlaylistHandler.h"
#include "StdPlaylist.h"
#include "StreamPlaylist.h"
#include "PlaylistLoader.h"
#include "PlaylistDBWrapper.h"

#include "Components/Directories/DirectoryReader.h"
#include "Components/PlayManager/PlayManager.h"
#include "Database/DatabaseConnector.h"

#include "Utils/globals.h"
#include "Utils/Parser/PlaylistParser.h"
#include "Utils/Playlist/CustomPlaylist.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Set.h"

#include <algorithm>
#include <memory>
#include <vector>

using PlaylistCollection=std::vector<PlaylistPtr>;
using Playlist::Handler;

struct Handler::Private
{
	DB::Connector*		db=nullptr;
	PlayManagerPtr			play_manager=nullptr;
	PlaylistCollection		playlists;
	int						active_playlist_idx;
	int						current_playlist_idx;
	int						playlist_idx_before_stop;

	Private() :
		db(DB::Connector::instance()),
		play_manager(PlayManager::instance()),
		active_playlist_idx(-1),
		current_playlist_idx(-1),
		playlist_idx_before_stop(-1)
	{}
};

Handler::Handler(QObject * parent) :
	QObject (parent),
	SayonaraClass()
{
	qRegisterMetaType<PlaylistPtr>("PlaylistPtr");
	qRegisterMetaType<PlaylistConstPtr>("PlaylistConstPtr");

	m = Pimpl::make<Private>();

	connect(m->play_manager, &PlayManager::sig_playstate_changed, this, &Handler::playstate_changed);
	connect(m->play_manager, &PlayManager::sig_next, this, &Handler::next);
	connect(m->play_manager, &PlayManager::sig_wake_up, this, &Handler::wake_up);
	connect(m->play_manager, &PlayManager::sig_previous, this, &Handler::previous);
	connect(m->play_manager, &PlayManager::sig_www_track_finished, this, &Handler::www_track_finished);
}

Handler::~Handler()
{
	m->playlists.clear();
}


void Handler::emit_cur_track_changed()
{
	PlaylistPtr pl = active_playlist();

	MetaData md;
	bool success = pl->current_track(md);
	int cur_track_idx = pl->current_track_index();

	m->playlist_idx_before_stop = pl->index();

	if(!success || cur_track_idx == -1){
		m->play_manager->stop();
		return;
	}

	_settings->set(Set::PL_LastPlaylist, pl->get_id());

	m->play_manager->change_track(md, cur_track_idx);

	emit sig_cur_track_idx_changed( cur_track_idx,	pl->index() );
}


int Handler::load_old_playlists()
{
	sp_log(Log::Debug, this) << "Loading playlists...";

	Loader loader;
	loader.create_playlists();

	int last_track_idx = -1;
	int last_playlist_idx = std::max(loader.get_last_playlist_idx(), 0);

	set_active_idx(last_playlist_idx);
	set_current_index(last_playlist_idx);

	if(active_playlist()->count() > 0){
		last_track_idx = std::max(loader.get_last_track_idx(), 0);
	}

	if(last_track_idx >= 0){
		change_track(last_track_idx, last_playlist_idx);
	}

	else{
		m->play_manager->stop();
		return m->playlists.size();
	}

	if(_settings->get(Set::PL_StartPlaying)){
		m->play_manager->play();
	}

	else{
		m->play_manager->pause();
	}

	return m->playlists.size();
}


PlaylistPtr Handler::new_playlist(Playlist::Type type, int playlist_idx, QString name)
{
	if(type == Playlist::Type::Stream) {
		return PlaylistPtr(new Stream(playlist_idx, name));
	}

	return PlaylistPtr(new Standard(playlist_idx, name));
}


int Handler::add_new_playlist(const QString& name, bool temporary, Playlist::Type type)
{
	PlaylistPtr pl;
	int idx = exists(name);

	if(idx >= 0) {
		return idx;
	}

	pl = new_playlist(type, m->playlists.size(), name);
	pl->set_temporary(temporary);

	m->playlists.push_back(pl);

	emit sig_new_playlist_added(pl);

	return pl->index();
}


// create a playlist, where metadata is already available
int Handler::create_playlist(const MetaDataList& v_md, const QString& name, bool temporary, Playlist::Type type)
{
	int idx;
	PlaylistPtr pl;

	idx = exists(name);

	if(idx == -1){
		idx = add_new_playlist(name, temporary, type);
		pl =  m->playlists[idx];
		pl->insert_temporary_into_db();
	}

	pl = m->playlists[idx];

	pl->create_playlist(v_md);
	pl->set_temporary( pl->is_temporary() && temporary );

	set_current_index(idx);

	return idx;
}


// create a new playlist, where only filepaths are given
// Load Folder, Load File...
int Handler::create_playlist(const QStringList& pathlist, const QString& name, bool temporary, Playlist::Type type)
{
	DirectoryReader reader;
	MetaDataList v_md = reader.metadata_from_filelist(pathlist);
	return create_playlist(v_md, name, temporary, type);
}


int Handler::create_playlist(const QString& dir, const QString& name, bool temporary, Playlist::Type type)
{
	QStringList lst{dir};

	return create_playlist(lst, name, temporary, type);
}


int Handler::create_playlist(const CustomPlaylist& cpl)
{
	int id = cpl.id();
	int idx;
	PlaylistPtr pl;

	auto it = std::find_if(m->playlists.begin(), m->playlists.end(), [id](const PlaylistPtr& pl){
		return (pl->get_id() == id);
	});

	if(it == m->playlists.end()){
		idx = add_new_playlist(cpl.name(), cpl.temporary(), Playlist::Type::Std);
	}

	else{
		idx = (*it)->index();
	}

	pl = m->playlists[idx];
	pl->create_playlist(cpl);
	pl->set_changed(false);

	return pl->index();
}

int Handler::create_empty_playlist(bool override_current)
{
	QString name;
	if(!override_current){
		name = request_new_playlist_name();
	}

	return create_playlist(MetaDataList(), name, true);
}

int Handler::create_empty_playlist(const QString& name)
{
	MetaDataList v_md;

	return create_playlist(v_md, name, true);
}


void Handler::clear_playlist(int pl_idx)
{
	if( !between(pl_idx, m->playlists)){
		return;
	}

	m->playlists[pl_idx]->clear();
}


void Handler::playstate_changed(PlayState state)
{
	switch(state){
		case PlayState::Playing:
			played();
			break;
		case PlayState::Paused:
			paused();
			break;
		case PlayState::Stopped:
			stopped();
			break;

		default:
			return;
	}
}

void Handler::played()
{
	active_playlist()->play();
}

void Handler::paused()
{
	active_playlist()->pause();
}

void Handler::stopped()
{
	m->active_playlist_idx = -1;

	for(PlaylistPtr pl : m->playlists){
		pl->stop();
	}

	return;
}

void Handler::next()
{
	active_playlist()->next();
	emit_cur_track_changed();
}

void Handler::wake_up()
{
	bool restore_track_after_stop = _settings->get(Set::PL_RememberTrackAfterStop);

	if(restore_track_after_stop)
	{
		if(active_playlist()->wake_up()){
			emit_cur_track_changed();
			return;
		}
	}

	next();
}


void Handler::previous()
{
	if( m->play_manager->current_position_ms() > 2000)
	{
		m->play_manager->seek_abs_ms(0);
		return;
	}

	active_playlist()->bwd();
	emit_cur_track_changed();
}


void Handler::change_track(int track_idx, int playlist_idx)
{
	bool track_changed;
	PlaylistPtr pl;

	if( !between(playlist_idx, m->playlists) ) {
		playlist_idx = active_playlist()->index();
	}

	if( playlist_idx != m->active_playlist_idx &&
		playlist_idx >= 0 )
	{
		active_playlist()->stop();
		set_active_idx(playlist_idx);
		pl = active_playlist();
	}

	pl = m->playlists[playlist_idx];
	track_changed = pl->change_track(track_idx);

	if(track_changed){
		emit_cur_track_changed();
	}

	else{
		m->play_manager->stop();
	}
}


void Handler::set_active_idx(int idx)
{
	if(between(idx, m->playlists)){
		m->active_playlist_idx = idx;
	}

	else{
		m->active_playlist_idx = active_playlist()->index();
	}

	_settings->set(Set::PL_LastPlaylist, active_playlist()->get_id());
}

void Handler::set_current_index(int pl_idx)
{
	if(pl_idx == m->current_playlist_idx){
		return;
	}

	m->current_playlist_idx = pl_idx;

	emit sig_playlist_idx_changed(pl_idx);
}


void Handler::play_next(const MetaDataList& v_md)
{
	PlaylistPtr active = active_playlist();

	active->insert_tracks(v_md, active->current_track_index() + 1);
}


void Handler::insert_tracks(const MetaDataList& v_md, int row, int pl_idx)
{
	if(!between(pl_idx, m->playlists)){
		return;
	}

	PlaylistPtr pl = m->playlists[pl_idx];

	bool is_empty = pl->is_empty();
	bool stopped = (m->play_manager->playstate() == PlayState::Stopped);

	pl->insert_tracks(v_md, row);

	if( is_empty &&
		stopped &&
		_settings->get(Set::Lib_DD_PlayIfStoppedAndEmpty))
	{
		this->change_track(0, pl_idx);
	}
}


void Handler::append_tracks(const MetaDataList& v_md, int pl_idx)
{
	if(!between(pl_idx, m->playlists)){
		return;
	}

	m->playlists[pl_idx]->append_tracks(v_md);
}

void Handler::remove_rows(const IndexSet& indexes, int pl_idx)
{
	if(!between(pl_idx, m->playlists)){
		return;
	}

	m->playlists[pl_idx]->delete_tracks(indexes);
}


void Handler::move_rows(const IndexSet& indexes, int tgt_idx, int pl_idx)
{
	if(!between(pl_idx, m->playlists)){
		return;
	}

	m->playlists[pl_idx]->move_tracks(indexes, tgt_idx);
}


QString Handler::request_new_playlist_name() const
{
	return DBInterface::request_new_db_name();
}

void Handler::save_all_playlists()
{
	if(_settings->get(Set::PL_LoadTemporaryPlaylists))
	{
		m->db->transaction();
		for(PlaylistPtr pl : m->playlists){
			if(pl->is_temporary() && pl->was_changed()){
				pl->save();
			}
		}
		m->db->commit();
	}
}


void Handler::close_playlist(int idx)
{
	if(!between(idx, m->playlists)){
		return;
	}

	bool was_active = (idx == m->active_playlist_idx);

	if(m->playlists[idx]->is_temporary()){
		m->playlists[idx]->delete_playlist();
	}

	m->playlists.erase(m->playlists.begin() + idx);

	if( was_active ){
		set_active_idx(0);
	}

	else if(m->active_playlist_idx > idx){
		m->active_playlist_idx --;
	}

	for(PlaylistPtr pl : m->playlists){
		if(pl->index() >= idx){
			pl->set_index(pl->index() - 1);
		}
	}

	if(was_active){
		_settings->set(Set::PL_LastPlaylist, -1);
		_settings->set(Set::PL_LastTrack, -1);
	}
	else{
		_settings->set(Set::PL_LastPlaylist, active_playlist()->get_id());
	}
}

PlaylistConstPtr Handler::playlist(int idx) const
{
	if(! between(idx, m->playlists) ){
		return nullptr;
	}

	return std::const_pointer_cast<const Base>(m->playlists[idx]);
}

PlaylistPtr Handler::playlist(int idx, PlaylistPtr fallback) const
{
	if(! between(idx, m->playlists)){
		return fallback;
	}

	return m->playlists[idx];
}

PlaylistConstPtr Handler::active_playlist() const
{
	return playlist(this->active_index());
}


PlaylistPtr Handler::active_playlist()
{
	if(m->play_manager->playstate() == PlayState::Stopped){
		m->active_playlist_idx = -1;
	}

	// assure we have at least one playlist
	if(m->playlists.size() == 0){
		m->active_playlist_idx = add_new_playlist(request_new_playlist_name(), true, Playlist::Type::Std);
	}

	// assure valid idx
	if( !between(m->active_playlist_idx, m->playlists) )
	{
		if(between(m->current_playlist_idx, m->playlists)){
			m->active_playlist_idx = m->current_playlist_idx;
		}

		else {
			m->active_playlist_idx = 0;
		}
	}

	return m->playlists[m->active_playlist_idx];
}


int	Handler::active_index() const
{
	return m->active_playlist_idx;
}

int Handler::current_index() const
{
	return m->current_playlist_idx;
}

int Handler::exists(const QString& name) const
{
	if( name.isEmpty() &&
			between(m->current_playlist_idx, m->playlists))
	{
		return m->current_playlist_idx;
	}

	for(const PlaylistPtr& pl : m->playlists){
		if(pl->get_name().compare(name, Qt::CaseInsensitive) == 0){
			return pl->index();
		}
	}

	return -1;
}


void Handler::save_playlist_to_file(int pl_idx, const QString& filename, bool relative)
{
	if(!between(pl_idx, m->playlists)){
		return;
	}

	PlaylistPtr pl = m->playlists[pl_idx];
	PlaylistParser::save_m3u_playlist(filename, pl->playlist(), relative);
}


void Handler::reset_playlist(int pl_idx)
{
	if(!between(pl_idx, m->playlists)){
		return;
	}

	DBWrapper* db_connector = new DBWrapper();

	int id = m->playlists[pl_idx]->get_id();

	CustomPlaylist cpl = db_connector->get_playlist_by_id(id);

	clear_playlist(pl_idx);
	create_playlist(cpl);

	delete db_connector; db_connector = nullptr;
}


Playlist::DBInterface::SaveAsAnswer Handler::save_playlist(int idx)
{
	Playlist::DBInterface::SaveAsAnswer ret;

	if( !between(idx, m->playlists) )
	{
		return Playlist::DBInterface::SaveAsAnswer::Error;
	}

	PlaylistPtr pl = m->playlists[idx];

	m->db->transaction();
	ret = pl->save();
	m->db->commit();

	if(!pl->is_temporary()){
		emit sig_saved_playlists_changed();
	}

	return ret;
}


Playlist::DBInterface::SaveAsAnswer Handler::save_playlist_as(int idx, const QString& name, bool force_override)
{
	PlaylistPtr pl = m->playlists[idx];
	Playlist::DBInterface::SaveAsAnswer ret;

	// no empty playlists
	if(name.isEmpty())
	{
		return Playlist::DBInterface::SaveAsAnswer::Error;
	}

	ret = pl->save_as(name, force_override);
	if(ret != Playlist::DBInterface::SaveAsAnswer::Success)
	{
		return ret;
	}

	// for PlaylistChooser
	if(!pl->is_temporary()){
		emit sig_saved_playlists_changed();
	}

	// for GUI
	emit sig_playlist_name_changed(idx);

	return Playlist::DBInterface::SaveAsAnswer::Success;
}


Playlist::DBInterface::SaveAsAnswer Handler::rename_playlist(int pl_idx, const QString& name)
{
	// no empty playlists
	if(name.isEmpty()){
		return DBInterface::SaveAsAnswer::Error;
	}

	// get playlist we want to save
	PlaylistPtr pl = m->playlists[pl_idx];

	DBInterface::SaveAsAnswer ret = pl->rename(name);
	if(ret != DBInterface::SaveAsAnswer::Success){
		return ret;
	}

	emit sig_playlist_name_changed(pl_idx);

	// for PlaylistChooser
	if(!pl->is_temporary()){
		emit sig_saved_playlists_changed();
	}

	return DBInterface::SaveAsAnswer::Success;
}

void Handler::delete_playlist(int idx)
{
	PlaylistPtr pl = m->playlists[idx];
	bool was_temporary = pl->is_temporary();
	bool success = pl->remove_from_db();

	// for PlaylistChooser
	if(success && !was_temporary){
		emit sig_saved_playlists_changed();
	}
}

void Handler::delete_tracks(const IndexSet& rows, Library::TrackDeletionMode deletion_mode)
{
	int idx = current_index();
	if(!between(idx, m->playlists)){
		return;
	}

	PlaylistPtr pl = m->playlists[idx];
	const MetaDataList& tracks = pl->playlist();

	MetaDataList v_md;
	v_md.reserve(tracks.size());

	for(int i : rows)
	{
		if(i >= 0 && i < tracks.count())
		{
			v_md << tracks[i];
		}
	}

	if(v_md.isEmpty()){
		return;
	}

	emit sig_track_deletion_requested(v_md, deletion_mode);
}

void Handler::www_track_finished(const MetaData& md)
{
	PlaylistPtr active_pl = this->active_playlist();
	if(!active_pl){
		return;
	}

	if(_settings->get(Set::Stream_ShowHistory))
	{
		active_pl->insert_track(md, active_pl->current_track_index());
	}
}
