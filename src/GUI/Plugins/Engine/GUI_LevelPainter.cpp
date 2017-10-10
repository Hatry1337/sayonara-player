/* GUI_LevelPainter.cpp */

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


#include "GUI_LevelPainter.h"
#include "EngineColorStyleChooser.h"
#include "GUI/Plugins/Engine/ui_GUI_LevelPainter.h"

#include "Components/Engine/Playback/PlaybackEngine.h"
#include "Components/Engine/EngineHandler.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Settings/Settings.h"

#include <QPainter>
#include <QBrush>

#include <cstring>
#include <cmath>
#include <algorithm>
#include <array>

static const int Channels = 2;
using ChannelArray=std::array<float, Channels>;

struct GUI_LevelPainter::Private
{
    static const float MinLevelValue=-40.0f;
    static const float MaxLevelValue=0;

    ChannelArray    level;
    float           exp_lot[600];

    int**           steps=nullptr;


    void resize_steps(int n_rects)
    {
        if(!steps)
        {
            steps = new int*[Channels];
            memset(steps, 0, sizeof(int*) * Channels);
        }

        for(int i=0; i<2; i++)
        {
            if(steps[i]){
                delete[] steps[i];
            }

            steps[i] = new int[n_rects];
            memset(steps[i], 0, n_rects * sizeof(int));
        }
    }

    void set_level(float left, float right)
    {
        level[0] = left;
        level[1] = right;
    }

};


GUI_LevelPainter::GUI_LevelPainter(QWidget *parent) :
	EnginePlugin(parent)
{
    m = Pimpl::make<Private>();
	_settings->set(Set::Engine_ShowLevel, false);
}


GUI_LevelPainter::~GUI_LevelPainter()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}


void GUI_LevelPainter::init_ui()
{
    EnginePlugin::init_ui();
	setup_parent(this, &ui);

	_cur_style_idx = _settings->get(Set::Level_Style);
	_cur_style = _ecsc->get_color_scheme_level(_cur_style_idx);

	reload();

    // exp(-6.0) = 0.002478752
    // exp(0) = 1;
    float f=0;
    for(int i=0; i<600; i++, f+=0.01f)
    {
        m->exp_lot[i] = std::exp(-i / 100.0f);
	}

    m->resize_steps(_cur_style.n_rects);
    m->set_level(0, 0);

    Engine::Playback* playback_engine = engine()->get_playback_engine();
    if(playback_engine)
    {
		playback_engine->add_level_receiver(this);
	}
}


QString GUI_LevelPainter::get_name() const
{
	return "Level";
}


QString GUI_LevelPainter::get_display_name() const
{
	return tr("Level");
}


void GUI_LevelPainter::retranslate_ui()
{
	ui->retranslateUi(this);
}


void GUI_LevelPainter::set_level(float left, float right)
{
	if(!is_ui_initialized() || !isVisible()){
		return;
	}

    m->set_level(left, right);

    stop_fadeout_timer();
	update();
}


void GUI_LevelPainter::paintEvent(QPaintEvent* e)
{
	Q_UNUSED(e)

	QPainter painter(this);

	int n_rects = _cur_style.n_rects;
	int border_x = _cur_style.hor_spacing;
	int border_y = _cur_style.ver_spacing;
	int n_fading_steps = _cur_style.n_fading_steps;
	int h_rect = _cur_style.rect_height;
	int w_rect = _cur_style.rect_width;

	int y = 10;
	int num_zero = 0;
	int x_init = (w_rect + border_x);

    for(int c=0; c<Channels; c++)
    {
        float level = -std::max(m->level[c], -39.9f) * 15.0f;
		int idx = std::max(0, std::min(599, (int) level));

        level = m->exp_lot[idx];

		int n_colored_rects = n_rects * level;

		QRect rect(0, y, w_rect, h_rect);
        for(int r=0; r<n_rects; r++)
        {
            if(r < n_colored_rects)
            {
				if(!_cur_style.style[r].contains(-1)){
					sp_log(Log::Debug, this) << "Style does not contain -1";
				}

				painter.fillRect(rect, _cur_style.style[r].value(-1) );

                m->steps[c][r] = n_fading_steps - 1;
			}

            else
            {
                if(!_cur_style.style[r].contains(m->steps[c][r])){
                    sp_log(Log::Debug, this) << "2 Style does not contain " << m->steps[c][r] << ", " << c << ", " << r;
				}

                painter.fillRect(rect, _cur_style.style[r].value(m->steps[c][r]) );

                if(m->steps[c][r] > 0) {
                    m->steps[c][r] -= 1;
				}

                if(m->steps[c][r] == 0) {
                    num_zero++;
				}
			}

			rect.translate(x_init, 0);
		}

        if(num_zero == Channels * n_rects)
        {
            // all rectangles where fade out
            stop_fadeout_timer();
		}

		y += h_rect + border_y;
	}
}


void GUI_LevelPainter::do_fadeout_step()
{
    for(float& l : m->level)
    {
        l -= 2.0f;
	}

	update();
}

void GUI_LevelPainter::sl_update_style()
{
	_settings->set(Set::Level_Style, _cur_style_idx);
	_ecsc->reload(width(), height());
	_cur_style = _ecsc->get_color_scheme_level(_cur_style_idx);

    m->resize_steps(_cur_style.n_rects);

	update();
}


void GUI_LevelPainter::reload()
{
	int new_height = _cur_style.rect_height * 2 + _cur_style.ver_spacing + 12;

	setMinimumHeight(0);
	setMaximumHeight(100);

	setMinimumHeight(new_height);
	setMaximumHeight(new_height);

	if(isVisible()){
		emit sig_reload(this);
	}
}

void GUI_LevelPainter::showEvent(QShowEvent* e)
{
	_settings->set(Set::Engine_ShowLevel, true);
	EnginePlugin::showEvent(e);
}


void GUI_LevelPainter::closeEvent(QCloseEvent* e)
{
	_settings->set(Set::Engine_ShowLevel, false);
	EnginePlugin::closeEvent(e);
}


QWidget *GUI_LevelPainter::widget()
{
    return this;
}

bool GUI_LevelPainter::has_small_buttons() const
{
    return true;
}
