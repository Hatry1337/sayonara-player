/* LocalLibrary.h */

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
 */

#ifndef LocalLibrary_H
#define LocalLibrary_H

#include "AbstractLibrary.h"
#include "Helper/Singleton.h"
#include "Helper/Pimpl.h"

class ReloadThread;
class DatabaseConnector;
class FileSystemWatcher;

class LocalLibrary :
		public AbstractLibrary
{
    Q_OBJECT
	PIMPL(LocalLibrary)

signals:
	void sig_no_library_path();
	void sig_import_dialog_requested();

public:
	LocalLibrary(QObject* parent=nullptr);
	virtual ~LocalLibrary();


public slots:
	void delete_tracks(const MetaDataList& v_md, Library::TrackDeletionMode answer) override;
	void psl_reload_library(bool clear_first, Library::ReloadQuality quality) override;

	// emits new tracks, very similar to psl_selected_albums_changed
	void psl_disc_pressed(int);
	void change_track_rating(int idx, int rating) override;

	void refresh_artist() override;
	void refresh_albums() override;
	void refresh_tracks() override;

	void import_files(const QStringList& files) override;
	void merge_artists(const SP::Set<ArtistID>& artist_ids, ArtistID target_artist_id);
	void merge_albums(const SP::Set<AlbumID>& albums_ids, AlbumID target_album_id);


protected slots:
	void library_reloading_state_new_block();
	void reload_thread_finished();
	void _sl_search_mode_changed();
	void _sl_auto_update_changed();
	void indexing_finished();


private:
	void		get_all_artists(ArtistList& artists, Library::Sortings so) override;
	void		get_all_artists_by_searchstring(Library::Filter filter, ArtistList& artists, Library::Sortings so) override;

	void		get_all_albums(AlbumList& albums, Library::Sortings so) override;
	void		get_all_albums_by_artist(IDList artist_ids, AlbumList& albums, Library::Filter filter, Library::Sortings so) override;
	void		get_all_albums_by_searchstring(Library::Filter filter, AlbumList& albums, Library::Sortings so) override;

	void		get_all_tracks(MetaDataList& v_md, Library::Sortings so) override;
	void		get_all_tracks(const QStringList& paths, MetaDataList& v_md) override;
	void		get_all_tracks_by_artist(IDList artist_ids, MetaDataList& v_md, Library::Filter filter, Library::Sortings so) override;
	void		get_all_tracks_by_album(IDList album_ids, MetaDataList& v_md, Library::Filter filter, Library::Sortings so) override;
	void		get_all_tracks_by_searchstring(Library::Filter filter, MetaDataList& v_md, Library::Sortings so) override;

	void		get_album_by_id(int album_id, Album& album) override;
	void		get_artist_by_id(int artist_id, Artist& artist) override;

	void		update_track(const MetaData& md) override;
	void 		update_tracks(const MetaDataList& v_md) override;
	void		update_album(const Album& album) override;

	void		insert_tracks(const MetaDataList& v_md) override;
	void		apply_db_fixes();
	void		init_reload_thread();
};

#endif // LocalLibrary_H
