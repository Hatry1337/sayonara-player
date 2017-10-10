/* PlaybackPipeline.h */

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

#ifndef GSTPLAYBACKPIPELINE_H_
#define GSTPLAYBACKPIPELINE_H_

#include "ChangeablePipeline.h"
#include "Crossfader.h"
#include "Components/Engine/AbstractPipeline.h"
#include "Utils/Pimpl.h"

namespace Pipeline
{
    /**
     * @brief The PlaybackPipeline class
     * @ingroup Engine
     */
    class Playback :
            public Base,
            public CrossFader,
            public Changeable
    {
        Q_OBJECT
        PIMPL(Playback)

    public:
        explicit Playback(Engine::Base* engine, QObject *parent=nullptr);
        virtual ~Playback();

        bool init(GstState state=GST_STATE_NULL) override;
        bool set_uri(gchar* uri) override;

        void set_n_sound_receiver(int num_sound_receiver);

        void set_current_volume(double volume) override;
        double get_current_volume() const override;

        GstElement* get_source() const override;
        GstElement* get_pipeline() const override;

        void force_about_to_finish();


    public slots:
        void play() override;
        void stop() override;

        void set_eq_band(int band_name, int val);
        void set_streamrecorder_path(const QString& session_path);

        int64_t seek_rel(double percent, int64_t ref_ns);
        int64_t seek_abs(int64_t ns );

    private:
        void init_equalizer();
        bool create_elements() override;
        bool add_and_link_elements() override;
        bool configure_elements() override;
        uint64_t get_about_to_finish_time() const override;

    protected slots:
        void s_vol_changed();
        void s_show_level_changed();
        void s_show_spectrum_changed();
        void s_mute_changed();
        void s_speed_active_changed();
        void s_speed_changed();
    };
}

#endif
