/* GSTPipeline.h */

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

#ifndef GSTPIPELINE_H
#define GSTPIPELINE_H

#include "Utils/Settings/SayonaraClass.h"
#include "Utils/Pimpl.h"

#include <gst/gst.h>
#include <gst/gstbuffer.h>

#include <memory>

#include <QTimer>

namespace Engine
{
    class Base;
}

namespace Pipeline
{
    /**
     * @brief The GSTFileMode enum
     * @ingroup Engine
     */
    enum class GSTFileMode : uint8_t
    {
        File,
        Http
    };

    bool
    test_and_error(void* element, const QString& errorstr);

    bool
    test_and_error_bool(bool b, const QString& errorstr);

    /**
     * @brief The AbstractPipeline class
     * @ingroup Engine
     */
    class Base :
        public QObject,
        public SayonaraClass
    {
        Q_OBJECT
        PIMPL(Base)

        signals:
            void sig_duration_changed();

        protected:

            bool		_about_to_finish;
            QString		_name;

            GstBus*		_bus=nullptr;
            GstElement* _pipeline=nullptr;
            gchar*		_uri=nullptr;

            int64_t		_duration_ms;
            int64_t		_position_source_ms;
            int64_t		_position_pipeline_ms;

            bool tee_connect(GstElement* tee,
                            GstPadTemplate* tee_src_pad_template,
                             GstElement* queue,
                             const QString& queue_name
            );
            bool create_element(GstElement** elem, const gchar* elem_name, const gchar* name="");

            virtual bool create_elements()=0;
            virtual bool add_and_link_elements()=0;
            virtual bool configure_elements()=0;

            virtual uint64_t get_about_to_finish_time() const;

        signals:
            void sig_finished();
            void sig_about_to_finish(int64_t);
            void sig_pos_changed_ms(int64_t);
            void sig_data(uchar*, uint64_t);


        public slots:
            virtual void play();
            virtual void pause();
            virtual void stop();


        public:
            Base(QString name, Engine::Base* engine, QObject* parent=nullptr);
            virtual ~Base();

            virtual GstElement* get_source() const=0;
            virtual bool		init(GstState state=GST_STATE_READY);
            virtual GstElement* get_pipeline() const;
            virtual GstState	get_state();
            virtual void		refresh_position();

            virtual void		finished();
            virtual void		check_about_to_finish();
            virtual int64_t		get_time_to_go() const;
            virtual void		set_data(uchar* data, uint64_t size);

            virtual bool		set_uri(gchar* uri);

            void                update_duration_ms(int64_t duration_ms, GstElement* src);
            virtual int64_t		get_duration_ms() const final ;
            virtual int64_t		get_source_position_ms() const final;
            virtual int64_t		get_pipeline_position_ms() const final;

            bool 				has_element(GstElement* e) const;
    };
}

#endif // GSTPIPELINE_H
