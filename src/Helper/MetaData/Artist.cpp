/* Artist.cpp */

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

#include "Helper/MetaData/Artist.h"
#include "Helper/Logger/Logger.h"
#include "Helper/Language.h"

#include <QHash>
#include <QVariant>

Artist::Artist() :
	LibraryItem(),
	id(-1),
	num_albums(0),
	num_songs(0)
{}

Artist::Artist(const Artist& other) :
	LibraryItem(other),
	CASSIGN(id),
	CASSIGN(name),
	CASSIGN(num_albums),
	CASSIGN(num_songs)
{}

Artist::Artist(Artist&& other) :
	LibraryItem(std::move(other)),
	CMOVE(id),
	CMOVE(name),
	CMOVE(num_albums),
	CMOVE(num_songs)
{}

Artist& Artist::operator =(const Artist& other)
{
	LibraryItem::operator =(other);

	ASSIGN(id);
	ASSIGN(name);
	ASSIGN(num_albums);
	ASSIGN(num_songs);

	return *this;
}

Artist& Artist::operator =(Artist&& other)
{
	LibraryItem::operator =( std::move(other) );

	MOVE(id);
	MOVE(name);
	MOVE(num_albums);
	MOVE(num_songs);

	return *this;
}


Artist::~Artist() {}

QVariant Artist::toVariant(const Artist& artist) {
	QVariant var;
	var.setValue(artist);
	return var;
}


bool Artist::fromVariant(const QVariant& v, Artist& artist) {
	if( !v.canConvert<Artist>() ) return false;
	
	artist = v.value<Artist>();
	return true;
}

void Artist::print() const
{
	sp_log(Log::Info) << id << ": " << name << ": " << num_songs << " Songs, " << num_albums << " Albums";
}


ArtistList::ArtistList() :
    std::vector<Artist>()
{}

ArtistList::~ArtistList()
{}

QString ArtistList::get_major_artist(const QStringList& artists)
{
	QHash<QString, int> map;
	int n_artists = artists.size();

	if(artists.isEmpty()) {
		return "";
	}

	if(n_artists == 1){
		return artists.first().toLower().trimmed();
	}

	for(const QString& artist : artists) {
		QString alower = artist.toLower().trimmed();

		// count appearance of artist
		if( !map.contains(alower) ) {
			map.insert(alower, 1);
		}
		else {
			map[alower] = map.value(alower) + 1;
		};
	}

	// n_appearances have to be at least 2/3 of all appearances
	for(const QString& artist : map.keys()) {
		int n_appearances = map.value(artist);
		if(n_appearances * 3 > n_artists * 2) {
			return artist;
		}

	}

	return Lang::get(Lang::Various);
}

QString ArtistList::get_major_artist() const
{
	QStringList lst;

	for(auto it=this->begin(); it!=this->end(); it++){
		lst << it->name;
	}

	return get_major_artist(lst);
}

bool ArtistList::contains(int artist_id) const
{
	for(auto it=this->begin(); it!=this->end(); it++){
		if(it->id == artist_id){
			return true;
		}
	}

    return false;
}

int ArtistList::count() const
{
    return static_cast<int>(this->size());
}

ArtistList& ArtistList::operator <<(const Artist& artist)
{
    this->push_back(artist);
    return *this;
}

Artist ArtistList::first() const
{
    if(this->empty()){
        return Artist();
    }

    return this->at(0);
}



