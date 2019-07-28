/* Manager.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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

#include "CoverFetchManager.h"
#include "CoverFetcherInterface.h"

#include "GoogleCoverFetcher.h"
#include "LFMCoverFetcher.h"
#include "StandardCoverFetcher.h"
#include "DiscogsCoverFetcher.h"
#include "AllMusicCoverFetcher.h"
#include "AmazonCoverFetcher.h"

#include "Utils/Algorithm.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Settings/SettingNotifier.h"
#include "Utils/Logger/Logger.h"

#include <QStringList>
#include <QList>
#include <QMap>

namespace Algorithm=Util::Algorithm;
using namespace Cover;
using Cover::Fetcher::Manager;
using Cover::Fetcher::Base;
using Cover::Fetcher::FetchUrl;

using SortMap=QMap<QString, int>;

static void sort_coverfetchers(QList<Fetcher::Base*>& lst, const SortMap& cf_order)
{
	Algorithm::sort(lst, [&cf_order](Fetcher::Base* t1, Fetcher::Base* t2)
	{
		int order1 = cf_order[t1->identifier()];
		int order2 = cf_order[t2->identifier()];
		if(order1 != order2) {
			if(order1 == -1){
				return false; // order1 is worse
			}

			if(order2 == -1){
				return true; // order1 is better
			}

			return (order1 < order2);
		}

		int rating1 = t1->estimated_size();
		int rating2 = t2->estimated_size();

		return (rating1 > rating2);
	});
}

static SortMap create_sortmap(const QStringList& lst)
{
	SortMap ret;

	for(int i=0; i<lst.size(); i++)
	{
		QString str = lst[i];
		ret[str] = i;
	}

	return ret;
}


static Fetcher::Base* coverfetcher_by_identifier(const QString& identifier, const QList<Fetcher::Base*>& container)
{
	if(identifier.isEmpty()){
		return nullptr;
	}

	for(Fetcher::Base* cfi : container)
	{
		QString cfi_identifier = cfi->identifier();
		if(!cfi_identifier.isEmpty())
		{
			if(cfi_identifier.compare(identifier, Qt::CaseInsensitive) == 0){
				return cfi;
			}
		}
	}

	return nullptr;
}

static Fetcher::Base* coverfetcher_by_url(const QString& url, const QList<Fetcher::Base*>& container)
{
	if(url.isEmpty()){
		return nullptr;
	}

	for(Fetcher::Base* cfi : container)
	{
		QString identifier = cfi->identifier();
		if(!identifier.isEmpty()){
			if(url.contains(identifier, Qt::CaseInsensitive)){
				return cfi;
			}
		}
	}

	return nullptr;
}

struct Manager::Private
{
	QMap<QString, int>		cf_order;
	QMap<QString, bool>		active_map;
	QList<Fetcher::Base*>	coverfetchers;
	Fetcher::Standard*		std_cover_fetcher = nullptr;

	Private()
	{
		std_cover_fetcher = new Standard();
	}

	~Private()
	{
		std_cover_fetcher = nullptr;

		for(auto it=coverfetchers.begin(); it != coverfetchers.end(); it++)
		{
			Fetcher::Base* b = *it;
			delete b;
		}

		coverfetchers.clear();
	}

	void set_active(QString identifier, bool enabled)
	{
		active_map[identifier.toLower()] = enabled;
	}

	bool is_active(QString identifier) const
	{
		identifier = identifier.toLower();
		if(!active_map.keys().contains(identifier)){
			return false;
		}

		return active_map[identifier];
	}
};


Manager::Manager() :
	QObject()
{
	m = Pimpl::make<Private>();

	register_coverfetcher(new Fetcher::LastFM());
	register_coverfetcher(new Fetcher::Discogs());
	register_coverfetcher(new Fetcher::Google());
	register_coverfetcher(new Fetcher::AllMusicCoverFetcher());
	register_coverfetcher(new Fetcher::Amazon());

	register_coverfetcher(m->std_cover_fetcher);

	ListenSetting(Set::Cover_Server, Manager::servers_changed);
}

Manager::~Manager() = default;

void Manager::register_coverfetcher(Base* t)
{
	Fetcher::Base* cfi = coverfetcher_by_identifier(t->identifier(), m->coverfetchers);
	if(cfi){ // already there
		return;
	}

	m->set_active(t->identifier(), true);
	m->coverfetchers << t;
}


Fetcher::Base* Manager::coverfetcher(const QString& url) const
{
	Fetcher::Base* cfi = coverfetcher_by_url(url, m->coverfetchers);
	if(!cfi || !is_active(cfi)){
		return m->std_cover_fetcher;
	}

	return cfi;
}


QList<Fetcher::Base*> Manager::coverfetchers() const
{
	return m->coverfetchers;
}

QList<Fetcher::Base*> Manager::active_coverfetchers() const
{
	QList<Fetcher::Base*> ret;
	for(Fetcher::Base* cfi : m->coverfetchers)
	{
		if(is_active(cfi)){
			ret << cfi;
		}
	}

	return ret;
}

QList<Fetcher::Base*> Manager::inactive_coverfetchers() const
{
	QList<Fetcher::Base*> ret;
	for(Fetcher::Base* cfi : m->coverfetchers)
	{
		if(!is_active(cfi)){
			ret << cfi;
		}
	}

	return ret;
}

bool Manager::is_active(const Fetcher::Base* cfi) const
{
	return is_active(cfi->identifier());
}

bool Manager::is_active(const QString& identifier) const
{
	return m->is_active(identifier);
}

QString Manager::identifier_by_url(const QString& url) const
{
	Fetcher::Base* cfi = coverfetcher(url);
	if(cfi && is_active(cfi)){
		return cfi->identifier();
	}

	return QString();
}


void Manager::servers_changed()
{
	QStringList servers = GetSetting(Set::Cover_Server);

	for(const QString& key : m->active_map.keys()){
		m->active_map[key] = servers.contains(key);
	}

	SortMap sortmap = create_sortmap(servers);
	sort_coverfetchers(m->coverfetchers, sortmap);
}


QList<FetchUrl> Manager::artist_addresses(const QString& artist, bool also_inactive) const
{
	QList<FetchUrl> urls;

	for(const Fetcher::Base* cfi : Algorithm::AsConst(m->coverfetchers))
	{
		if(cfi->is_artist_supported())
		{
			FetchUrl url
			(
				is_active(cfi->identifier()),
				cfi->identifier(),
				cfi->artist_address(artist)
			);

			if(url.active || also_inactive)
			{
				urls << url;
			}
		}
	}

	return urls;
}


QList<FetchUrl> Manager::album_addresses(const QString& artist, const QString& album, bool also_inactive) const
{
	QList<FetchUrl> urls;

	for(const Fetcher::Base* cfi : Algorithm::AsConst(m->coverfetchers))
	{
		if(cfi->is_album_supported())
		{
			FetchUrl url
			(
				is_active(cfi->identifier()),
				cfi->identifier(),
				cfi->album_address(artist, album)
			);

			if(url.active || also_inactive)
			{
				urls << url;
			}
		}
	}

	return urls;
}


QList<FetchUrl> Manager::search_addresses(const QString& str, bool also_inactive) const
{
	QList<FetchUrl> urls;

	for(const Fetcher::Base* cfi : Algorithm::AsConst(m->coverfetchers))
	{
		if(cfi->is_search_supported())
		{
			FetchUrl url
			(
				is_active(cfi->identifier()),
				cfi->identifier(),
				cfi->search_address(str)
			);

			if(url.active || also_inactive)
			{
				urls << url;
			}
		}
	}

	return urls;
}

QList<FetchUrl> Manager::search_addresses(const QString &str, const QString& cover_fetcher_identifier, bool also_inactive) const
{
	QList<FetchUrl> urls;

	for(const Fetcher::Base* cfi : Algorithm::AsConst(m->coverfetchers))
	{
		if( (cfi->is_search_supported()) &&
			(is_active(cfi)) &&
			(cover_fetcher_identifier.compare(cfi->identifier()) == 0))
		{
			FetchUrl url
			(
				is_active(cfi->identifier()),
				cfi->identifier(),
				cfi->search_address(str)
			);

			if(url.active || also_inactive)
			{
				urls << url;
			}
		}
	}

	if(urls.isEmpty()){
		return search_addresses(str, also_inactive);
	}

	return urls;
}
