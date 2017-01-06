/* StreamRecorder.h */

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



#ifndef STREAMRECORDER_H
#define STREAMRECORDER_H

#include "Components/PlayManager/PlayState.h"
#include "Helper/Settings/SayonaraClass.h"
#include "Helper/Pimpl.h"

#include <QObject>
#include <QString>

class MetaData;
class PlayManager;
class StreamRecorder :
		public QObject,
		protected SayonaraClass
{

	PIMPL(StreamRecorder)

private:
	// set metadata, add to session collector
    bool save();

	void clear();

	// saves session collector into playlist, creates new session,
	void new_session();

	// check and create session path
	QString check_session_path(const QString& sr_path);


public:
	explicit StreamRecorder(QObject *parent=nullptr);
	virtual ~StreamRecorder();

	// change recording destination, create session path
	// returns destination file
	QString change_track(const MetaData& md);

	// start or end a session
	void record(bool b);

	// is in a session currently
	bool is_recording() const;

private slots:
	void playstate_changed(PlayState state);


};

#endif // STREAMRECORDER_H
