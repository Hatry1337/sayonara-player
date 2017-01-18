/* Filter.h

 * Copyright (C) 2011-2017 Lucio Carreras
 *
 * This file is part of sayonara-player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * created by Lucio Carreras,
 * Jul 9, 2012
 *
 */

#ifndef FILTER_H_
#define FILTER_H_


#include "Helper/Pimpl.h"

class QString;
namespace Library
{
    class DateFilter;
}

namespace Library
{
	/**
	 * @brief The Filter class
	 * @ingroup LibraryHelper
	 */
    class Filter
    {
	public:

	    enum Mode
	    {
			Fulltext=0,
			Filename,
			Genre,
			Date
	    };


	    Filter();
	    ~Filter();
	    void operator=(const Filter& other);
	    Filter(const Filter& other);

	    Library::DateFilter date_filter() const;
	    void set_date_filter(const Library::DateFilter& filter);

	    QString filtertext() const;
	    void set_filtertext(const QString& str);

	    Library::Filter::Mode mode() const;
	    void set_mode(Library::Filter::Mode mode);

	    void clear();
	    bool cleared() const;

	private:
	    PIMPL(Filter)
    };
}

#endif /* FILTER_H_ */
