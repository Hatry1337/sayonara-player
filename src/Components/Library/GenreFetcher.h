/* GenreFetcher.h */

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



#ifndef GENREFETCHER_H
#define GENREFETCHER_H

#include <QObject>
#include "Utils/Pimpl.h"
#include "Utils/SetFwd.h"

class LocalLibrary;
class Genre;

namespace Tagging
{
	class Editor;
	class UserOperations;
}

class GenreFetcher :
		public QObject
{
	Q_OBJECT
	PIMPL(GenreFetcher)

signals:
	void sigGenresFetched();
	void sigProgress(int progress);
	void sigFinished();

public:
	explicit GenreFetcher(QObject* parent=nullptr);
	~GenreFetcher() override;

	Util::Set<Genre> genres() const;

	void applyGenreToMetadata(const MetaDataList& v_md, const Genre& genre);
	void createGenre(const Genre& genre);
	void deleteGenre(const Genre& genre);
	void deleteGenres(const Util::Set<Genre>& genres);
	void renameGenre(const Genre& oldGenre, const Genre& newGenre);

	void set_local_library(LocalLibrary* local_library);

public slots:
	void reloadGenres();

private:
	Tagging::UserOperations* initTagging();

};

#endif // GENREFETCHER_H
