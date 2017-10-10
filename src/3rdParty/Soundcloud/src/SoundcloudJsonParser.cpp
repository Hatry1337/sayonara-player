/* SoundcloudJsonParser.cpp */

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

#include "SoundcloudGlobal.h"
#include "SoundcloudJsonParser.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Genre.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language.h"

#include <QJsonDocument>
#include <QJsonParseError>


struct SC::JsonParser::Private
{
	QJsonDocument		json_doc;
	QByteArray			content;
	QJsonParseError		error;

	Private(const QByteArray& content) :
		content(content)
	{
		json_doc = QJsonDocument::fromJson(content, &error);
	}
};

SC::JsonParser::JsonParser(const QByteArray& content) :
	QObject()
{
	m = Pimpl::make<Private>(content);
	QString target_file = Util::sayonara_path() + "/tmp.json";

	Util::File::write_file(
		m->json_doc.toJson(QJsonDocument::Indented), target_file
	);

	QJsonParseError::ParseError pe = m->error.error;
	if(pe != QJsonParseError::NoError){
		sp_log(Log::Warning) << "Cannot parse json document: " << m->error.errorString();
	}
}

SC::JsonParser::~JsonParser() {}

bool SC::JsonParser::parse_artists(ArtistList& artists)
{
	if(m->json_doc.isArray()){
		return parse_artist_list(artists, m->json_doc.array());
	}

	else if(m->json_doc.isObject()){
		Artist artist;
		if(parse_artist(artist, m->json_doc.object())){
			artists << artist;
			return true;
		}
	}

	return false;
}


bool SC::JsonParser::parse_artist_list(ArtistList& artists, QJsonArray arr){
	artists.clear();

	for(auto it = arr.begin(); it != arr.end(); it++){
		QJsonValueRef ref = *it;
		if(ref.isObject()){
			Artist artist;
			if(parse_artist(artist, ref.toObject())){
				artists << artist;
			}
		}
	}

	return true;
}


bool SC::JsonParser::parse_artist(Artist& artist, QJsonObject object)
{
	QString cover_download_url;
	get_int("id", object, artist.id);

	QString artist_name;
	get_string("username", object, artist_name);
	artist.set_name(artist_name);

	get_string("avatar_url", object, cover_download_url);
	artist.set_cover_download_url(cover_download_url);

	QString description, website, permalink;
	if(get_string("website", object, website)){
		artist.add_custom_field("website", tr("Website"), website);
	}

	if(get_string("permalink", object, permalink)){
		artist.add_custom_field("permalink", tr("Permalink Url"), permalink);
	}

	if(get_string("description", object, description)){
		artist.add_custom_field("description", Lang::get(Lang::About), description);
	}

	int followers=-1;
	int following=-1;
	get_int("followers_count", object, followers);
	get_int("followings_count", object, following);

	if(followers != -1 && following != -1){
		artist.add_custom_field("followers_following", tr("Followers/Following"), QString::number(followers) + "/" + QString::number(following));
	}

	return (artist.id > 0);
}


bool SC::JsonParser::parse_tracks(ArtistList& artists, MetaDataList &v_md)
{
	if(!m->json_doc.isArray()){
		return false;
	}

	return parse_track_list(artists, v_md, m->json_doc.array());
}


bool SC::JsonParser::parse_track_list(ArtistList& artists, MetaDataList &v_md, QJsonArray arr){
	v_md.clear();

	for(auto it = arr.begin(); it != arr.end(); it++){
		QJsonValueRef ref = *it;
		if(ref.isObject()){
			MetaData md;
			Artist artist;
			if(parse_track(artist, md, ref.toObject())){
				md.track_num = v_md.size() + 1;

				v_md << md;

				if(!artists.contains(artist.id)){
					artists << artist;
				}

			}

			else{
				sp_log(Log::Debug, this) << "Invalid md found";
			}
		}
	}

	return true;
}

bool SC::JsonParser::parse_track(Artist& artist, MetaData& md, QJsonObject object)
{
	QString cover_download_url;
	get_int("id", object, md.id);
	get_string("title", object, md.title);
	get_string("artwork_url", object, cover_download_url);
	md.set_cover_download_url(cover_download_url);


	int length;
	if(get_int("duration", object, length)){
        md.length_ms = (uint32_t) length;
	}

	int year;
	if(get_int("release_year", object, year)){
		md.year = (uint16_t) year;
	}

	int filesize;
	if(get_int("original_content_size", object, filesize)){
         md.filesize = (uint32_t) filesize;
	}

	QString stream_url;
	if(get_string("stream_url", object, stream_url)){
		md.set_filepath(stream_url + '?' + CLIENT_ID_STR);
	}

	QString genre;
	if(get_string("genre", object, genre)){
        md.add_genre(Genre(genre));
	}

	QString purchase_url;
	if(get_string("purchase_url", object, purchase_url)){
		md.add_custom_field("purchase_url", tr("Purchase Url"), create_link(purchase_url, purchase_url));
	}

	QJsonObject artist_object;
	if(get_object("user", object, artist_object)){
		if( parse_artist(artist, artist_object) ){
			md.set_artist(artist.name());
			md.artist_id = artist.id;

			if(md.album_id < 0){
				md.album_id = 0;
				md.set_album(Lang::get(Lang::None));
			}
		}
	}

	return (md.filepath().size() > 0 && md.id > 0);
}


