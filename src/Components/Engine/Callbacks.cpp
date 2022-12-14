/* EngineCallbacks.cpp */

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

#include "Callbacks.h"
#include "Components/Engine/EngineUtils.h"
#include "Components/Engine/Engine.h"
#include "Components/Engine/Pipeline.h"

#include "Utils/Utils.h"
#include "Utils/WebAccess/Proxy.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaData.h"
#include "Utils/Logger/Logger.h"
#include "Utils/globals.h"

#include <QList>
#include <QRegExp>
#include <QByteArray>

#include <memory>
#include <algorithm>
#include <vector>

#include <gst/gst.h>

namespace EngineUtils = ::Engine::Utils;
namespace Callbacks = ::Engine::Callbacks;
using ::Engine::Engine;
namespace EngineNS = Engine;

const char* ClassEngineCallbacks = "Engine Callbacks";

#ifdef Q_OS_WIN
void EngineCallbacks::destroy_notify(gpointer data) {}

GstBusSyncReply
EngineCallbacks::bus_message_received(GstBus* bus, GstMessage* msg, gpointer data) {
	if(bus_state_changed(bus, msg, data)){
		gst_message_unref(msg);
		return GST_BUS_DROP;
	}

	gst_message_unref(msg);
	return GST_BUS_DROP;
}
#endif

namespace
{
	bool isSoupSource(GstElement* source)
	{
		auto* factory = gst_element_get_factory(source);
		const auto elementType = gst_element_factory_get_element_type(factory);
		const auto* name = g_type_name(elementType);

		return (name && QString(name).toLower() == "gstsouphttpsrc");
	}

	GstStructure* getSoundcloudOAuthStructure()
	{
		const auto soundcloudAuthToken = GetSetting(SetNoDB::Soundcloud_AuthToken);
		const auto oauthTokenValue = QString("SoundcloudAuth,Authorization=\"OAuth\\ %1\"")
			.arg(soundcloudAuthToken);

		return gst_structure_new_from_string(oauthTokenValue.toLocal8Bit().data());
	}

	bool hasSoundcloudUri(GstElement* source)
	{
		gchar* uri = nullptr;
		g_object_get(source, "location", &uri, nullptr);
		return (uri && !strncmp(uri, "https://api.soundcloud.com", 26));
	}
}

static bool parse_image(GstElement* src, GstTagList* tags, EngineNS::Engine* engine)
{
	GstSample* sample;
	bool success = gst_tag_list_get_sample(tags, GST_TAG_IMAGE, &sample);

	if(!success)
	{
		success = gst_tag_list_get_sample(tags, GST_TAG_PREVIEW_IMAGE, &sample);
		if(!success)
		{
			return false;
		}
	}

	GstCaps* caps = gst_sample_get_caps(sample);
	if(!caps)
	{
		gst_sample_unref(sample);
		return false;
	}

	EngineUtils::GStringAutoFree mimetype(gst_caps_to_string(caps));
	if(mimetype.data() == nullptr)
	{
		gst_sample_unref(sample);
		return false;
	}

	QString mime;
	QString fullMime(mimetype.data());

	QRegExp re(".*(image/[a-z|A-Z]+).*");
	if(re.indexIn(fullMime) >= 0)
	{
		mime = re.cap(1);
	}

	spLog(Log::Develop, "Engine Callbacks") << "Cover in Track: " << fullMime;

	GstBuffer* buffer = gst_sample_get_buffer(sample);
	if(!buffer)
	{
		gst_sample_unref(sample);
		return false;
	}

	gsize size = gst_buffer_get_size(buffer);
	if(size == 0)
	{
		gst_sample_unref(sample);
		return false;
	}

	gchar* data = new gchar[size];
	size = gst_buffer_extract(buffer, 0, data, size);

	if(size == 0)
	{
		delete[] data;
		gst_sample_unref(sample);
		return false;
	}

	QByteArray arr(data, size);
	engine->updateCover(src, arr, mime);

	delete[] data;
	gst_sample_unref(sample);

	return (size > 0);
}

