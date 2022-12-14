/* GUI_AbstractStream.cpp */

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

#include "AbstractStationPlugin.h"
#include "GUI_ConfigureStation.h"

#include "Components/Streaming/Streams/AbstractStationHandler.h"
#include "Interfaces/PlaylistInterface.h"

#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Style.h"
#include "Gui/Utils/Widgets/ProgressBar.h"
#include "Gui/Utils/MenuTool/MenuToolButton.h"

#include "Utils/Algorithm.h"
#include "Utils/Logger/Logger.h"
#include "Utils/Language/Language.h"
#include "Utils/Message/Message.h"
#include "Utils/Settings/Settings.h"
#include "Utils/Streams/Station.h"

#include <QComboBox>
#include <QPushButton>

struct Gui::AbstractStationPlugin::Private
{
	QMap<QString, StationPtr> temporaryStations;
	ProgressBar* loadingBar = nullptr;
	QComboBox* comboStream = nullptr;
	QPushButton* btnPlay = nullptr;
	MenuToolButton* btnTool = nullptr;
	AbstractStationHandler* streamHandler = nullptr;
	PlaylistCreator* playlistCreator;
	GUI_ConfigureStation* configDialog = nullptr;

	bool searching;

	Private(PlaylistCreator* playlistCreator) :
		playlistCreator(playlistCreator),
		searching(false) {}

	~Private()
	{
		if(configDialog)
		{
			configDialog->deleteLater();
		}
	}

	QString currentName() const
	{
		return comboStream->currentText();
	}

	QString currentUrl() const
	{
		return comboStream->currentData().toString();
	}

	void setSearching(bool isSearching)
	{
		const auto text = isSearching
		                  ? Lang::get(Lang::Stop)
		                  : Lang::get(Lang::Listen);

		btnPlay->setText(text);
		btnPlay->setDisabled(false);
		loadingBar->setVisible(isSearching);
		searching = isSearching;
	}
};

Gui::AbstractStationPlugin::AbstractStationPlugin(PlaylistCreator* playlistCreator, QWidget* parent) :
	PlayerPlugin::Base(parent)
{
	m = Pimpl::make<Private>(playlistCreator);
}

Gui::AbstractStationPlugin::~AbstractStationPlugin() = default;

void Gui::AbstractStationPlugin::initConnections()
{
	m->btnPlay->setFocusPolicy(Qt::StrongFocus);

	m->btnTool->showActions(ContextMenuEntries(ContextMenu::EntryNew));
	m->btnTool->registerPreferenceAction(new StreamPreferenceAction(m->btnTool));
	m->btnTool->registerPreferenceAction(new StreamRecorderPreferenceAction(m->btnTool));

	setTabOrder(m->comboStream, m->btnPlay);

	connect(m->btnPlay, &QPushButton::clicked, this, &AbstractStationPlugin::listenClicked);
	connect(m->btnTool, &MenuToolButton::sigEdit, this, &AbstractStationPlugin::editClicked);
	connect(m->btnTool, &MenuToolButton::sigDelete, this, &AbstractStationPlugin::deleteClicked);
	connect(m->btnTool, &MenuToolButton::sigNew, this, &AbstractStationPlugin::newClicked);
	connect(m->btnTool, &MenuToolButton::sigSave, this, &AbstractStationPlugin::saveClicked);

	connect(m->comboStream, combo_activated_int, this, &AbstractStationPlugin::currentIndexChanged);
	connect(m->streamHandler, &AbstractStationHandler::sigError, this, &AbstractStationPlugin::error);
	connect(m->streamHandler, &AbstractStationHandler::sigDataAvailable, this, &AbstractStationPlugin::dataAvailable);
	connect(m->streamHandler, &AbstractStationHandler::sigStopped, this, &AbstractStationPlugin::stopped);
}

void Gui::AbstractStationPlugin::initUi()
{
	m->configDialog = createConfigDialog();
	m->configDialog->init_ui();

	connect(m->configDialog, &Gui::Dialog::accepted, this, &AbstractStationPlugin::configFinished);
	connect(m->configDialog, &Gui::Dialog::rejected, this, &AbstractStationPlugin::configFinished);

	m->streamHandler = streamHandler();
	connect(m->streamHandler, &AbstractStationHandler::sigUrlCountExceeded,
	        this, &AbstractStationPlugin::urlCountExceeded);

	m->loadingBar = new ProgressBar(this);

	initConnections();
	setupStations();
	skinChanged();

	m->setSearching(false);
}

