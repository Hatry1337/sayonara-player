/* EngineHandler.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
 *
 * This file is part of sayonara player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY{} without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "EngineHandler.h"
#include "Engine.h"

#include "Components/PlayManager/PlayManager.h"
#include "Components/Engine/SoundOutReceiver.h"
#include "Interfaces/RawSoundReceiver/RawSoundReceiverInterface.h"

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/MetaData/MetaData.h"

using Engine::Handler;
namespace Algorithm=Util::Algorithm;

struct Handler::Private
{
	QList<RawSoundReceiverInterface*>	raw_sound_receiver;
	Engine* engine=nullptr;
};

Handler::Handler(QObject* parent) :
	QObject(parent)
{
	m = Pimpl::make<Private>();

	m->engine = new Engine(this);
	m->engine->init();

	PlayManager* play_manager = PlayManager::instance();

	connect(play_manager, &PlayManager::sig_playstate_changed,
			this, &Handler::playstate_changed);

	connect(play_manager, &PlayManager::sig_track_changed,
			this, [=](const MetaData& md){
				m->engine->change_track(md);
			});

	connect(play_manager, &PlayManager::sig_seeked_abs_ms,
			m->engine, &Engine::jump_abs_ms);

	connect(play_manager, &PlayManager::sig_seeked_rel,
			m->engine, &Engine::jump_rel);

	connect(play_manager, &PlayManager::sig_seeked_rel_ms,
			m->engine, &Engine::jump_rel_ms);

	connect(play_manager, &PlayManager::sig_record,
			m->engine, &Engine::set_streamrecorder_recording);

	const MetaData& md = play_manager->current_track();
	if(!md.filepath().isEmpty()) {
		m->engine->change_track(md);
	}

	connect(m->engine, &Engine::sig_cover_changed, this, &Handler::sig_cover_changed);
	connect(m->engine, &Engine::sig_error, play_manager, &PlayManager::error);
	connect(m->engine, &Engine::sig_current_position_changed, play_manager, &PlayManager::set_position_ms);
	connect(m->engine, &Engine::sig_track_finished, play_manager, &PlayManager::set_track_finished);
	connect(m->engine, &Engine::sig_track_ready, play_manager, &PlayManager::set_track_ready);
	connect(m->engine, &Engine::sig_buffering, play_manager, &PlayManager::buffering);

	connect(m->engine, &Engine::sig_duration_changed, this, [play_manager](const MetaData& md){
		play_manager->change_duration(md.duration_ms);
	});

	connect(m->engine, &Engine::sig_bitrate_changed, this, [play_manager](const MetaData& md){
		play_manager->change_bitrate(md.bitrate);
	});

	connect(m->engine, &Engine::sig_metadata_changed, this, [play_manager](const MetaData& md){
		play_manager->change_track_metadata(md);
	});
}

Handler::~Handler() = default;

bool Handler::init()
{
	return true;
}

void Handler::shutdown()
{
	delete m->engine;
}

void Handler::playstate_changed(PlayState state)
{
	switch(state)
	{
		case PlayState::Playing:
			m->engine->play();
			break;

		case PlayState::Paused:
			m->engine->pause();
			break;

		case PlayState::Stopped:
			m->engine->stop();
			break;
		default:
			return;
	}
}


void Handler::new_data(const uchar* data, uint64_t n_bytes)
{
	for(RawSoundReceiverInterface* receiver : Algorithm::AsConst(m->raw_sound_receiver))
	{
		receiver->new_audio_data(data, n_bytes);
	}
}

void Handler::register_raw_sound_receiver(RawSoundReceiverInterface* receiver)
{
	if(m->raw_sound_receiver.contains(receiver)){
		return;
	}

	m->raw_sound_receiver << receiver;
	m->engine->set_n_sound_receiver(m->raw_sound_receiver.size());
}

void Handler::unregister_raw_sound_receiver(RawSoundReceiverInterface* receiver)
{
	if(!m->raw_sound_receiver.contains(receiver)){
		return;
	}

	m->raw_sound_receiver.removeOne(receiver);
	m->engine->set_n_sound_receiver(m->raw_sound_receiver.size());
}

void Handler::add_level_receiver(LevelReceiver* receiver)
{
	m->engine->add_level_receiver(receiver);
}

void Handler::add_spectrum_receiver(SpectrumReceiver* receiver)
{
	m->engine->add_spectrum_receiver(receiver);
}

void Handler::set_equalizer(int band, int value)
{
	m->engine->set_equalizer(band, value);
}
