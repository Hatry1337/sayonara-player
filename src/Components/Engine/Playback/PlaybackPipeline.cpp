/* PlaybackPipeline.cpp */

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

#include "PlaybackPipeline.h"
#include "PipelineProbes.h"
#include "SpeedHandler.h"
#include "EqualizerHandler.h"
#include "SeekHandler.h"
#include "StreamRecorderHandler.h"
#include "StreamRecorderData.h"

#include "Components/Engine/Callbacks/PipelineCallbacks.h"
#include "Utils/globals.h"
#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Logger/Logger.h"

#include <gst/base/gstdataqueue.h>
#include <gst/app/gstappsink.h>

#include <algorithm>

//http://gstreamer.freedesktop.org/data/doc/gstreamer/head/manual/html/chapter-dataaccess.html

using Pipeline::Playback;
using Pipeline::test_and_error;
using Pipeline::test_and_error_bool;

struct Playback::Private :
		public SpeedHandler,
		public EqualizerHandler,
		public StreamRecorderHandler,
		public SeekHandler
{
	GstElement*			audio_src=nullptr;
	GstElement*			audio_convert=nullptr;
	GstElement*			tee=nullptr;

	GstElement*			eq_queue=nullptr;
	GstElement*			equalizer=nullptr;
	GstElement*			speed=nullptr;
	GstElement*			volume=nullptr;
	GstElement*			pitch=nullptr;

	GstElement*			audio_sink=nullptr;

	GstElement*			spectrum_queue=nullptr;
	GstElement*			spectrum=nullptr;
	GstElement*			spectrum_sink=nullptr;

	GstElement*			level_queue=nullptr;
	GstElement*			level=nullptr;
	GstElement*			level_sink=nullptr;

	GstElement*			lame_queue=nullptr;
	GstElement*			lame_converter=nullptr;
	GstElement*			lame_resampler=nullptr;
	GstElement*			lame=nullptr;
	GstElement*			lame_app_sink=nullptr;

	GstElement*			file_queue=nullptr;
	GstElement*			file_converter=nullptr;
	GstElement*			file_sink=nullptr;
	GstElement*			file_resampler=nullptr;
	GstElement*			file_lame=nullptr;

	gulong				level_probe, spectrum_probe, lame_probe, file_probe;
	int					vol;
	bool				show_level, show_spectrum, run_broadcast, run_sr;

	Private() :
		SpeedHandler(),
		EqualizerHandler(),
		StreamRecorderHandler(),
		SeekHandler(),

		level_probe(0),
		spectrum_probe(0),
		lame_probe(0),
		file_probe(0),
		vol(0),
		show_level(false),
		show_spectrum(false),
		run_broadcast(false),
		run_sr(false)
	{}


	protected:
		GstElement* get_pitch_element() const override
		{
			return pitch;
		}

		GstElement* get_equalizer_element() const override
		{
			return equalizer;
		}

		GstElement* get_streamrecorder_sink_element() const override
		{
			return file_sink;
		}

		GstElement* get_source() const override
		{
			return audio_src;
		}
};


Playback::Playback(Engine::Base* engine, QObject *parent) :
	Pipeline::Base("Playback Pipeline", engine, parent),
	CrossFader(),
	Pipeline::Changeable()
{
	m = Pimpl::make<Private>();
}

Playback::~Playback() {}

bool Playback::init(GstState state)
{
	if(!Base::init(state)){
		return false;
	}

	_settings->set(SetNoDB::MP3enc_found, m->lame != nullptr);
	_settings->set(SetNoDB::Pitch_found, m->lame != nullptr);

	Set::listen(Set::Engine_Vol, this, &Playback::s_vol_changed);
	Set::listen(Set::Engine_Mute, this, &Playback::s_mute_changed);
	Set::listen(Set::Engine_Sink, this, &Playback::s_sink_changed, false);

	// set by gui, initialized directly in pipeline
	Set::listen(Set::Engine_ShowLevel, this, &Playback::s_show_level_changed);
	Set::listen(Set::Engine_ShowSpectrum, this, &Playback::s_show_spectrum_changed);
	Set::listen(Set::Engine_Pitch, this, &Playback::s_speed_changed);
	Set::listen(Set::Engine_Speed, this, &Playback::s_speed_changed);
	Set::listen(Set::Engine_PreservePitch, this, &Playback::s_speed_changed);
	Set::listen(Set::Engine_SpeedActive, this, &Playback::s_speed_active_changed);

	set_n_sound_receiver(false);

	set_streamrecorder_path("");

	return true;
}

