/* CoverLocation.cpp */

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

#include "CoverLocation.h"
#include "CoverUtils.h"
#include "CoverFetchManager.h"
#include "LocalCoverSearcher.h"

#include "Utils/Tagging/TaggingCover.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/Album.h"
#include "Utils/MetaData/Artist.h"
#include "Utils/globals.h"
#include "Utils/Logger/Logger.h"

#include "Database/Connector.h"
#include "Database/LibraryDatabase.h"

#include <QDir>
#include <QUrl>
#include <QStringList>
#include <QImage>
#include <QPixmap>
#include <QFileInfo>

using Cover::Location;
using namespace Cover::Fetcher;
using Cover::StringMap;

namespace FileUtils=::Util::File;

static void check_coverpath(const QString& audio_path, const QString& cover_path);

struct Location::Private
{
	QString			search_term;		// Term provided to search engine
	QStringList		search_urls;		// Search url where to fetch covers
	QStringList		search_term_urls;	// Search urls where to fetch cover when using freetext search
	StringMap		all_search_urls;	// key = identifier of coverfetcher, value = search url
	QString			cover_path;		// cover_path path, in .Sayonara, where cover is stored. Ignored if local_paths are not empty
	QString			identifier;		// Some human readable identifier with methods where invokded
	QString			audio_file_source;	// A saved cover from an audio file
	QString			audio_file_target;
	QString			local_path_hint;
	QString			hash;			// A unique identifier, mostly referred to as the cover token

	bool			freetext_search;
	bool			valid;			// valid if CoverLocation object contains a valid download url

	Private() :
		freetext_search(false),
		valid(false)
	{}

	Private(const Private& other) :
		CASSIGN(search_term),
		CASSIGN(search_urls),
		CASSIGN(search_term_urls),
		CASSIGN(all_search_urls),
		CASSIGN(cover_path),
		CASSIGN(identifier),
		CASSIGN(audio_file_source),
		CASSIGN(audio_file_target),
		CASSIGN(local_path_hint),
		CASSIGN(hash),
		CASSIGN(freetext_search),
		CASSIGN(valid)
	{}

	Private& operator=(const Private& other)
	{
		ASSIGN(search_term);
		ASSIGN(search_urls);
		ASSIGN(search_term_urls);
		ASSIGN(all_search_urls);
		ASSIGN(cover_path);
		ASSIGN(identifier);
		ASSIGN(audio_file_source);
		ASSIGN(audio_file_target);
		ASSIGN(local_path_hint);
		ASSIGN(hash);
		ASSIGN(freetext_search);
		ASSIGN(valid);

		return (*this);
	}

	~Private()
	{
		if(FileUtils::exists(audio_file_target))
		{
			FileUtils::delete_files({audio_file_target});
		}
	}
};


void Location::set_valid(bool b)
{
	m->valid = b;
}

void Location::set_identifier(const QString& identifier)
{
	m->identifier = identifier;
}

void Location::set_cover_path(const QString& cover_path)
{
	m->cover_path = cover_path;
}

Location::Location()
{
	qRegisterMetaType<Location>("CoverLocation");

	m = Pimpl::make<Location::Private>();
}

Location::~Location() {}

Location::Location(const Location& other)
{
	m = Pimpl::make<Location::Private>(*(other.m));
}

Location& Location::operator=(const Location& other)
{
	*m = *(other.m);
	return *this;
}


Location Location::invalid_location()
{
	Location cl;

	cl.set_valid(false);
	cl.set_cover_path(::Util::share_path("logo.png"));
	cl.set_search_urls(QStringList());
	cl.set_search_term(QString());
	cl.set_identifier("Invalid location");
	cl.set_audio_file_source(QString(), QString());
	cl.set_local_path_hint(QString());

	return cl;
}


bool Location::is_invalid(const QString& cover_path)
{
	QString path1 = FileUtils::clean_filename(cover_path);
	QString path2 = invalid_location().cover_path();

	return (path1 == path2);
}


Location Location::cover_location(const QString& album_name, const QString& artist_name)
{
	using namespace Cover::Fetcher;
	if(album_name.trimmed().isEmpty() && artist_name.trimmed().isEmpty())
	{
		return invalid_location();
	}

	QString cover_token = Cover::Utils::calc_cover_token(artist_name, album_name);
	QString cover_path = Cover::Utils::cover_directory( cover_token + ".jpg" );
	Fetcher::Manager* cfm = Fetcher::Manager::instance();

	Location ret;

	ret.set_valid(true);
	ret.set_cover_path(cover_path);
	ret.set_hash(cover_token);
	ret.set_search_term(artist_name + " " + album_name);
	ret.set_search_urls(cfm->album_addresses(artist_name, album_name));
	ret.set_identifier("CL:By album: " + album_name + " by " + artist_name);

	return ret;
}

