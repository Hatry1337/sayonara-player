/* Visualizer.cpp */

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



#include "Visualizer.h"
#include "Probing.h"
#include "Components/Engine/EngineUtils.h"

#include "Utils/Settings/Settings.h"

#include <QString>
#include <QList>

using namespace PipelineExtensions;

struct Visualizer::Private
{
	GstElement* pipeline=nullptr;
	GstElement* tee=nullptr;

	GstElement*			bin=nullptr;
	GstElement*			queue=nullptr;
	GstElement*			spectrum=nullptr;
	GstElement*			level=nullptr;
	GstElement*			sink=nullptr;

	gulong				probe;
	bool				isRunning;

	Private(GstElement* pipeline, GstElement* tee) :
		pipeline(pipeline),
		tee(tee),
		probe(0),
		isRunning(false)
	{}
};

Visualizer::Visualizer(GstElement* pipeline, GstElement* tee)
{
	m = Pimpl::make<Private>(pipeline, tee);
}

Visualizer::~Visualizer() = default;

bool Visualizer::init()
{
	if(m->bin){
		return true;
	}

	{ // create
		if(	Engine::Utils::createElement(&m->queue, "queue", "visualizer") &&
			Engine::Utils::createElement(&m->level, "level") &&	// in case of renaming, also look in EngineCallbase GST_MESSAGE_EVENT
			Engine::Utils::createElement(&m->spectrum, "spectrum") &&
			Engine::Utils::createElement(&m->sink,"fakesink", "visualizer"))
		{
			Engine::Utils::createBin(&m->bin, {m->queue, m->level, m->spectrum, m->sink}, "visualizer");
		}

		if(!m->bin){
			return false;
		}
	}

	{ // link
		gst_bin_add(GST_BIN(m->pipeline), m->bin);
		bool success = Engine::Utils::connectTee(m->tee, m->bin, "Visualizer");
		if(!success)
		{
			gst_bin_remove(GST_BIN(m->pipeline), m->bin);
			gst_object_unref(m->bin);
			m->bin = nullptr;
			return false;
		}
	}

	{ // configure
		Engine::Utils::setValues(G_OBJECT(m->level), "post-messages", true);
		Engine::Utils::setUint64Value(G_OBJECT(m->level), "interval", 20 * GST_MSECOND);
		Engine::Utils::setValues(G_OBJECT (m->spectrum),
					  "post-messages", true,
					  "message-phase", false,
					  "message-magnitude", true,
					  "multi-channel", false);

		Engine::Utils::setIntValue(G_OBJECT(m->spectrum), "threshold", -75);
		Engine::Utils::setUintValue(G_OBJECT(m->spectrum), "bands", GetSetting(Set::Engine_SpectrumBins));
		Engine::Utils::setUint64Value(G_OBJECT(m->spectrum), "interval", 20 * GST_MSECOND);

		Engine::Utils::configureQueue(m->queue, 1000);
		Engine::Utils::configureSink(m->sink);
	}

	return true;
}

bool Visualizer::setEnabled(bool b)
{
	if(!init()){
		return false;
	}

	m->isRunning = b;
	Probing::handleProbe(&m->isRunning, m->queue, &m->probe, Probing::spectrumProbed);

	bool show_level = GetSetting(Set::Engine_ShowLevel);
	bool show_spectrum = GetSetting(Set::Engine_ShowSpectrum);

	Engine::Utils::setValue(G_OBJECT(m->level), "post-messages", show_level);
	Engine::Utils::setValue(G_OBJECT(m->spectrum), "post-messages", show_spectrum);

	return true;
}