bool Playback::create_elements()
{
	// input
	if(!create_element(&m->audio_src, "uridecodebin", "src")) return false;
	if(!create_element(&m->audio_convert, "audioconvert")) return false;
	if(!create_element(&m->equalizer, "equalizer-10bands")) return false;

	if(!create_element(&m->pitch, "pitch")){
		m->pitch = nullptr;
	}

	if(!create_element(&m->tee, "tee")) return false;

	// standard output branch
	if(!create_element(&m->eq_queue, "queue", "eq_queue")) return false;
	if(!create_element(&m->volume, "volume")) return false;

	m->audio_sink = create_audio_sink(_settings->get(Set::Engine_Sink));
	if(!m->audio_sink){
		return false;
	}

	// level branch
	if(!create_element(&m->level_queue, "queue", "level_queue")) return false;
	if(!create_element(&m->level, "level")) return false;
	if(!create_element(&m->level_sink, "fakesink", "level_sink")) return false;

	// spectrum branch
	if(!create_element(&m->spectrum_queue, "queue", "spectrum_queue")) return false;
	if(!create_element(&m->spectrum, "spectrum")) return false;
	if(!create_element(&m->spectrum_sink,"fakesink", "spectrum_sink")) return false;

	// lame branch
	if( !create_element(&m->lame_queue, "queue", "lame_queue") ||
		!create_element(&m->lame_converter, "audioconvert", "lame_converter") ||
		!create_element(&m->lame_resampler, "audioresample", "lame_resampler") ||
		!create_element(&m->lame, "lamemp3enc") ||
		!create_element(&m->lame_app_sink, "appsink", "lame_appsink"))
	{
		m->lame = nullptr;
	}

	// stream recorder branch
	if(	!create_element(&m->file_queue, "queue", "sr_queue") ||
		!create_element(&m->file_converter, "audioconvert", "sr_converter") ||
		!create_element(&m->file_resampler, "audioresample", "sr_resample") ||
		!create_element(&m->file_lame, "lamemp3enc", "sr_lame")  ||
		!create_element(&m->file_sink, "filesink", "sr_filesink"))
	{
		m->file_sink = nullptr;
	}

	else{
		m->streamrecorder_data()->queue = m->file_queue;
		m->streamrecorder_data()->sink = m->file_sink;
	}

	return true;
}

GstElement* Playback::create_audio_sink(const QString& name)
{
	GstElement* ret=nullptr;

	if(name == "pulse"){
		sp_log(Log::Info, this) << "Create pulseaudio sink";
		create_element(&ret, "pulsesink", name.toLocal8Bit().data());
	}

	else if(name == "alsa"){
		sp_log(Log::Info, this) << "Create alsa sink";
		create_element(&ret, "alsasink", name.toLocal8Bit().data());
	}

	if(ret == nullptr){
		sp_log(Log::Info, this) << "Create automatic sink";
		create_element(&ret, "autoaudiosink", name.toLocal8Bit().data());
	}

	return ret;
}

bool Playback::add_and_link_elements()
{
	bool success;
	GstPadTemplate* tee_src_pad_template;

	gst_bin_add_many(GST_BIN(_pipeline),
					 m->audio_src, m->audio_convert, m->equalizer, m->tee,

					 m->eq_queue, m->volume, m->audio_sink,
					 m->level_queue, m->level, m->level_sink,
					 m->spectrum_queue, m->spectrum, m->spectrum_sink,

					 nullptr);

	/* before tee */
	success = gst_element_link_many(m->audio_convert, m->equalizer, m->tee,  nullptr);
	if(!test_and_error_bool(success, "Engine: Cannot link audio convert with tee")){
		return false;
	}

	/* standard output branch */
	success = gst_element_link_many(m->eq_queue, m->volume, /*_speed,*/ m->audio_sink, nullptr);
	if(!test_and_error_bool(success, "Engine: Cannot link eq with audio sink")) {
		return false;
	}


	/* level branch */
	success = gst_element_link_many(m->level_queue, m->level, m->level_sink, nullptr);
	if(!test_and_error_bool(success, "Engine: Cannot link Level pipeline")){
		return false;
	}


	/* spectrum branch */
	success = gst_element_link_many(m->spectrum_queue, m->spectrum, m->spectrum_sink, nullptr);
	if(!test_and_error_bool(success, "Engine: Cannot link Spectrum pipeline")){
		return false;
	}


	/* lame branch (optional) */
	if(m->lame){
		gst_bin_add_many(GST_BIN(_pipeline), m->lame_queue,  m->lame_converter, m->lame_resampler, m->lame, m->lame_app_sink, nullptr);
		success = gst_element_link_many( m->lame_queue, m->lame_converter, m->lame_resampler, m->lame, m->lame_app_sink, nullptr);
		test_and_error_bool(success, "Engine: Cannot link lame stuff");
	}

	/* stream rippper branch (optional) */
	if(m->file_sink){
		gst_bin_add_many(GST_BIN(_pipeline), m->file_queue, m->file_converter, m->file_resampler, m->file_lame, m->file_sink, nullptr);
		success = gst_element_link_many( m->file_queue, m->file_converter, m->file_resampler, m->file_lame, m->file_sink, nullptr);
		test_and_error_bool(success, "Engine: Cannot link streamripper stuff");
	}

	/* create tee pads */
	tee_src_pad_template = gst_element_class_get_pad_template (GST_ELEMENT_GET_CLASS (m->tee), "src_%u");
	if(!test_and_error(tee_src_pad_template, "Engine: _tee_src_pad_template is nullptr")) {
		return false;
	}

	/* connect branches with tee */
	success = tee_connect(m->tee, tee_src_pad_template, m->level_queue, "Level");
	if(!test_and_error_bool(success, "Engine: Cannot link level queue with tee")){
		return false;
	}

	tee_connect(m->tee, tee_src_pad_template, m->spectrum_queue, "Spectrum");
	if(!test_and_error_bool(success, "Engine: Cannot link spectrum queue with tee")){
		return false;
	}

	tee_connect(m->tee, tee_src_pad_template, m->eq_queue, "Equalizer");
	if(!test_and_error_bool(success, "Engine: Cannot link eq queue with tee")){
		return false;
	}

	if(m->lame){
		success = tee_connect(m->tee, tee_src_pad_template, m->lame_queue, "Lame");
		if(!test_and_error_bool(success, "Engine: Cannot link lame queue with tee")){
			_settings->set(SetNoDB::MP3enc_found, false);
		}
	}

	if(m->file_sink){
		success = tee_connect(m->tee, tee_src_pad_template, m->file_queue, "Streamripper");
		if(!test_and_error_bool(success, "Engine: Cannot link streamripper stuff")){
			_settings->set(Set::Engine_SR_Active, false);
		}
	}

	return true;
}