Location Location::cover_location(const QString& album_name, const QStringList& artists)
{
	QString major_artist = ArtistList::get_major_artist(artists);
	return cover_location(album_name, major_artist);
}


void check_coverpath(const QString& audio_path, const QString& cover_path)
{
	if(audio_path.isEmpty() || cover_path.isEmpty())
	{
		return;
	}

	if(Util::File::is_www(audio_path)){
		return;
	}

	QFileInfo fi(cover_path);

	// broken symlink
	if(fi.isSymLink() && !FileUtils::exists(fi.symLinkTarget()))
	{
		Util::File::delete_files({cover_path});
		fi = QFileInfo(cover_path);
	}

	// good symlink
	if(fi.exists() && fi.isSymLink())
	{
		return;
	}

	// create symlink to local path
	QStringList local_paths = Cover::LocalSearcher::cover_paths_from_filename(audio_path);
	if(local_paths.isEmpty())
	{
		return;
	}

	// no symlink, but real file, go back
	if(fi.exists())
	{
		return;
	}

	QString source = local_paths.first();

	QString ext = FileUtils::get_file_extension(source);
	if(ext.contains("jpg", Qt::CaseInsensitive) == false)
	{
		QImage img(source);
		QString jpg_source = source + ".jpg";
		img.save(jpg_source);

		source = jpg_source;
	}

	FileUtils::create_symlink(source, cover_path);
}

// TODO: Clean me up
// TODO: Check for albumID
// TODO: Check for dbid
// TODO: Make this class nicer: e.g. valid(), isInvalidLocation()
Location Location::cover_location(const Album& album)
{
	Location cl;

	{ //setup basic CoverLocation
		if( album.album_artists().size() == 1)
		{
			cl = Location::cover_location(album.name(), album.album_artists().at(0));
		}

		else if(album.artists().size() > 1)
		{
			cl = Location::cover_location(album.name(), album.artists());
		}

		else if(album.artists().size() == 1)
		{
			cl = Location::cover_location(album.name(), album.artists().at(0));
		}

		else
		{
			cl = Location::cover_location(album.name(), "");
		}

		if(!album.cover_download_url().isEmpty())
		{
			cl.set_search_urls({album.cover_download_url()});
		}
	}

	// setup local paths. No audio file source. That may last too long
	{
		DB::Connector* db = DB::Connector::instance();
		DB::LibraryDatabase* lib_db = db->library_db(-1, 0);

		MetaDataList v_md;
		lib_db->getAllTracksByAlbum(album.id, v_md);


		if(!v_md.isEmpty())
		{
			cl.set_local_path_hint(v_md.first().filepath());
			cl.set_audio_file_source(v_md.first().filepath(), cl.cover_path());
		}
	}

	cl.set_search_term(album.name() + " " + ArtistList::get_major_artist(album.artists()));

	return cl;
}

Location Location::cover_location(const Artist& artist)
{
	Location cl = Location::cover_location(artist.name());

	if(!artist.cover_download_url().trimmed().isEmpty())
	{
		cl.set_search_urls({ artist.cover_download_url() });
	}

	cl.set_search_term(artist.name());
	cl.set_identifier("CL:By artist: " + artist.name());

	return cl;
}


Location Location::cover_location(const QString& artist)
{
	if(artist.trimmed().isEmpty()) {
		return invalid_location();
	}

	QString cover_token = QString("artist_") + Cover::Utils::calc_cover_token(artist, "");
	QString cover_path = Cover::Utils::cover_directory(cover_token + ".jpg");
	Fetcher::Manager* cfm = Fetcher::Manager::instance();

	Location ret;

	ret.set_valid(true);
	ret.set_cover_path(cover_path);
	ret.set_search_urls(cfm->artist_addresses(artist));
	ret.set_search_term(artist);
	ret.set_identifier("CL:By artist name: " + artist);
	ret.set_hash(cover_token);

	return ret;
}

Location Get_cover_location(AlbumId album_id, DbId db_id)
{
	if(album_id < 0) {
		return Location::invalid_location();
	}

	DB::Connector* db = DB::Connector::instance();
	DB::LibraryDatabase* lib_db = db->library_db(-1, db_id);
	if(!lib_db){
		return Location();
	}

	Album album;
	bool success = lib_db->getAlbumByID(album_id, album, true);
	if(!success) {
		return Location::invalid_location();
	}

	return Location::cover_location(album);
}

