/* CoverLookup.h */

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


/*
 * CoverLookup.h
 *
 *  Created on: Apr 4, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef COVERLOOKUP_H_
#define COVERLOOKUP_H_

#include "AbstractCoverLookup.h"
#include "Utils/Pimpl.h"
#include "Utils/CoverUtils.h"

#include <QList>
#include <any>

class QPixmap;
namespace Cover
{
	class Location;

	/**
	 * @brief The CoverLookup class
	 * @ingroup Covers
	 */
	class Lookup :
		public LookupBase
	{
		Q_OBJECT
		PIMPL(Lookup)

		public:
			Lookup(const Location& cl, int n_covers, QObject* parent);
			~Lookup() override;

			/**
			 * @brief Stop the Cover::FetchThread if running and
			 * retrieve the sigFinished signal
			 * If no Cover::FetchThread is running, nothing will happen
			 */
			void stop() override;

			/**
			 * @brief Set some custom data you can retrieve later
			 * @param data
			 */
			template<typename T>
			void setUserData(const T t)
			{
				mUserData = t;
			}

			/**
			 * @brief Fetch your custom data again
			 * @return
			 */
			template<typename T>
			auto userData() const -> T
			{
				try {
					return std::any_cast<T>(mUserData);
				}
				catch(...)
				{
					return T{};
				}
			}

			/**
			 * @brief Get a copy of all pixmaps that where fetched
			 * @return
			 */
			QList<QPixmap> pixmaps() const;

			Util::Covers::Source source() const;

		private:

			bool fetchFromDatabase();
			bool fetchFromExtractor();
			bool fetchFromWWW();

			bool startExtractor(const Location& cl);
			/**
			 * @brief Starts a new CoverFetchThread
			 * @param cl CoverLocation object
			 */
			bool startNewThread(const Location& cl);

			bool addNewCover(const QPixmap& pm, bool save);

			void emitFinished(bool success);

		public slots:
			void start();

		private slots:
			/**
			 * @brief called when CoverFetchThread has found cover
			 * @param cl
			 */
			void coverFound(int idx);

			/**
			 * @brief called when CoverFetchThread has finished
			 */
			void threadFinished(bool);

			void extractorFinished();

		private:
			std::any mUserData;
	};


	/**
	 * @brief CoverLookupPtr
	 * @ingroup Covers
	 */
	using LookupPtr = std::shared_ptr<Lookup>;

}
#endif /* COVERLOOKUP_H_ */
