/* GUI_LastFM.cpp */

/* Copyright (C) 2011-2017 Lucio Carreras
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


/*
 * GUI_LastFM.cpp
 *
 *  Created on: Apr 21, 2011
 *      Author: Lucio Carreras
 */

#include "GUI_LastFM.h"
#include "GUI/Preferences/ui_GUI_LastFM.h"

#include "Components/StreamPlugins/LastFM/LastFM.h"

#include "Helper/Language.h"
#include "Helper/Settings/Settings.h"


GUI_LastFM::GUI_LastFM(QWidget* parent) :
	PreferenceWidgetInterface(parent)
{
    _lfm = LastFM::Base::instance();
}


GUI_LastFM::~GUI_LastFM()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}


void GUI_LastFM::init_ui()
{
	setup_parent(this, &ui);

	revert();

	connect(ui->btn_login, &QPushButton::clicked, this, &GUI_LastFM::btn_login_clicked);
	connect(ui->cb_activate, &QCheckBox::toggled, this, &GUI_LastFM::active_changed);
    connect(_lfm, &LastFM::Base::sig_logged_in, this, &GUI_LastFM::logged_in);
}


QString GUI_LastFM::get_action_name() const
{
	return "Last.fm";
}

void GUI_LastFM::retranslate_ui()
{
	ui->retranslateUi(this);
	ui->lab_activate->setText(Lang::get(Lang::Activate));
	ui->lab_sec->setText(Lang::get(Lang::Seconds));

}

void GUI_LastFM::commit()
{
	StringPair user_pw;
	user_pw.first = ui->tf_username->text();
	user_pw.second = ui->tf_password->text();

	_settings->set(Set::LFM_Login, user_pw);

	if( ui->tf_username->text().length() >= 3 &&
		ui->tf_password->text().length() >= 3 )
	{
		_lfm->psl_login();

		_settings->set( Set::LFM_Active, ui->cb_activate->isChecked() );
	}

	_settings->set(Set::LFM_ScrobbleTimeSec, ui->sb_scrobble_time->value());
}


void GUI_LastFM::revert()
{
	bool active = _settings->get(Set::LFM_Active);

	ui->cb_activate->setChecked(active);
	active_changed(active);

	logged_in(_lfm->is_logged_in());

	StringPair user_pw = _settings->get(Set::LFM_Login);
	ui->tf_username->setText(user_pw.first);
	ui->tf_password->setText(user_pw.second);
	ui->sb_scrobble_time->setValue( _settings->get(Set::LFM_ScrobbleTimeSec) );
}


void GUI_LastFM::btn_login_clicked()
{
	if(ui->tf_username->text().length() < 3) {
		return;
	}

	if(ui->tf_password->text().length() < 3) {
		return;
	}

	StringPair user_pw;
	user_pw.first = ui->tf_username->text();
	user_pw.second = ui->tf_password->text();

	_settings->set(Set::LFM_Login, user_pw);

	_lfm->psl_login();
}


void GUI_LastFM::active_changed(bool active) {
	if(!is_ui_initialized()){
		return;
	}

	ui->tf_username->setEnabled(active);
	ui->tf_password->setEnabled(active);
}


void GUI_LastFM::logged_in(bool success){
	if(!is_ui_initialized()){
		return;
	}

	if(success){
		ui->lab_status->setText(tr("Logged in"));
	}

	else{
		ui->lab_status->setText(tr("Not logged in"));
	}
}