// check messages from bus
gboolean Callbacks::busStateChanged(GstBus* bus, GstMessage* msg, gpointer data)
{
	static QStringList string_tags
		{
			GST_TAG_TITLE,
			GST_TAG_ARTIST,
			GST_TAG_ALBUM,
			GST_TAG_ALBUM_ARTIST,
			GST_TAG_COMMENT,

			GST_TAG_PERFORMER,
			GST_TAG_HOMEPAGE,
			GST_TAG_DESCRIPTION,
			GST_TAG_ORGANIZATION,
			GST_TAG_CONTACT,
			GST_TAG_SHOW_NAME,
			GST_TAG_PUBLISHER
		};

	Q_UNUSED(bus);

	auto* engine = static_cast<Engine*>(data);
	if(!engine)
	{
		return true;
	}

	GstMessageType msg_type = GST_MESSAGE_TYPE(msg);
	QString msg_src_name = QString(GST_MESSAGE_SRC_NAME(msg)).toLower();
	GstElement* src = reinterpret_cast<GstElement*>(msg->src);

	switch(msg_type)
	{
		case GST_MESSAGE_EOS:

			if(!msg_src_name.contains("sr_filesink") &&
			   !msg_src_name.contains("level_sink") &&
			   !msg_src_name.contains("spectrum_sink") &&
			   !msg_src_name.contains("pipeline"))
			{
				spLog(Log::Debug, ClassEngineCallbacks) << "EOF reached: " << msg_src_name;
				break;
			}

			engine->setTrackFinished(src);

			break;

		case GST_MESSAGE_ELEMENT:
			if(msg_src_name.compare("spectrum") == 0)
			{
				return spectrumHandler(bus, msg, engine);
			}

			if(msg_src_name.compare("level") == 0)
			{
				return levelHandler(bus, msg, engine);
			}

			break;

		case GST_MESSAGE_SEGMENT_DONE:
			spLog(Log::Debug, ClassEngineCallbacks) << "Segment done: " << msg_src_name;
			break;

		case GST_MESSAGE_TAG:
		{
			if(msg_src_name.contains("fake") ||
			   msg_src_name.contains("lame") ||
			   !msg_src_name.contains("sink"))
			{
				break;
			}

			GstTagList* tags = nullptr;
			gst_message_parse_tag(msg, &tags);
			if(!tags)
			{
				break;
			}

			parse_image(src, tags, engine);

			bool success;
			MetaData md = engine->currentTrack();
			bool update_metadata = false;
			for(const QString& tag : string_tags)
			{
				gchar* value = nullptr;
				success = gst_tag_list_get_string(tags, tag.toLocal8Bit().constData(), &value);
				if(!success)
				{
					continue;
				}

				update_metadata = true;

				if(tag == GST_TAG_TITLE)
				{
					md.setTitle(value);
				}

				else if(tag == GST_TAG_ARTIST)
				{
					md.setArtist(value);
				}

				else if(tag == GST_TAG_ALBUM)
				{
					md.setAlbum(value);
				}

				else if(tag == GST_TAG_ALBUM_ARTIST)
				{
					md.setAlbumArtist(value);
				}

				else if(tag == GST_TAG_COMMENT)
				{
					md.setComment(value);
				}

				else
				{
					const gchar* nick = gst_tag_get_nick(tag.toLocal8Bit().constData());

					QString sNick = tag;
					if(nick && strnlen(nick, 3) > 0)
					{
						sNick = QString::fromLocal8Bit(nick);
					}

					md.replaceCustomField(tag, Util::stringToFirstUpper(sNick), value);
				}

				g_free(value);
			}

			Bitrate bitrate;
			success = gst_tag_list_get_uint(tags, GST_TAG_BITRATE, &bitrate);
			if(success)
			{
				engine->updateBitrate((bitrate / 1000) * 1000, src);
			}

			if(update_metadata)
			{
				engine->updateMetadata(md, src);
			}

			gst_tag_list_unref(tags);
		}

			break;

		case GST_MESSAGE_STATE_CHANGED:
			GstState old_state, new_state, pending_state;

			gst_message_parse_state_changed(msg, &old_state, &new_state, &pending_state);
//			sp_log(Log::Debug, "Callback") << GST_MESSAGE_SRC_NAME(msg) << ": "
//							   << "State changed from "
//							   << gst_element_state_get_name(old_state)
//							   << " to "
//							   << gst_element_state_get_name(new_state)
//							   << " pending: "
//							   << gst_element_state_get_name(pending_state);

			if(!msg_src_name.contains("pipeline", Qt::CaseInsensitive))
			{
				break;
			}

			if(new_state == GST_STATE_PLAYING ||
			   new_state == GST_STATE_PAUSED ||
			   new_state == GST_STATE_READY)
			{
				engine->setTrackReady(src);
			}

			break;

		case GST_MESSAGE_BUFFERING:

			gint percent;

			gint avg_in, avg_out;
			int64_t buffering_left;

			GstBufferingMode mode;

			gst_message_parse_buffering(msg, &percent);
			gst_message_parse_buffering_stats(msg, &mode, &avg_in, &avg_out, &buffering_left);

			spLog(Log::Crazy, "Engine Callback") << "Buffering: " << percent;
			spLog(Log::Crazy, "Engine Callback") << "Avg In: " << avg_in << " Avg Out: " << avg_out
			                                     << " buffering_left: " << buffering_left;

			engine->setBufferState(percent, src);
			break;

		case GST_MESSAGE_DURATION_CHANGED:
			engine->updateDuration(src);
			break;

		case GST_MESSAGE_INFO:
			/*gst_message_parse_info(msg, &err, nullptr);*/
			break;

		case GST_MESSAGE_WARNING:
		{
			GError* err;
			gst_message_parse_warning(msg, &err, nullptr);
			spLog(Log::Warning, ClassEngineCallbacks) << "Engine: GST_MESSAGE_WARNING: " << err->message << ": "
			                                          << GST_MESSAGE_SRC_NAME(msg);
			g_error_free(err);
		}
			break;

		case GST_MESSAGE_ERROR:
		{
			static QString error_msg;
			GError* err;
			gst_message_parse_error(msg, &err, nullptr);

			QString src_name(GST_MESSAGE_SRC_NAME(msg));

			spLog(Log::Error, ClassEngineCallbacks) << "Engine: GST_MESSAGE_ERROR: " << err->message << ": "
			                                        << src_name;

			QString new_error_msg = QString(err->message);

			if(error_msg != new_error_msg)
			{
				engine->error(new_error_msg, src_name);
				error_msg = new_error_msg;
			}

			g_error_free(err);
		}
			break;

		case GST_MESSAGE_STREAM_STATUS:
			/*{
				GstStreamStatusType type;
				gst_message_parse_stream_status(msg, &type, NULL);
				sp_log(Log::Debug, ClassEngineCallbacks) << "Get stream status " << type;
			}*/
			break;

		default:
			break;
	}

	return true;
}

