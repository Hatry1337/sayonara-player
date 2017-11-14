/* typedefs.h */

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

#ifndef TYPEDEFS_H
#define TYPEDEFS_H

#include <cstdint>
#include <vector>

class QString;
template <typename A, typename B> struct QPair;
template <typename T> class QList;
namespace SP
{
	template <typename T> class Set;
}

/**
 * @brief Sayonara Typedefs
 * @ingroup Helper
 */
using StringPair=QPair<QString, QString>;
using IntList=QList<int>;
using IdList=QList<int>;
using IdxList=QList<int> ;
using BoolList=std::vector<bool> ;
using ArtistId=int32_t;
using AlbumId=int32_t;
using TrackID=int32_t;
using Id=int32_t;
using IntSet=SP::Set<int>;
using IndexSet=SP::Set<int>;
using IdSet=SP::Set<Id>;
using LibraryId=int8_t;
using DbId=uint8_t;
using Byte=uint8_t;
using Disc=uint8_t;
using Rating=uint8_t;

#endif
