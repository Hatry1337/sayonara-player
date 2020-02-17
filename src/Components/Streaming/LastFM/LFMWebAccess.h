/* WebAccess.h */

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
 * WebAccess.h
 *
 *  Created on: Oct 22, 2011
 *      Author: Michael Lugmair (Lucio Carreras)
 */

#ifndef WebAccess_H_
#define WebAccess_H_

#include <QObject>
#include <QMap>

class QByteArray;

namespace LastFM
{
    class UrlParams :  public QMap<QByteArray, QByteArray>
    {
		public:
			UrlParams();
			void appendSignature();
    };

    class WebAccess : public QObject
    {
        Q_OBJECT

    signals:
        void sigResponse(const QByteArray& response);
        void sigError(const QString& error);

    public:
        void callUrl(const QString& url);
        void callPostUrl(const QString& url, const QByteArray& data);

    public:
        static QString parseTokenAnswer(const QString& content);
        static QString createStandardUrl(const QString& base_url, const UrlParams& data);
        static QString createPostUrl(const QString& base_url, const UrlParams& data, QByteArray& postData);

    private:
        QString parseErrorMessage(const QString& response);
        bool checkError(const QByteArray& data);

	private slots:
        void awaFinished();




    };
}
#endif /* WebAccess_H_ */