// level changed
gboolean
Callbacks::levelHandler(GstBus* bus, GstMessage* message, gpointer data)
{
	Q_UNUSED(bus);

	auto* engine = static_cast<Engine*>(data);
	if(!engine)
	{
		return true;
	}

	const GstStructure* structure = gst_message_get_structure(message);
	if(!structure)
	{
		spLog(Log::Warning, ClassEngineCallbacks) << "structure is null";
		return true;
	}

	const gchar* name = gst_structure_get_name(structure);
	if(strcmp(name, "level") != 0)
	{
		return true;
	}

	const GValue* peak_value = gst_structure_get_value(structure, "peak");
	if(!peak_value)
	{
		return true;
	}

	auto* rms_arr = static_cast<GValueArray*>(g_value_get_boxed(peak_value));
	guint n_peak_elements = rms_arr->n_values;
	if(n_peak_elements == 0)
	{
		return true;
	}

	double channel_values[2];
	n_peak_elements = std::min((guint) 2, n_peak_elements);
	for(guint i = 0; i < n_peak_elements; i++)
	{
		const GValue* val = rms_arr->values + i;

		if(!G_VALUE_HOLDS_DOUBLE(val))
		{
			spLog(Log::Debug, ClassEngineCallbacks) << "Could not find a double";
			break;
		}

		double d = g_value_get_double(val);
		if(d < 0)
		{
			channel_values[i] = d;
		}
	}

	if(n_peak_elements >= 2)
	{
		engine->setLevel(channel_values[0], channel_values[1]);
	}

	else if(n_peak_elements == 1)
	{
		engine->setLevel(channel_values[0], channel_values[0]);
	}

	return true;
}

// spectrum changed
gboolean
Callbacks::spectrumHandler(GstBus* bus, GstMessage* message, gpointer data)
{
	Q_UNUSED(bus);

	static std::vector<float> spectrumValues;

	auto* engine = static_cast<Engine*>(data);
	if(!engine)
	{
		return true;
	}

	const GstStructure* structure = gst_message_get_structure(message);
	if(!structure)
	{
		return true;
	}

	const gchar* structure_name = gst_structure_get_name(structure);
	if(strcmp(structure_name, "spectrum") != 0)
	{
		return true;
	}

	const GValue* magnitudes = gst_structure_get_value(structure, "magnitude");

	int bins = std::max(1, GetSetting(Set::Engine_SpectrumBins));
	if(spectrumValues.empty())
	{
		spectrumValues.resize(bins, 0);
	}

	for(int i = 0; i < bins; ++i)
	{
		const GValue* mag = gst_value_list_get_value(magnitudes, i);
		if(!mag)
		{
			continue;
		}

		float f = g_value_get_float(mag);
		spectrumValues[i] = f;
	}

	engine->setSpectrum(spectrumValues);

	return true;
}