bool SC::JsonParser::parse_playlists(ArtistList& artists, AlbumList &albums, MetaDataList &v_md)
{
	if(m->json_doc.isArray()){
		return parse_playlist_list(artists, albums, v_md, m->json_doc.array());
	}

	else if(m->json_doc.isObject()){
		Album album;
		if(parse_playlist(artists, album, v_md, m->json_doc.object())){
			albums << album;
			return true;
		}
	}

	return false;
}


bool SC::JsonParser::parse_playlist_list(ArtistList& artists, AlbumList& albums, MetaDataList& v_md, QJsonArray arr)
{
	albums.clear();

	for(auto it = arr.begin(); it != arr.end(); it++){
		QJsonValueRef ref = *it;
		if(ref.isObject()){
			Album album;
			MetaDataList v_md_tmp;
			ArtistList artists_tmp;

			if(parse_playlist(artists_tmp, album, v_md_tmp, ref.toObject())){
				v_md << v_md_tmp;

				for(const Artist& artist_tmp : artists_tmp){
					if(!artists.contains(artist_tmp.id) && artist_tmp.id > 0){
						artists << artist_tmp;
					}
				}

				if(!albums.contains(album.id)){
					albums << album;
				}
			}
		}
	}

	return true;
}


bool SC::JsonParser::parse_playlist(ArtistList& artists, Album& album, MetaDataList& v_md, QJsonObject object)
{
	Artist pl_artist;
	QString cover_download_url;

	get_int("id", object, album.id);

	QString album_name;
	get_string("title", object, album_name);
	album.set_name(album_name);

	get_string("artwork_url", object, cover_download_url);
	album.set_cover_download_url(cover_download_url);

	int num_songs;
	if(get_int("track_count", object, num_songs)){
		album.num_songs = num_songs;
	}

	int length;
	if(get_int("duration", object, length)){
		album.length_sec = length / 1000;
	}

	QJsonObject artist_object;
	if(get_object("user", object, artist_object)){
		parse_artist(pl_artist, artist_object);
		if(!artists.contains(pl_artist.id) && pl_artist.id > 0){
			artists << pl_artist;
		}
	}

	QJsonArray track_array;
	if(get_array("tracks", object, track_array)){
		ArtistList tmp_artists;
		MetaDataList v_md_tmp;
		parse_track_list(tmp_artists, v_md_tmp, track_array);
		for(const Artist& tmp_artist : tmp_artists){
			if(!artists.contains(tmp_artist.id)){
				artists << tmp_artist;
			}
		}

		for(const MetaData& md : v_md_tmp){
			if(!v_md.contains(md.id)){
				v_md << md;
			}
		}
	}

	QString permalink, purchase_url;
	if(get_string("permalink", object, permalink)){
		album.add_custom_field(permalink, tr("Permalink Url"), create_link("Soundcloud", permalink));
	}

	if(get_string("purchase_url", object, purchase_url)){
		album.add_custom_field(purchase_url, tr("Purchase Url"), create_link(purchase_url, purchase_url));
	}

	album_name = album.name();

	for(int i=0; i<v_md.count(); i++)
	{
		MetaData& md = v_md[i];
		md.track_num = i+1;
		md.set_album(album.name());
		md.album_id = album.id;

		if(md.artist_id != pl_artist.id && pl_artist.id > 0 && md.artist_id > 0)
		{
			md.set_album( md.album() + " (by " + pl_artist.name() + ")");
			album_name = album.name() + " (by " + pl_artist.name() + ")";
		}

		if(!album.cover_download_url().isEmpty()){
			v_md[i].set_cover_download_url(album.cover_download_url());
		}
	}

	album.set_name(album_name);

	QStringList lst;
	for(const Artist& artist : artists){
		lst << artist.name();
	}

	album.set_artists(lst);

	return (album.id > 0);
}


QString SC::JsonParser::create_link(const QString& name, const QString& target)
{
	bool dark = (Settings::instance()->get(Set::Player_Style) == 1);
	return Util::create_link(name, dark, target);
}


bool SC::JsonParser::get_string(const QString& key, const QJsonObject& object, QString& str)
{
	auto it = object.find(key);
	if(it != object.end()){
		QJsonValue ref = *it;
		if(ref.isString()){
			str = ref.toString();
			str.replace("\\n", "<br />");
			str.replace("\\\"", "\"");
			str = str.trimmed();
			return true;
		}
	}

	return false;
}

bool SC::JsonParser::get_int(const QString& key, const QJsonObject& object, int& i)
{
	auto it = object.find(key);
	if(it != object.end()){
		QJsonValue ref = *it;
		if(ref.isDouble()){
			i = ref.toInt();
			return true;
		}
	}

	return false;
}


bool SC::JsonParser::get_array(const QString& key, const QJsonObject& object, QJsonArray& arr)
{
	auto it = object.find(key);
	if(it != object.end()){
		QJsonValue ref = *it;
		if(ref.isArray()){
			arr = ref.toArray();
			return true;
		}
	}

	return false;
}

bool SC::JsonParser::get_object(const QString& key, const QJsonObject& object, QJsonObject& o)
{
	auto it = object.find(key);
	if(it != object.end()){
		QJsonValue ref = *it;
		if(ref.isObject()){
			o = ref.toObject();
			return true;
		}
	}

	return false;
}