Location Location::cover_location(const MetaData& md)
{
	Location cl;

	if(!md.cover_download_url().isEmpty())
	{
		QString extension = FileUtils::get_file_extension(md.cover_download_url());

		QString cover_token = Cover::Utils::calc_cover_token(md.artist(), md.album());
		QString cover_path = Cover::Utils::cover_directory(cover_token + "." + extension);

		cl = cover_location(QUrl(md.cover_download_url()), cover_path);
	}

	else if(md.album_id >= 0){
		cl = Get_cover_location(md.album_id, md.db_id());
	}

	if(!cl.valid() && !md.album().isEmpty() && !md.artist().isEmpty()){
		cl = cover_location(md.album(), md.artist());
	}

	if(cl.audio_file_source().isEmpty() && !md.filepath().isEmpty() && Tagging::Covers::has_cover(md.filepath())) {
		cl.set_audio_file_source(md.filepath(), cl.cover_path());
	}

	if(cl.search_urls().isEmpty()){
		cl.set_search_urls({md.cover_download_url()});
	}

	cl.set_local_path_hint(md.filepath());
	cl.set_identifier("CL:By metadata: " + md.album() + " by " + md.artist());

	return cl;
}


Location Location::cover_location(const QUrl& url, const QString& target_path)
{
	Location cl;

	cl.set_valid(true);
	cl.set_cover_path(target_path);
	cl.set_search_urls({url.toString()});
	cl.set_identifier("CL:By direct download url: " + url.toString());

	return cl;
}

bool Location::valid() const
{
	return m->valid;
}


QString Location::cover_path() const
{
	return m->cover_path;
}

QString Location::preferred_path() const
{
	// first search for cover in track
	if(has_audio_file_source())
	{
		bool target_exists = FileUtils::exists(this->audio_file_target());
		if(!target_exists)
		{
			if(Tagging::Covers::has_cover(this->audio_file_source()))
			{
				QPixmap pm = Tagging::Covers::extract_cover(this->audio_file_source());
				if(!pm.isNull())
				{
					target_exists = pm.save(this->audio_file_target());
				}
			}
		}

		if(target_exists)
		{
			return this->audio_file_target();
		}
	}

	if(!m->local_path_hint.isEmpty())
	{
		check_coverpath(m->local_path_hint, this->cover_path());
	}

	// return the calculated path
	if(FileUtils::exists(this->cover_path())){
		return this->cover_path();
	}

	return invalid_location().cover_path();
}


QString Location::identifer() const
{
	return m->identifier;
}

const QStringList& Location::search_urls() const
{
	if(m->freetext_search) {
		return m->search_term_urls;
	}

	else {
		return m->search_urls;
	}
}

QString Location::search_url(int idx) const
{
	if(!between(idx, m->search_urls)){
		return QString();
	}

	return m->search_urls.at(idx);
}

bool Location::has_search_urls() const
{
	return !(m->search_urls.isEmpty());
}


QString Location::search_term() const
{
	return m->search_term;
}

void Location::set_search_term(const QString& search_term)
{
	Fetcher::Manager* cfm = Fetcher::Manager::instance();

	m->search_term = search_term;
	m->search_term_urls = cfm->search_addresses(search_term);
}

void Location::set_search_term(const QString& search_term,
							   const QString& cover_fetcher_identifier)
{
	Fetcher::Manager* cfm = Fetcher::Manager::instance();

	m->search_term = search_term;
	m->search_term_urls = cfm->search_addresses(search_term, cover_fetcher_identifier);
}

void Location::set_search_urls(const QStringList &urls)
{
	m->search_urls = urls;
}

void Location::enable_freetext_search(bool b)
{
	m->freetext_search = b;
}

bool Location::is_freetext_search_enabled() const
{
	return m->freetext_search;
}

bool Location::has_audio_file_source() const
{
	return
	(
		(m->audio_file_target.size() > 0) &&
		(m->audio_file_source.size() > 0) &&
		(FileUtils::exists(m->audio_file_source))
	);
}

QString Location::audio_file_source() const
{
	return m->audio_file_source;
}

QString Location::audio_file_target() const
{
	return m->audio_file_target;
}

bool Location::set_audio_file_source(const QString& audio_filepath, const QString& cover_path)
{
	m->audio_file_source = QString();
	m->audio_file_target = QString();

	if(audio_filepath.isEmpty() || cover_path.isEmpty())
	{
		return false;
	}

	QString dir, filename;
	FileUtils::split_filename(cover_path, dir, filename);
	filename.prepend("fromtag_");

	m->audio_file_source = audio_filepath;
	m->audio_file_target = dir + "/" + filename;

	return true;
}

QString Location::local_path_hint() const
{
	return m->local_path_hint;
}

void Location::set_local_path_hint(const QString& base_path)
{
	m->local_path_hint = base_path;
}

QString Location::hash() const
{
	return m->hash;
}

void Location::set_hash(const QString& hash)
{
	m->hash	= hash;
}


QString Location::to_string() const
{
	return	"Cover Location: Valid? " + QString::number(m->valid) + ", "
			"Cover Path: " + cover_path() + ", "
//			"Preferred Path: " + preferred_path() + ", "
			"Search Urls: " + search_urls().join(',') + ", "
			"Search Term: " + search_term() + ", "
											  "Identifier: " + identifer();
}

