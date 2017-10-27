
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

#include "InstanceThread.h"
#include "Utils/Utils.h"
#include "Utils/FileUtils.h"
#include "Utils/Logger/Logger.h"
#include "Components/Playlist/PlaylistHandler.h"


struct InstanceThread::Private
{
	QSharedMemory		memory;
	bool				may_run;

	Private() :
		memory("SayonaraMemory"),
		may_run(true)
	{}
};

InstanceThread::InstanceThread(QObject* parent) :
	QThread(parent)
{
	m = Pimpl::make<Private>();
}

InstanceThread::~InstanceThread()
{
	m->memory.detach();
}

void InstanceThread::run()
{
	m->memory.attach(QSharedMemory::ReadWrite);
	if(!m->memory.data()){
		return;
	}

	while(m->may_run)
	{
		char* ptr = (char*) m->memory.data();

		if(memcmp(ptr, "Req", 3) == 0)
		{
			sp_log(Log::Debug, this) << "Second instance saying hello";

			if(*(ptr + 3) == 'D'){
				parse_memory();
			}

			m->memory.lock();
			memcpy(m->memory.data(), "Ack", 3);
			m->memory.unlock();

			emit sig_player_raise();
		}

		if(m->may_run){
			Util::sleep_ms(100);
		}
	}
}

void InstanceThread::stop()
{
	m->may_run = false;
}

void InstanceThread::parse_memory()
{
	sp_log(Log::Debug, this) << "parse memory for new file...";

	if(m->memory.isAttached()){
		sp_log(Log::Debug, this) << "memory already attached";
	}

	else if(!m->memory.attach()){
		sp_log(Log::Debug, this) << "Cannot attach shared memory " << m->memory.errorString();
		return;
	}

	m->memory.lock();

	const char* data = static_cast<const char*>(m->memory.constData()) + 4;

	if(data)
	{
		QByteArray array(data, m->memory.size());
		QList<QByteArray> strings = array.split('\n');
		QStringList file_list;

		for(const QByteArray& arr : strings)
		{
			QString filename = QString::fromUtf8(arr);

			if(!filename.isEmpty() &&
			   (Util::File::is_playlistfile(filename) || Util::File::is_soundfile(filename)))
			{
				sp_log(Log::Debug, this) << "Add file " << filename;
				file_list << filename;
			}
		}

		QString new_name =  Playlist::Handler::instance()->request_new_playlist_name();
		emit sig_create_playlist(file_list, new_name, true);
	}

	m->memory.unlock();
}
