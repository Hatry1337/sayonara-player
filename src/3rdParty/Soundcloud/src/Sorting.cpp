#include "Sorting.h"
#include "Helper/MetaData/MetaData.h"
#include "Helper/MetaData/Artist.h"
#include "Helper/MetaData/Album.h"
#include "Helper/MetaData/MetaDataList.h"

#include <functional>

namespace Compare
{
	bool artistNameAsc(const Artist& artist1, const Artist& artist2) {
		return (artist1.name() < artist2.name());
	}

	bool artistNameDesc(const Artist& artist1, const Artist& artist2) {
		return artistNameAsc(artist2, artist1);
	}

	bool artistTrackcountAsc(const Artist& artist1, const Artist& artist2) {
		return (artist1.num_songs < artist2.num_songs);
	}

	bool artistTrackcountDesc(const Artist& artist1, const Artist& artist2) {
		return artistTrackcountAsc(artist2, artist1);
	}

	bool albumNameAsc(const Album& album1, const Album& album2) {
		return (album1.name() < album2.name());
	}

	bool albumNameDesc(const Album& album1, const Album& album2) {
		return albumNameAsc(album2, album1);
	}

	bool albumYearAsc(const Album& album1, const Album& album2) {
		return (album1.year < album2.year);
	}

	bool albumYearDesc(const Album& album1, const Album& album2) {
		return albumYearAsc(album2, album1);
	}

	bool albumRatingAsc(const Album& album1, const Album& album2) {
		return (album1.rating < album2.rating);
	}

	bool albumRatingDesc(const Album& album1, const Album& album2) {
		return albumRatingAsc(album2, album1);
	}

	bool albumDurationAsc(const Album& album1, const Album& album2) {
		return (album1.length_sec < album2.length_sec);
	}

	bool albumDurationDesc(const Album& album1, const Album& album2) {
		return albumDurationAsc(album2, album1);
	}

	bool trackTitleAsc(const MetaData& md1, const MetaData& md2){
		return (md1.title < md2.title);
	}

	bool trackTitleDesc(const MetaData& md1, const MetaData& md2){
		return trackTitleAsc(md2, md1);
	}

	bool trackNumAsc(const MetaData& md1, const MetaData& md2){
		return (md1.track_num < md2.track_num);
	}

	bool trackNumDesc(const MetaData& md1, const MetaData& md2){
		return trackNumAsc(md2, md1);
	}

	bool trackAlbumAsc(const MetaData& md1, const MetaData& md2){
		if(md1.album() == md2.album()){
			return trackNumAsc(md1, md2);
		}

		return (md1.album() < md2.album());
	}

	bool trackAlbumDesc(const MetaData& md1, const MetaData& md2){
		return trackAlbumAsc(md2, md1);
	}

	bool trackArtistAsc(const MetaData& md1, const MetaData& md2){
		if(md1.artist() == md2.artist()){
			return trackAlbumAsc(md1, md2);
		}

		return (md1.artist() < md2.artist());
	}

	bool trackArtistDesc(const MetaData& md1, const MetaData& md2){
		return trackArtistAsc(md2, md1);
	}

	bool trackYearAsc(const MetaData& md1, const MetaData& md2){
		if(md1.year == md2.year){
			return trackArtistAsc(md1, md2);
		}

		return (md1.year < md2.year);
	}

	bool trackYearDesc(const MetaData& md1, const MetaData& md2){
		return trackYearAsc(md2, md1);
	}

	bool trackLengthAsc(const MetaData& md1, const MetaData& md2){
		return (md1.length_ms < md2.length_ms);
	}

	bool trackLengthDesc(const MetaData& md1, const MetaData& md2){
		return trackLengthAsc(md2, md1);
	}

	bool trackBitrateAsc(const MetaData& md1, const MetaData& md2){
		if(md1.bitrate == md2.bitrate){
			return trackArtistAsc(md1, md2);
		}

		return (md1.bitrate < md2.bitrate);
	}

	bool trackBitrateDesc(const MetaData& md1, const MetaData& md2){
		return trackBitrateAsc(md2, md1);
	}
}



void SC::Sorting::sort_artists(ArtistList& artists, Library::SortOrder so)
{
	std::function<bool (const Artist& a1, const Artist& a2)> fn;
	switch(so) {
		case Library::SortOrder::ArtistNameAsc:
			fn = Compare::artistNameAsc;
			break;

		case Library::SortOrder::ArtistNameDesc:
			fn = Compare::artistNameDesc;
			break;

		case Library::SortOrder::ArtistTrackcountAsc:
			fn = Compare::artistTrackcountAsc;
			break;

		case Library::SortOrder::ArtistTrackcountDesc:
			fn = Compare::artistTrackcountDesc;
			break;

		default:
			return;
	}

	std::sort(artists.begin(), artists.end(), fn);
}

void SC::Sorting::sort_albums(AlbumList& albums, Library::SortOrder so)
{
	std::function<bool (const Album& a1, const Album& a2)> fn;
	switch(so) 
	{
		case Library::SortOrder::AlbumNameAsc:
			fn = Compare::albumNameAsc;
			break;

		case Library::SortOrder::AlbumNameDesc:
			fn = Compare::albumNameDesc;
			break;

		case Library::SortOrder::AlbumYearAsc:
			fn = Compare::albumYearAsc;
			break;

		case Library::SortOrder::AlbumYearDesc:
			fn = Compare::albumYearDesc;
			break;

		case Library::SortOrder::AlbumDurationAsc:
			fn = Compare::albumDurationAsc;
			break;

		case Library::SortOrder::AlbumDurationDesc:
			fn = Compare::albumDurationDesc;
			break;

		default:
			return;
	}

	std::sort(albums.begin(), albums.end(), fn);
}

void SC::Sorting::sort_tracks(MetaDataList& v_md, Library::SortOrder so)
{
	std::function<bool (const MetaData& md1, const MetaData& md2)> fn;

	switch(so)
	{
		case Library::SortOrder::TrackNumAsc:
			fn = Compare::trackNumAsc;
			break;

		case Library::SortOrder::TrackNumDesc:
			fn = Compare::trackNumDesc;
			break;

		case Library::SortOrder::TrackTitleAsc:
			fn = Compare::trackTitleAsc;
			break;

		case Library::SortOrder::TrackTitleDesc:
			fn = Compare::trackTitleDesc;
			break;

		case Library::SortOrder::TrackAlbumAsc:
			fn = Compare::trackAlbumAsc;
			break;

		case Library::SortOrder::TrackAlbumDesc:
			fn = Compare::trackAlbumDesc;
			break;

		case Library::SortOrder::TrackArtistAsc:
			fn = Compare::trackArtistAsc;
			break;

		case Library::SortOrder::TrackArtistDesc:
			fn = Compare::trackArtistDesc;
			break;

		case Library::SortOrder::TrackYearAsc:
			fn = Compare::trackYearAsc;
			break;

		case Library::SortOrder::TrackYearDesc:
			fn = Compare::trackYearDesc;
			break;

		case Library::SortOrder::TrackLenghtAsc:
			fn = Compare::trackLengthAsc;
			break;

		case Library::SortOrder::TrackLengthDesc:
			fn = Compare::trackLengthDesc;
			break;


		case Library::SortOrder::TrackBitrateAsc:
			fn = Compare::trackBitrateAsc;
			break;

		case Library::SortOrder::TrackBitrateDesc:
			fn = Compare::trackBitrateDesc;
			break;

		default:
			return;
	}

	std::sort(v_md.begin(), v_md.end(), fn);
}
