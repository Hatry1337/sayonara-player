/* AlbumCoverFetchThread.cpp */

/* Copyright (C) 2011-2020 Michael Lugmair (Lucio Carreras)
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

#include "AlbumCoverFetchThread.h"
#include "Utils/MetaData/Album.h"
#include "Components/Covers/CoverLookup.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Covers/CoverLookup.h"

#include "Utils/Utils.h"
#include "Utils/Set.h"
#include "Utils/Mutex.h"
#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/FileUtils.h"

#include <QFile>
#include <QFileInfo>
#include <QSqlDatabase>

using Cover::Location;
using Cover::Lookup;
using Library::AlbumCoverFetchThread;
using Hash=AlbumCoverFetchThread::Hash;
using AtomicBool=std::atomic<bool>;
using AtomicInt=std::atomic<int>;

namespace Algorithm=Util::Algorithm;
namespace FileUtils=::Util::File;
static const int MaxThreads=20;

struct AlbumCoverFetchThread::Private
{
	AlbumCoverFetchThread::HashAlbumList hashAlbumList;
	QList<HashLocationPair> lookups;

	QStringList			queuedHashes;

	std::mutex mutexAlbumList;
	std::mutex mutexQueuedHashes;
	std::mutex mutexLookup;

	AtomicInt	pausedToGo;
	AtomicInt	done;
	AtomicBool	stopped;
	AtomicBool	inPausedState;

	Private()
	{
		init();
	}

	void init()
	{
		done = 0;
		stopped = false;
		hashAlbumList.clear();
		inPausedState = false;
		pausedToGo = 0;
	}

	void pause(int ms = 10)
	{
		pausedToGo = std::min<int>(pausedToGo + ms, 70);
	}

	void wait()
	{
		auto ms = std::min<int>(20, pausedToGo);
		Util::sleepMs(ms);
		pausedToGo -= ms;
	}

	bool may_run()
	{
		if(stopped){
			inPausedState = true;
			return false;
		}

		if(queuedHashes.count() >= MaxThreads) {
			inPausedState = true;
			wait();
		};

		if(pausedToGo > 0) {
			inPausedState = true;
			wait();
		}

		else {
			inPausedState = false;
			return true;
		}

		return false;
	}
};

AlbumCoverFetchThread::AlbumCoverFetchThread(QObject* parent) :
	QThread(parent)
{
	m = Pimpl::make<Private>();
}

AlbumCoverFetchThread::~AlbumCoverFetchThread() = default;

void AlbumCoverFetchThread::run()
{
	m->init();

	while(!m->stopped)
	{
		if(!m->may_run()){
			continue;
		}

		QList<HashAlbumPair> haps;
		{
			LOCK_GUARD(m->mutexAlbumList)
			haps = m->hashAlbumList;
		}
		
		if(haps.isEmpty())
		{
			m->pause();
			continue;	
		}

		for(const HashAlbumPair& hap : haps)
		{
			QString hash = hap.first;
			Album album = hap.second;
			Cover::Location cl = Cover::Location::xcoverLocation(album);
			{
				LOCK_GUARD(m->mutexLookup);
				m->lookups << HashLocationPair(hash, cl);
			}

			emit sigNext();
		}
		{
			LOCK_GUARD(m->mutexAlbumList)
			m->hashAlbumList.clear();
		}
	}
}

void AlbumCoverFetchThread::addAlbum(const Album& album)
{
	if(m->stopped){
		spLog(Log::Develop, this) << "Currently inactive";
		return;
	}

	m->pause();

	QString hash = getHash(album);
	if(checkAlbum(hash)){
		spLog(Log::Develop, this) << "Already processing " << hash;
		return;
	}

	LOCK_GUARD(m->mutexAlbumList)
	m->hashAlbumList.push_front(HashAlbumPair(hash, album));
}

bool AlbumCoverFetchThread::checkAlbum(const QString& hash)
{
	bool has_hash = false;
	{
		LOCK_GUARD(m->mutexLookup)
		has_hash = Algorithm::contains(m->lookups, [hash](const HashLocationPair& p){
			return (p.first == hash);
		});
	}

	if(has_hash){
		spLog(Log::Crazy, this) << "Cover " << hash << " already in lookups";
		emit sigNext();
		return true;
	}

	{
		LOCK_GUARD(m->mutexQueuedHashes)
		if(m->queuedHashes.contains(hash)){
			spLog(Log::Crazy, this) << "Cover " << hash << " already in queued hashes";
			return true;
		}
	}

	{
		LOCK_GUARD(m->mutexAlbumList)
		has_hash = Algorithm::contains(m->hashAlbumList, [hash](const HashAlbumPair& p){
			return (p.first == hash);
		});
	}

	if(has_hash){
		spLog(Log::Crazy, this) << "Cover " << hash << " already in hash_album_list";
	}

	return has_hash;
}

int AlbumCoverFetchThread::lookupsReady() const
{
	return m->lookups.size();
}

int AlbumCoverFetchThread::queuedHashes() const
{
	return m->queuedHashes.size();
}

int AlbumCoverFetchThread::unprocessedHashes() const
{
	return m->hashAlbumList.size();
}

AlbumCoverFetchThread::HashLocationPair AlbumCoverFetchThread::takeCurrentLookup()
{
	HashLocationPair ret;

	{
		LOCK_GUARD(m->mutexLookup)
		if(!m->lookups.isEmpty()){
			ret = m->lookups.takeLast();
		}
	}

	{
		LOCK_GUARD(m->mutexQueuedHashes)
		m->queuedHashes.push_back(ret.first);
	}

	return ret;

}

void AlbumCoverFetchThread::done(const AlbumCoverFetchThread::Hash& hash)
{
	{
		LOCK_GUARD(m->mutexQueuedHashes)
		m->queuedHashes.removeAll(hash);
	}

	{
		LOCK_GUARD(m->mutexLookup)
		for(int i=m->lookups.size() - 1; i>=0; i--)
		{
			if(m->lookups[i].first == hash){
				m->lookups.removeAt(i);
			}
		}
	}
}

AlbumCoverFetchThread::Hash AlbumCoverFetchThread::getHash(const Album& album)
{
	return album.name() + "-" + QString::number(album.id());
}

void AlbumCoverFetchThread::pause()
{
	m->pause();
}

void AlbumCoverFetchThread::stop()
{
	m->stopped = true;
}

void AlbumCoverFetchThread::resume()
{
	m->pausedToGo = 0;
}

void AlbumCoverFetchThread::clear()
{
	{
		LOCK_GUARD(m->mutexAlbumList)
		m->hashAlbumList.clear();
	}

	{
		LOCK_GUARD(m->mutexLookup)
		m->lookups.clear();
	}
}


