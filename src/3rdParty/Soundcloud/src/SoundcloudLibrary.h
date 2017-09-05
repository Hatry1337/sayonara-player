/* SoundcloudLibrary.h */

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

#ifndef SOUNDCLOUD_H
#define SOUNDCLOUD_H

#include "SoundcloudData.h"
#include "Components/Library/AbstractLibrary.h"
#include "Helper/Pimpl.h"
#include "Helper/Library/Sorting.h"

class CoverLocation;

namespace SC
{
	class Library :
		public AbstractLibrary
	{
		Q_OBJECT
		PIMPL(Library)

	signals:
		// called, when webservice returns artists/albums/tracks
		void sig_artists_found(const ArtistList& artists);
		void sig_albums_found(const AlbumList& albums);
		void sig_tracks_found(const MetaDataList& v_md);

	public:
		explicit Library(QObject *parent=nullptr);
		~Library();

		void	load() override;
		void	search_artist(const QString& artist_name);
		void	fetch_tracks_by_artist(int64_t artist_sc_id);
		void	fetch_playlists_by_artist(int64_t artist_sc_id);
		void	insert_tracks(const MetaDataList& v_md) override;
		void	insert_tracks(const MetaDataList& v_md, const ArtistList& artists, const AlbumList& albums);
		void	get_album_by_id(int album_id, Album& album) override;
		void  	get_artist_by_id(int artist_id, Artist& artist) override;


	protected:
		void	get_all_artists(ArtistList& artists, ::Library::Sortings so=::Library::Sortings()) override;
		void	get_all_artists_by_searchstring(::Library::Filter filter, ArtistList& artists, ::Library::Sortings so) override;

		void	get_all_albums(AlbumList& albums, ::Library::Sortings so) override;
		void	get_all_albums_by_artist(IDList artist_ids, AlbumList& albums, ::Library::Filter filter, ::Library::Sortings so) override;
		void	get_all_albums_by_searchstring(::Library::Filter filter, AlbumList& albums, ::Library::Sortings so) override;

		void	get_all_tracks(const QStringList& paths, MetaDataList& v_md) override;
		void	get_all_tracks(MetaDataList& v_md, ::Library::Sortings so) override;
		void	get_all_tracks_by_artist(IDList artist_ids, MetaDataList& v_md, ::Library::Filter filter, ::Library::Sortings so) override;
		void	get_all_tracks_by_album(IDList album_ids, MetaDataList& v_md, ::Library::Filter filter, ::Library::Sortings so) override;
		void	get_all_tracks_by_searchstring(::Library::Filter filter, MetaDataList& v_md, ::Library::Sortings so) override;

		void	update_track(const MetaData& md) override;
		void	update_album(const Album& album) override;
		void	delete_tracks(const MetaDataList& v_md, ::Library::TrackDeletionMode mode) override;

		void	apply_artist_and_album_to_md();


	private slots:
		void	artists_fetched(const ArtistList& artists);
		void	tracks_fetched(const MetaDataList& v_md);
		void	albums_fetched(const AlbumList& albums);
		void	cover_found(const CoverLocation& cl);

	public slots:
		void	psl_reload_library(bool clear_first, ::Library::ReloadQuality quality) override;
		void	refresh_artist() override;
		void	refresh_albums() override;
		void	refresh_tracks() override;
	};
}

#endif // LocalLibrary_H
