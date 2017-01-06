/* LFMLoginThread.h */

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

#ifndef LFMLOGINTHREAD_H
#define LFMLOGINTHREAD_H

#include <QObject>
#include "Helper/globals.h"


struct LFMLoginStuff{
	QString token;
	QString session_key;
	bool logged_in;
	bool subscriber;
	QString error;

	LFMLoginStuff(){
		logged_in = false;
		subscriber = false;
	}
};


class LFMLoginThread :
		public QObject
{
	Q_OBJECT

signals:
	void sig_token_received(const QString& token);
	void sig_error(const QString& error);
	void sig_logged_in(bool success);

public:
	explicit LFMLoginThread(QObject *parent=nullptr);
	~LFMLoginThread();

	void login(const QString& username, const QString& password);
	bool request_authorization();

    LFMLoginStuff getLoginStuff();

private slots:
	void wa_response(const QByteArray& data);
	void wa_error(const QString& response);

private:

	LFMLoginStuff _login_info;
};

#endif // LFMLOGINTHREAD_H