void Gui::AbstractStationPlugin::setupStations()
{
	const auto lastName = m->currentName();
	const auto lastUrl = m->currentUrl();
	auto stations = QList<StationPtr> {};

	m->btnPlay->setEnabled(false);
	m->comboStream->clear();
	m->streamHandler->getAllStreams(stations);

	for(const auto& station : stations)
	{
		m->comboStream->addItem(station->name(), station->url());
	}

	auto index = Util::Algorithm::indexOf(stations, [&](const auto& station) {
		return (lastName == station->name()) ||
		       (lastUrl == station->url());
	});

	if(m->comboStream->count() > 0)
	{
		index = std::max(0, index);
	}

	if(index >= 0)
	{
		m->comboStream->setCurrentIndex(index);
		currentIndexChanged(m->comboStream->currentIndex());
	}
}

void Gui::AbstractStationPlugin::currentIndexChanged(int index)
{
	const auto currentName = m->currentName();
	const auto currentUrl = m->currentUrl();
	const auto isTemporary = m->temporaryStations.contains(currentName);
	const auto isEditable = ((index >= 0) && !isTemporary);
	const auto isListentDisabled = ((currentUrl.size() < 8) && !m->searching);

	m->btnTool->showAction(ContextMenu::EntrySave, isTemporary);
	m->btnTool->showAction(ContextMenu::EntryEdit, isEditable);
	m->btnTool->showAction(ContextMenu::EntryDelete, (index >= 0));
	m->comboStream->setToolTip(currentUrl);
	m->btnPlay->setDisabled(isListentDisabled);
}

void Gui::AbstractStationPlugin::dataAvailable()
{
	m->setSearching(false);
}

void Gui::AbstractStationPlugin::listenClicked()
{
	if(m->searching)
	{
		m->btnPlay->setDisabled(true);
		m->streamHandler->stop();

		return;
	}

	if(m->currentUrl().size() > 5)
	{
		const auto currentName = m->currentName();
		const auto stationName = currentName.isEmpty()
		                         ? titleFallbackName()
		                         : currentName;

		m->setSearching(true);
		play(stationName);

		return;
	}

	spLog(Log::Warning, this) << "Url is empty";
}

void Gui::AbstractStationPlugin::play(const QString& stationName)
{
	auto stationPtr = m->streamHandler->station(stationName);
	if(!stationPtr)
	{
		stationPtr = m->temporaryStations[stationName];
		if(!stationPtr)
		{
			return;
		}
	}

	bool success = m->streamHandler->parseStation(stationPtr);
	if(!success)
	{
		spLog(Log::Warning, this) << "Stream Handler busy";
		m->setSearching(false);
	}
}

void Gui::AbstractStationPlugin::stopped()
{
	m->setSearching(false);
}

void Gui::AbstractStationPlugin::error()
{
	m->setSearching(false);

	const auto question = QString("%1\n%2\n\n%3")
		.arg(tr("Cannot open stream"))
		.arg(m->currentUrl())
		.arg(Lang::get(Lang::Retry).question());

	const auto answer = Message::question_yn(question);
	if(answer == Message::Answer::Yes)
	{
		listenClicked();
	}

	else
	{
		const auto removed = (m->temporaryStations.remove(m->currentName()) == 0);
		if(removed)
		{
			m->comboStream->removeItem(m->comboStream->currentIndex());
		}
	}
}

int Gui::AbstractStationPlugin::addStream(const QString& name, const QString& url)
{
	const auto stationPtr = m->streamHandler->createStreamInstance(name, url);

	m->temporaryStations.insert(name, stationPtr);
	m->comboStream->addItem(name, url);
	m->comboStream->setCurrentText(name);

	currentIndexChanged(m->comboStream->currentIndex());

	return m->comboStream->findText(name);
}

