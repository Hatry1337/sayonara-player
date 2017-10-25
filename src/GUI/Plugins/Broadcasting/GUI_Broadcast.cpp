/* GUI_Broadcast.cpp */

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

#include "GUI_Broadcast.h"

#include "Components/Broadcasting/StreamServer.h"
#include "Utils/Message/Message.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language.h"
#include "Utils/Settings/Settings.h"

#include "GUI/Plugins/Broadcasting/ui_GUI_Broadcast.h"

struct GUI_Broadcast::Private
{
	StreamServer*   server=nullptr;
	QAction*        action_dismiss=nullptr;
	QAction*        action_dismiss_all=nullptr;
};

GUI_Broadcast::GUI_Broadcast(QWidget *parent) :
	PlayerPluginInterface(parent)
{
	m = Pimpl::make<GUI_Broadcast::Private>();
	m->server = new StreamServer(this);

	connect(m->server, &StreamServer::sig_new_connection, this, &GUI_Broadcast::connection_established);
	connect(m->server, &StreamServer::sig_connection_closed, this, &GUI_Broadcast::connection_closed);
	connect(m->server, &StreamServer::sig_listening, this, &GUI_Broadcast::can_listen_changed);
}


GUI_Broadcast::~GUI_Broadcast()
{
	m->server->deleteLater();

	if(ui)
	{
		delete ui; ui = nullptr;
	}
}


QString GUI_Broadcast::get_name() const
{
	return "Broadcast";
}


QString GUI_Broadcast::get_display_name() const
{
	return Lang::get(Lang::Broadcast);
}


void GUI_Broadcast::retranslate_ui()
{
	ui->retranslateUi(this);
	set_status_label();
	ui->btn_retry->setText(Lang::get(Lang::Retry));
}


void GUI_Broadcast::init_ui()
{
	if(is_ui_initialized()){
		return;
	}

	setup_parent(this, &ui);

	m->action_dismiss = new QAction(tr("Dismiss"), ui->btn_menu);
	m->action_dismiss_all = new QAction(tr("Dismiss all"), ui->btn_menu);

	m->action_dismiss->setEnabled(false);
	m->action_dismiss_all->setEnabled(false);

	ui->btn_menu->register_action(m->action_dismiss);
	ui->btn_menu->register_action(m->action_dismiss_all);

	connect(m->action_dismiss, &QAction::triggered, this, &GUI_Broadcast::dismiss_clicked);
	connect(m->action_dismiss_all, &QAction::triggered, this, &GUI_Broadcast::dismiss_all_clicked);
	connect(ui->combo_clients, combo_current_index_changed_int, this, &GUI_Broadcast::combo_changed);
	connect(ui->btn_retry, &QPushButton::clicked, this, &GUI_Broadcast::retry);

	set_status_label();

	Set::listen(SetNoDB::MP3enc_found, this, &GUI_Broadcast::mp3_enc_found);
}


void GUI_Broadcast::set_status_label()
{
	if(!is_ui_initialized()){
		return;
	}

	int n_listeners = ui->combo_clients->count();
	QString str_listeners;

	if(n_listeners == 1){
		str_listeners = tr("%1 listener").arg(n_listeners);
	}

	else{
		str_listeners = tr("%1 listeners").arg(n_listeners);
	}

	ui->lab_status->setText(str_listeners);
}


// finally connection is established
void GUI_Broadcast::connection_established(const QString& ip)
{
	if(!is_ui_initialized()){
		return;
	}

	ui->combo_clients->addItem(ip);
	set_status_label();
	ui->combo_clients->setCurrentIndex(ui->combo_clients->count() -1);
	m->action_dismiss_all->setEnabled(true);
}


void GUI_Broadcast::connection_closed(const QString& ip)
{
	if(!is_ui_initialized()){
		return;
	}

	if(ip.isEmpty()) {
		return;
	}

	sp_log(Log::Info) << "Connection closed: " << ip;

	int idx;
	for(idx=0; idx<ui->combo_clients->count(); idx++){
		if(ui->combo_clients->itemText(idx).contains(ip)){
			break;
		}
	}

	if(idx >= ui->combo_clients->count()){
		return;
	}

	ui->combo_clients->removeItem(idx);

	if(ui->combo_clients->count() == 0){
		m->action_dismiss->setEnabled(false);
		m->action_dismiss_all->setEnabled(false);
	}

	set_status_label();
}

void GUI_Broadcast::can_listen_changed(bool success)
{
	if(!is_ui_initialized()){
		return;
	}

	ui->lab_status->setVisible(success);
	ui->lab_error->setVisible(!success);
	ui->btn_retry->setVisible(!success);

	if(!success){
		QString msg = tr("Cannot broadcast on port %1").arg(_settings->get(Set::Broadcast_Port));
		msg += "\n" + tr("Maybe another application is using this port?");

		Message::warning(msg);
	}
}


void GUI_Broadcast::retry()
{
	m->server->restart();
}


void GUI_Broadcast::dismiss_at(int idx)
{
	if(!is_ui_initialized()){
		return;
	}

	QString ip = ui->combo_clients->itemText(idx);

	if(ip.startsWith("(d)")) return;

	ui->combo_clients->setItemText(idx, QString("(d) ") + ip);

	m->server->dismiss(idx);
}


void GUI_Broadcast::dismiss_clicked()
{
	int idx = ui->combo_clients->currentIndex();
	dismiss_at(idx);
	m->action_dismiss->setEnabled(false);
}


void GUI_Broadcast::dismiss_all_clicked()
{
	for(int idx = 0; idx <ui->combo_clients->count(); idx++){
		dismiss_at(idx);
	}

	m->action_dismiss_all->setEnabled(false);
	m->action_dismiss->setEnabled(false);
}


void GUI_Broadcast::combo_changed(int idx)
{
	Q_UNUSED(idx)
	QString text = ui->combo_clients->currentText();

	if(text.startsWith("(d)")){
		m->action_dismiss->setEnabled(false);
	}
	else{
		m->action_dismiss->setEnabled(true);
	}
}


void GUI_Broadcast::mp3_enc_found()
{
	if(!is_ui_initialized()){
		return;
	}

	bool active = _settings->get(SetNoDB::MP3enc_found);
	if(!active)
	{
		ui->combo_clients->hide();
		ui->lab_status->hide();
		ui->lab_error->setText(Lang::get(Lang::CannotFindLame));
	}

	else{
		ui->lab_error->hide();
		ui->btn_retry->hide();
	}

	m->action_dismiss->setVisible(active);
	m->action_dismiss_all->setVisible(active);
}