gboolean Callbacks::positionChanged(gpointer data)
{
	auto* pipeline = static_cast<Pipeline*>(data);
	if(!pipeline)
	{
		return false;
	}

	GstState state = pipeline->state();
	if(state != GST_STATE_PLAYING &&
	   state != GST_STATE_PAUSED &&
	   state != GST_STATE_READY)
	{
		return true;
	}

	pipeline->checkPosition();

	return true;
}

// dynamic linking, important for decodebin
void Callbacks::decodebinReady(GstElement* source, GstPad* new_src_pad, gpointer data)
{
	EngineUtils::GStringAutoFree element_name(gst_element_get_name(source));
	spLog(Log::Develop, "Callback") << "Source: " << element_name.data();

	auto* element = static_cast<GstElement*>(data);
	GstPad* sink_pad = gst_element_get_static_pad(element, "sink");
	if(!sink_pad)
	{
		return;
	}

	if(gst_pad_is_linked(sink_pad))
	{
		gst_object_unref(sink_pad);
		return;
	}

	GstPadLinkReturn pad_link_return = gst_pad_link(new_src_pad, sink_pad);

	if(pad_link_return != GST_PAD_LINK_OK)
	{
		spLog(Log::Error, ClassEngineCallbacks) << "Dynamic pad linking: Cannot link pads";

		switch(pad_link_return)
		{
			case GST_PAD_LINK_WRONG_HIERARCHY:
				spLog(Log::Error, ClassEngineCallbacks) << "Cause: Wrong hierarchy";
				break;
			case GST_PAD_LINK_WAS_LINKED:
				spLog(Log::Error, ClassEngineCallbacks) << "Cause: Pad was already linked";
				break;
			case GST_PAD_LINK_WRONG_DIRECTION:
				spLog(Log::Error, ClassEngineCallbacks) << "Cause: Pads have wrong direction";
				break;
			case GST_PAD_LINK_NOFORMAT:
				spLog(Log::Error, ClassEngineCallbacks) << "Cause: Pads have incompatible format";
				break;
			case GST_PAD_LINK_NOSCHED:
				spLog(Log::Error, ClassEngineCallbacks) << "Cause: Pads cannot cooperate scheduling";
				break;
			case GST_PAD_LINK_REFUSED:
			default:
				spLog(Log::Error, ClassEngineCallbacks) << "Cause: Refused because of different reason";
				break;
		}
	}

	else
	{
		spLog(Log::Develop, "Callbacks") << "Successfully linked " << gst_element_get_name(source) << " with "
		                                 << gst_element_get_name(element);
	}

	gst_object_unref(sink_pad);
}

#define TCP_BUFFER_SIZE 16384

GstFlowReturn Callbacks::newBuffer(GstElement* sink, gpointer p)
{
	static char data[TCP_BUFFER_SIZE];

	auto* pipeline = static_cast<PipelineExtensions::BroadcastDataReceiver*>(p);
	if(!pipeline)
	{
		return GST_FLOW_OK;
	}

	GstSample* sample = gst_app_sink_pull_sample(GST_APP_SINK(sink));
	if(!sample)
	{
		return GST_FLOW_OK;
	}

	GstBuffer* buffer = gst_sample_get_buffer(sample);
	if(!buffer)
	{
		gst_sample_unref(sample);
		return GST_FLOW_OK;
	}

	gsize size = gst_buffer_get_size(buffer);
	gsize newSize = gst_buffer_extract(buffer, 0, data, size);

	QByteArray bytes(data, int(newSize));
	pipeline->setRawData(bytes);

	gst_sample_unref(sample);

	return GST_FLOW_OK;
}

void Callbacks::sourceReady(GstURIDecodeBin* /* bin */, GstElement* source, gpointer /* data */)
{
	spLog(Log::Develop, "Engine Callback") << "Source ready: is soup? " << isSoupSource(source);
	gst_base_src_set_dynamic_size(GST_BASE_SRC(source), false);

	if(isSoupSource(source))
	{
		if(Proxy::active())
		{
			spLog(Log::Develop, "Engine Callback") << "Will use proxy: " << Proxy::fullUrl();

			if(Proxy::hasUsername())
			{
				spLog(Log::Develop, "Engine Callback") << "Will use proxy username: " << Proxy::username();

				EngineUtils::setValues(
					source,
					"proxy-id", Proxy::username().toLocal8Bit().data(),
					"proxy-pw", Proxy::password().toLocal8Bit().data());
			}
		}

		if(hasSoundcloudUri(source))
		{
			EngineUtils::setValues(
				source,
				"extra-headers", getSoundcloudOAuthStructure());
		}
	}
}
