/* CrossFader.cpp */

/* Copyright (C) 2011-2019  Lucio Carreras
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


/* CrossFader.cpp */

#include "Crossfadeable.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

#include <QThread>

#include <glib.h>
#include <functional>

using PipelineExtensions::CrossFadeable;

class FaderThreadData
{
private:
	CrossFadeable*			m_crossfader=nullptr;
	int						m_cycles;
	int						m_cycle_time_ms;

public:

	FaderThreadData(CrossFadeable* crossfader) :
		m_crossfader(crossfader)
	{
		m_cycles = 0;
		m_cycle_time_ms = 0;
	}

	void set_fading_time(int fading_time)
	{
		m_cycle_time_ms = fading_time / m_cycles;
	}

	void reset()
	{
		m_cycles = get_max_cycles();
	}

	bool is_active() const
	{
		return (m_cycles > 0);
	}

	void abort()
	{
		m_cycles = 0;
	}

	void wait()
	{
		Util::sleep_ms(m_cycle_time_ms);
		m_cycles --;
		m_crossfader->fader_timed_out();
	}

	int get_cycles() const
	{
		return m_cycles;
	}

	static int get_max_cycles()
	{
		return 500;
	}
};

class FaderThread : public QThread
{
	private:
		FaderThreadData*	_ftd=nullptr;

	public:
		FaderThread(FaderThreadData* ftd) :
			QThread(nullptr),
			_ftd(ftd)
		{}

	protected:
		void run() override
		{
			while(_ftd && _ftd->is_active())
			{
				_ftd->wait();
			}

			sp_log(Log::Develop, this) << "Fader thread finished";
		}
};

struct CrossFadeable::Private
{
	FaderThread*    fader=nullptr;
	FaderThreadData* fader_data=nullptr;

	double			volume;
	double			fade_step;

	FadeMode	    fade_mode;
	bool			destructed;

	Private() :
		volume(0),
		fade_step(0),
		fade_mode(CrossFadeable::FadeMode::NoFading),
		destructed(false)
	{}

	~Private()
	{
		if(fader){
			delete fader; fader=nullptr;
		}

		if(fader_data) {
			delete fader_data; fader_data=nullptr;
		}
	}
};

CrossFadeable::CrossFadeable()
{
	m = Pimpl::make<Private>();

	m->fader_data = new FaderThreadData(this);
}

CrossFadeable::~CrossFadeable()
{
	m->destructed = true;

	if(m->fader_data && m->fader_data->is_active())
	{
		m->fader_data->abort();
	}

	if(m->fader)
	{
		while(m->fader->isRunning())
		{
			Util::sleep_ms(10);
		}
	}
}

void CrossFadeable::init_fader()
{
	if(m->fade_mode == CrossFadeable::FadeMode::NoFading){
		return;
	}

	if(m->fader && m->fader_data->is_active())
	{
		m->fader_data->abort();

		while(m->fader->isRunning()){
			Util::sleep_ms(10);
		}

		delete m->fader; m->fader=nullptr;
	}

	int fading_time = GetSetting(Set::Engine_CrossFaderTime);

	m->fader_data->reset();
	m->fader_data->set_fading_time(fading_time);
	m->fader = new FaderThread(m->fader_data);
	m->fader->start();
}



void CrossFadeable::fade_in()
{
	sp_log(Log::Develop, this) << "Fading in";
	double volume = GetSetting(Set::Engine_Vol) / 100.0;

	m->volume = 0;
	m->fade_mode = CrossFadeable::FadeMode::FadeIn;
	m->fade_step = volume / (FaderThreadData::get_max_cycles() * 1.0);

	sp_log(Log::Crazy, this) << "Fading in: "
		<< "Target Volume: " << volume
		<< "Fade step: = " << m->fade_step;

	set_current_volume(0.00001);

	fade_in_handler();
	init_fader();
	play();
}

void CrossFadeable::fade_out()
{
	sp_log(Log::Develop, this) << "Fading out";

	m->volume = GetSetting(Set::Engine_Vol) / 100.0;
	m->fade_mode = CrossFadeable::FadeMode::FadeOut;
	m->fade_step = m->volume / (FaderThreadData::get_max_cycles() * 1.0);

	sp_log(Log::Crazy, this) << "Fading out: "
		<< "Volume: " << m->volume
		<< "Fade step: = " << m->fade_step;

	set_current_volume( m->volume );

	fade_out_handler();
	init_fader();
}

bool CrossFadeable::is_fading_out() const
{
	return (m->fader &&
			m->fader->isRunning() &&
			(m->fade_mode == CrossFadeable::FadeMode::FadeOut));
}

bool CrossFadeable::is_fading_int() const
{
	return (m->fader &&
			m->fader->isRunning() &&
			(m->fade_mode == CrossFadeable::FadeMode::FadeIn));
}


void CrossFadeable::fader_timed_out()
{
	if(m->destructed){
		return;
	}

	if(m->fade_mode == CrossFadeable::FadeMode::FadeIn){
		increase_volume();
	}

	else if(m->fade_mode == CrossFadeable::FadeMode::FadeOut){
		decrease_volume();
	}
}


void CrossFadeable::increase_volume()
{
	double max_volume = GetSetting(Set::Engine_Vol) / 100.0;

	// maybe volume has changed in the meantime


	m->fade_step = std::max(m->fade_step, m->volume / (m->fader_data->get_cycles() * 1.0));
	m->volume += m->fade_step;

	if(m->volume > max_volume)
	{
		sp_log(Log::Develop, this) << "Max volume reached: " << m->volume;
		abort_fader();
		m->destructed = false;
		return;
	}

	set_current_volume(m->volume);
}


void CrossFadeable::decrease_volume()
{
	double max_volume = GetSetting(Set::Engine_Vol) / 100.0;

	// maybe volume has changed in the meantime
	m->volume = std::min(m->volume, max_volume);
	m->fade_step = std::max(m->fade_step, m->volume / (m->fader_data->get_cycles() * 1.0));

	m->volume -= m->fade_step;

	if(m->volume < 0.00001)
	{

		sp_log(Log::Develop, this) <<  "Min volume reached: " << m->volume << ". Stop track";
		sp_log(Log::Crazy, this) << "Max volume: " << max_volume;
		sp_log(Log::Crazy, this) << "Fade step: " << m->fade_step;

		abort_fader();
		stop();
		m->destructed = false;
		return;
	}

	set_current_volume(m->volume);
}

MilliSeconds CrossFadeable::get_fading_time_ms() const
{
	if(GetSetting(Set::Engine_CrossFaderActive))
	{
		return (MilliSeconds) GetSetting(Set::Engine_CrossFaderTime);
	}

	return 0;
}

void CrossFadeable::abort_fader()
{
	if(m->fader_data->is_active())
	{
		m->fader_data->abort();
		Util::sleep_ms(10);
	}
}