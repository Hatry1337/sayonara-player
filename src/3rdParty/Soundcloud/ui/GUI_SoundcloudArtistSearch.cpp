/* GUI_SoundcloudArtistSearch.cpp */

/* Copyright (C) 2011-2016  Lucio Carreras
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



#include "GUI_SoundcloudArtistSearch.h"


GUI_SoundcloudArtistSearch::GUI_SoundcloudArtistSearch(SoundcloudLibrary* library, QWidget *parent) :
	SayonaraDialog(parent),
	Ui::GUI_SoundcloudArtistSearch()
{
	setupUi(this);

	_library = library;
	_fetcher = new SoundcloudDataFetcher(this);

	connect(btn_search, &QPushButton::clicked, this, &GUI_SoundcloudArtistSearch::search_clicked);
	connect(btn_add, &QPushButton::clicked, this, &GUI_SoundcloudArtistSearch::add_clicked);
	connect(btn_cancel, &QPushButton::clicked, this, &GUI_SoundcloudArtistSearch::close);
	connect(btn_clear, &QPushButton::clicked, this, &GUI_SoundcloudArtistSearch::clear_clicked);

	connect(list_artists, &QListWidget::currentRowChanged, this, &GUI_SoundcloudArtistSearch::artist_selected);

	connect(_fetcher, &SoundcloudDataFetcher::sig_artists_fetched, this, &GUI_SoundcloudArtistSearch::artists_fetched);
	connect(_fetcher, &SoundcloudDataFetcher::sig_ext_artists_fetched, this, &GUI_SoundcloudArtistSearch::artists_ext_fetched);
	connect(_fetcher, &SoundcloudDataFetcher::sig_playlists_fetched, this, &GUI_SoundcloudArtistSearch::albums_fetched);
	connect(_fetcher, &SoundcloudDataFetcher::sig_tracks_fetched, this, &GUI_SoundcloudArtistSearch::tracks_fetched);

	clear_clicked();
}

void GUI_SoundcloudArtistSearch::search_clicked(){

	QString text = le_search->text();
	clear_clicked();

	le_search->setText(text);

	if(text.size() <= 3){
		lab_status->setText(tr("Query to short"));
	}

	_fetcher->search_artists(text);
}

void GUI_SoundcloudArtistSearch::clear_clicked(){

	list_artists->clear();
	list_playlists->clear();
	list_tracks->clear();

	le_search->clear();

	_v_md.clear();
	_artists.clear();
	_albums.clear();

	lab_status->clear();

	lab_n_artists->clear();
	lab_n_playlists->clear();
	lab_n_tracks->clear();

	btn_add->setEnabled(false);
}


void GUI_SoundcloudArtistSearch::add_clicked(){

	if(_v_md.size() > 0 && _artists.size() > 0 && _albums.size() > 0){
		_library->insert_tracks(_v_md, _artists, _albums);
		close();
	}
}

void GUI_SoundcloudArtistSearch::close_clicked(){
	close();
}


void GUI_SoundcloudArtistSearch::artist_selected(int idx){

	list_playlists->clear();
	list_tracks->clear();

	_v_md.clear();
	_albums.clear();

	if(!between(idx, 0, _artists.size())) {
		return;
	}

	_cur_artist_sc_id = _artists[idx].id;

	_artists.clear();
	_fetcher->get_tracks_by_artist(_cur_artist_sc_id);
}

void GUI_SoundcloudArtistSearch::language_changed()
{
	retranslateUi(this);
}


void GUI_SoundcloudArtistSearch::artists_fetched(const ArtistList& artists){

	list_artists->clear();


	if(artists.size() == 0){
		lab_status->setText(tr("No artists found"));
		return;
	}

	else{
		lab_n_artists->setText( tr("Found %1 artists").arg(artists.size()) );
		for(const Artist& artist: artists){
			list_artists->addItem(artist.name);
		}

		_artists = artists;
	}
}

void GUI_SoundcloudArtistSearch::artists_ext_fetched(const ArtistList &artists){
	_artists = artists;
}


void GUI_SoundcloudArtistSearch::albums_fetched(const AlbumList& albums){

	list_playlists->clear();

	for(const Album& album : albums){
		list_playlists->addItem(album.name);
	}

	_albums = albums;

	lab_n_playlists->setText( tr("%1 playlists found").arg(albums.size()) );
}


void GUI_SoundcloudArtistSearch::tracks_fetched(const MetaDataList& v_md){

	list_tracks->clear();

	for(const MetaData& md : v_md){
		list_tracks->addItem(md.title);
	}

	lab_n_tracks->setText( tr("%1 tracks found").arg(v_md.size()) );
	_v_md = v_md;

	btn_add->setEnabled(v_md.size() > 0);
}