bool Playback::configure_elements()
{
	guint64 interval = 25000000;
	gint threshold = -75;
	QList<GstElement*> sinks;

	g_object_set (G_OBJECT (m->audio_src),
				  "use-buffering", true,
				  nullptr);

	g_object_set (G_OBJECT (m->level),
				  "post-messages", true,
				  "interval", interval,
				  nullptr);

	int bins = _settings->get(Set::Engine_SpectrumBins);
	g_object_set (G_OBJECT (m->spectrum),
				  "post-messages", true,
				  "interval", interval,
				  "bands", bins,
				  "threshold", threshold,
				  "message-phase", false,
				  "message-magnitude", true,
				  "multi-channel", false,
				  nullptr);

	m->init_equalizer();

	/* run synced and not as fast as we can */
	g_object_set(G_OBJECT (m->audio_sink),
				 "sync",
				 true,
				 nullptr);

	if(m->lame){
		g_object_set(G_OBJECT (m->lame),
					 "perfect-timestamp", true,
					 "target", 1,
					 "cbr", true,
					 "bitrate", 128,
					 "encoding-engine-quality", 2,
					 nullptr);

		gst_object_ref(m->lame_app_sink);
		g_object_set(G_OBJECT(m->lame_app_sink),
					 "emit-signals",
					 true, nullptr );

		sinks << m->lame_app_sink;
	}

	if(m->file_sink){
		g_object_set(G_OBJECT (m->file_lame),

					 "perfect-timestamp", true,
					 "target", 1,
					 "cbr", true,
					 "bitrate", 128,
					 "encoding-engine-quality", 2,
					 nullptr);

		g_object_set(G_OBJECT(m->file_sink),
					 "buffer-size", 8192,
					 "location", (Util::sayonara_path() + "bla.mp3").toLocal8Bit().data(),
					 nullptr);

		sinks << m->file_sink;

		gst_element_set_state(m->file_sink, GST_STATE_NULL);
	}

	sinks << m->level_sink << m->spectrum_sink;

	for(GstElement* sink : sinks){
		//gst_object_ref(sink);
		/* run synced and not as fast as we can */
		g_object_set(G_OBJECT (sink), "sync", true, nullptr);

		/* change the state, when it's said to change the state */
		g_object_set(G_OBJECT (sink), "async", false, nullptr);
	}

	g_signal_connect (m->audio_src, "pad-added", G_CALLBACK (Callbacks::decodebin_ready), m->audio_convert);
	g_signal_connect (m->audio_src, "source-setup", G_CALLBACK (Callbacks::source_ready), nullptr);
	if(m->lame){
		g_signal_connect (m->lame_app_sink, "new-sample", G_CALLBACK(Callbacks::new_buffer), this);
	}

	return true;
}

MilliSeconds Playback::get_about_to_finish_time() const
{
	return std::max( get_fading_time_ms(), Base::get_about_to_finish_time() );
}

void Playback::play()
{
	Base::play();
}

void Playback::stop()
{
	Base::stop();

	abort_fader();
}

