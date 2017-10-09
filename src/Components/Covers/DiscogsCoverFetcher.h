/* DiscogsCoverFetcher.h */

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



#ifndef DISCOGSCOVERFETCHER_H
#define DISCOGSCOVERFETCHER_H

#include "CoverFetcherInterface.h"

namespace Cover
{
namespace Fetcher
{
    /**
     * @brief The DiscogsCoverFetcher class. See CoverFetcherInterface
     * @ingroup Covers
     */
    class Discogs :
            public Cover::Fetcher::Base
    {

    public:
        bool can_fetch_cover_directly() const override;
        QStringList calc_addresses_from_website(const QByteArray& website) const override;
        QString artist_address(const QString& artist) const override;
        QString album_address(const QString& artist, const QString& album) const override;
        QString search_address(const QString& str) const override;
        bool is_search_supported() const override;
        bool is_album_supported() const override;
        bool is_artist_supported() const override;
        int estimated_size() const override;
        QString keyword() const override;
    };

} //fetcher
} //cover
#endif // DISCOGSCOVERFETCHER_H