void Gui::AbstractStationPlugin::newClicked()
{
	m->configDialog->setMode(titleFallbackName(), GUI_ConfigureStation::Mode::New);
	m->configDialog->configureWidgets(nullptr);
	m->configDialog->open();
}

void Gui::AbstractStationPlugin::saveClicked()
{
	const auto station = m->streamHandler->createStreamInstance(m->currentName(), m->currentUrl());

	m->configDialog->setMode(m->currentName(), GUI_ConfigureStation::Mode::Save);
	m->configDialog->configureWidgets(station);
	m->configDialog->open();
}

void Gui::AbstractStationPlugin::editClicked()
{
	const auto station = m->streamHandler->station(m->currentName());
	if(station)
	{
		m->configDialog->setMode(station->name(), GUI_ConfigureStation::Mode::Edit);
		m->configDialog->configureWidgets(station);
		m->configDialog->open();
	}
}

void Gui::AbstractStationPlugin::configFinished()
{
	auto* configDialog = dynamic_cast<GUI_ConfigureStation*>(sender());
	if(!configDialog->isAccepted())
	{
		return;
	}

	const auto station = configDialog->configuredStation();
	const auto mode = configDialog->mode();
	if(mode == GUI_ConfigureStation::Mode::New)
	{
		const auto stationIndex = m->comboStream->findText(station->name());
		if(stationIndex >= 0)
		{
			configDialog->setError(tr("Please choose another name"));
			configDialog->open();
			return;
		}

		const auto streamAdded = m->streamHandler->addNewStream(station);
		if(streamAdded)
		{
			m->temporaryStations.remove(station->name());
		}
	}

	else if(mode == GUI_ConfigureStation::Mode::Edit || mode == GUI_ConfigureStation::Mode::Save)
	{
		bool success;
		if(m->temporaryStations.contains(m->currentName()))
		{
			success = m->streamHandler->addNewStream(station);
			if(success)
			{
				m->temporaryStations.remove(m->currentName());
			}
		}

		else
		{
			success = m->streamHandler->update(m->currentName(), station);
		}

		spLog(Log::Info, this) << "Updated " << m->currentName() << ": " << success;
	}

	setupStations();
}

void Gui::AbstractStationPlugin::deleteClicked()
{
	const auto answer = Message::question_yn(tr("Do you really want to delete %1").arg(m->currentName()) + "?");
	if(answer == Message::Answer::Yes)
	{
		const auto success = m->streamHandler->deleteStream(m->currentName());
		spLog(Log::Info, this) << "Delete " << m->currentName() << success;
	}

	setupStations();
}

void Gui::AbstractStationPlugin::urlCountExceeded(int urlCount, int maxUrlCount)
{
	Message::error(QString("Found %1 urls").arg(urlCount) + "<br />" +
	               QString("Maximum number is %1").arg(maxUrlCount)
	);

	m->setSearching(false);
}

bool Gui::AbstractStationPlugin::hasLoadingBar() const
{
	return true;
}

void Gui::AbstractStationPlugin::assignUiVariables()
{
	m->comboStream = comboStream();
	m->btnPlay = btnPlay();
	m->btnTool = btnMenu();
}

void Gui::AbstractStationPlugin::retranslate()
{
	const auto text = (m->searching) ?
	                  Lang::get(Lang::Stop) : Lang::get(Lang::Listen);

	m->btnPlay->setText(text);
}

void Gui::AbstractStationPlugin::skinChanged()
{
	if(isUiInitialized())
	{
		m->setSearching(m->searching);
		btnPlay()->setIcon(Gui::Icons::icon(Gui::Icons::Play));
	}
}

Gui::StreamPreferenceAction::StreamPreferenceAction(QWidget* parent) :
	PreferenceAction(QString(Lang::get(Lang::Streams) + " && " + Lang::get(Lang::Podcasts)), identifier(), parent) {}

Gui::StreamPreferenceAction::~StreamPreferenceAction() = default;

QString Gui::StreamPreferenceAction::identifier() const { return "streams"; }

QString Gui::StreamPreferenceAction::displayName() const
{
	return Lang::get(Lang::Streams) + " && " + Lang::get(Lang::Podcasts);
}