void Playback::s_vol_changed()
{
	m->vol = _settings->get(Set::Engine_Vol);

	float vol_val = (float) ((m->vol * 1.0f) / 100.0f);

	g_object_set(G_OBJECT(m->volume), "volume", vol_val, nullptr);
}


void Playback::s_mute_changed()
{
	bool muted = _settings->get(Set::Engine_Mute);
	g_object_set(G_OBJECT(m->volume), "mute", muted, nullptr);
}

void Playback::s_show_level_changed()
{
	m->show_level = _settings->get(Set::Engine_ShowLevel);
	Probing::handle_probe(&m->show_level, m->level_queue, &m->level_probe, Probing::level_probed);
}


void Playback::s_show_spectrum_changed()
{
	m->show_spectrum = _settings->get(Set::Engine_ShowSpectrum);

	Probing::handle_probe(&m->show_spectrum, m->spectrum_queue, &m->spectrum_probe, Probing::spectrum_probed);
}


void Playback::set_n_sound_receiver(int num_sound_receiver)
{
	if(!m->lame){
		return;
	}

	m->run_broadcast = (num_sound_receiver > 0);

	Probing::handle_probe(&m->run_broadcast, m->lame_queue, &m->lame_probe, Probing::lame_probed);
}

GstElement* Playback::get_source() const
{
	return m->audio_src;
}

GstElement* Playback::get_pipeline() const
{
	return _pipeline;
}

void Playback::force_about_to_finish()
{
	_about_to_finish = true;
	emit sig_about_to_finish(get_about_to_finish_time());
}


bool Playback::set_uri(gchar* uri)
{
	stop();

	g_object_set(G_OBJECT(m->audio_src), "uri", uri, nullptr);

	gst_element_set_state(_pipeline, GST_STATE_PAUSED);

	return true;
}


void Playback::set_eq_band(int band, int val)
{
	m->set_band(band, val);
}

void Playback::set_streamrecorder_path(const QString& path)
{
	m->set_streamrecorder_target_path(path);
}

NanoSeconds Playback::seek_rel(double percent, NanoSeconds ref_ns)
{
	return m->seek_rel(percent, ref_ns);
}

NanoSeconds Playback::seek_abs(NanoSeconds ns)
{
	return m->seek_abs(ns);
}

void Playback::set_current_volume(double volume)
{
	g_object_set(m->volume, "volume", volume, nullptr);
}


double Playback::get_current_volume() const
{
	double volume;
	g_object_get(m->volume, "volume", &volume, nullptr);
	return volume;
}


void Playback::s_speed_active_changed()
{
	if(!m->pitch){
		return;
	}

	GstElement* source = get_source();
	bool active = _settings->get(Set::Engine_SpeedActive);

	NanoSeconds pos;
	bool success = gst_element_query_position(source, GST_FORMAT_TIME, &pos);

	if(active){
		add_element(m->pitch, m->audio_convert, m->equalizer);
		s_speed_changed();
	}

	else{
		remove_element(m->pitch, m->audio_convert, m->equalizer);
	}

	if(this->get_state() == GST_STATE_PLAYING && success)
	{
		pos = std::max<NanoSeconds>(pos, 0);
		m->seek_nearest( (NanoSeconds) pos);
	}
}


void Playback::s_speed_changed()
{
	m->set_speed(
		_settings->get(Set::Engine_Speed),
		_settings->get(Set::Engine_Pitch) / 440.0,
		_settings->get(Set::Engine_PreservePitch)
	);
}

void Playback::s_sink_changed()
{
	GstElement* e = create_audio_sink(_settings->get(Set::Engine_Sink));
	if(!e){
		return;
	}

	GstState old_state, state;
	gst_element_get_state(get_pipeline(), &old_state, nullptr, 0);

	state = old_state;

	stop();
	while(state != GST_STATE_NULL)
	{
		gst_element_get_state(get_pipeline(), &state, nullptr, 0);
		Util::sleep_ms(50);
	}

	NanoSeconds pos;
	gst_element_query_position(get_pipeline(), GST_FORMAT_TIME, &pos);

	gst_element_unlink(m->volume, m->audio_sink);
	gst_bin_remove(GST_BIN(get_pipeline()), m->audio_sink);
	m->audio_sink = e;
	gst_bin_add(GST_BIN(get_pipeline()), m->audio_sink);
	gst_element_link(m->volume, m->audio_sink);

	gst_element_set_state(get_pipeline(), old_state);

	state = GST_STATE_NULL;
	while(state != old_state)
	{
		gst_element_get_state(get_pipeline(), &state, nullptr, 0);
		Util::sleep_ms(50);
	}

	if(old_state != GST_STATE_NULL) {
		seek_abs((NanoSeconds)(pos));
	}
}
