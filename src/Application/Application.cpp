/* application.cpp */

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

// need this here because of SAYONARA ENV variables
#include "Utils/Macros.h"

#include "Application/Application.h"
#include "Application/InstanceThread.h"
#include "Application/MetaTypeRegistry.h"
#include "Application/LocalLibraryWatcher.h"

#ifdef SAYONARA_WITH_DBUS

#include "DBus/DBusHandler.h"

#endif

#ifdef Q_OS_WIN
#include <windows.h>
#include "3rdParty/SomaFM/ui/SomaFMLibraryContainer.h"
#include "3rdParty/Soundcloud/ui/GUI_SoundcloudLibrary.h"
#endif

#include "Components/Playlist/PlaylistHandler.h"
#include "Components/Playlist/ExternTracksPlaylistGenerator.h"
#include "Components/RemoteControl/RemoteControl.h"
#include "Components/Engine/EngineHandler.h"
#include "Components/PlayManager/PlayManager.h"
#include "Components/Streaming/LastFM/LastFM.h"
#include "Components/Session/Session.h"
#include "Components/LibraryManagement/LibraryPluginHandler.h"

#include "Interfaces/Notification/NotificationHandler.h"

#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Style.h"
#include "Gui/Utils/Icons.h"

#include "Gui/Player/GUI_Player.h"
#include "Gui/Library/LocalLibraryContainer.h"
#include "Gui/Library/EmptyLibraryContainer.h"
#include "Gui/Soundcloud/SoundcloudLibraryContainer.h"
#include "Gui/SomaFM/SomaFMLibraryContainer.h"

#include "Gui/History/HistoryContainer.h"

#include "Gui/Plugins/PlayerPluginHandler.h"
#include "Gui/Plugins/PlaylistChooser/GUI_PlaylistChooser.h"
#include "Gui/Plugins/AudioConverter/GUI_AudioConverter.h"
#include "Gui/Plugins/Bookmarks/GUI_Bookmarks.h"
#include "Gui/Plugins/Broadcasting/GUI_Broadcast.h"
#include "Gui/Plugins/Engine/GUI_LevelPainter.h"
#include "Gui/Plugins/Engine/GUI_Spectrum.h"
#include "Gui/Plugins/Engine/GUI_Equalizer.h"
#include "Gui/Plugins/Engine/GUI_Speed.h"
#include "Gui/Plugins/Engine/GUI_Crossfader.h"
#include "Gui/Plugins/Engine/GUI_SpectrogramPainter.h"
#include "Gui/Plugins/Stream/GUI_Stream.h"
#include "Gui/Plugins/Stream/GUI_Podcasts.h"

#include "Gui/Preferences/Broadcast/GUI_BroadcastPreferences.h"
#include "Gui/Preferences/Covers/GUI_CoverPreferences.h"
#include "Gui/Preferences/Engine/GUI_EnginePreferences.h"
#include "Gui/Preferences/UiPreferences/GUI_UiPreferences.h"
#include "Gui/Preferences/Language/GUI_LanguagePreferences.h"
#include "Gui/Preferences/LastFM/GUI_LastFmPreferences.h"
#include "Gui/Preferences/Library/GUI_LibraryPreferences.h"
#include "Gui/Preferences/Notifications/GUI_NotificationPreferences.h"
#include "Gui/Preferences/Player/GUI_PlayerPreferences.h"
#include "Gui/Preferences/Playlist/GUI_PlaylistPreferences.h"
#include "Gui/Preferences/PreferenceDialog/GUI_PreferenceDialog.h"
#include "Gui/Preferences/Proxy/GUI_ProxyPreferences.h"
#include "Gui/Preferences/RemoteControl/GUI_RemoteControlPreferences.h"
#include "Gui/Preferences/Search/GUI_SearchPreferences.h"
#include "Gui/Preferences/Shortcuts/GUI_ShortcutPreferences.h"
#include "Gui/Preferences/Streams/GUI_StreamPreferences.h"
#include "Gui/Preferences/StreamRecorder/GUI_StreamRecorderPreferences.h"

#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"
#include "Utils/WebAccess/Proxy.h"

#include "Utils/Language/Language.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/MetaData/MetaDataSorting.h"

#include "Database/Connector.h"
#include "Database/Settings.h"

#include <QIcon>
#include <QTime>
#include <QDateTime>
#include <QSessionManager>

class MeasureApp
{
		QTime* m_t;
		QString m_component;
		int m_start;

	public:
		MeasureApp(const QString& component, QTime* t) :
			m_t(t),
			m_component(component)
		{
			m_start = m_t->elapsed();
			spLog(Log::Debug, this) << "Init " << m_component << ": " << m_start << "ms";
		}

		~MeasureApp()
		{
			int end = m_t->elapsed();
			spLog(Log::Debug, this) << "Init " << m_component << " finished: " << end << "ms (" << end - m_start
			                        << "ms)";
		}
};

#define measure(c) MeasureApp mt(c, m->timer); Q_UNUSED(mt);

struct Application::Private
{
	QTime* timer = nullptr;
	GUI_Player* player = nullptr;

	RemoteControl* remoteControl = nullptr;
	DB::Connector* db = nullptr;
	InstanceThread* instanceThread = nullptr;
	MetaTypeRegistry* metatypeRegistry = nullptr;
	Session::Manager* session = nullptr;

	bool shutdownTriggered;

	Private(Application* app)
	{
		Q_UNUSED(app)

		metatypeRegistry = new MetaTypeRegistry();
		qRegisterMetaType<uint64_t>("uint64_t");

		/* Tell the settings manager which settings are necessary */
		db = DB::Connector::instance();
		db->settingsConnector()->loadSettings();

		session = Session::Manager::instance();

		Gui::Icons::setSystemTheme(QIcon::themeName());
		Gui::Icons::forceStandardIcons(GetSetting(Set::Icon_ForceInDarkTheme));

		if(!Settings::instance()->checkSettings())
		{
			spLog(Log::Error, this) << "Cannot initialize settings";
			return;
		}

		Settings::instance()->applyFixes();

		init_resources();

		timer = new QTime();
		shutdownTriggered = false;
	}

	~Private()
	{
		if(timer)
		{
			delete timer;
			timer = nullptr;
		}

		if(instanceThread)
		{
			instanceThread->stop();
			while(instanceThread->isRunning())
			{
				Util::sleepMs(100);
			}

			instanceThread = nullptr;
		}

		if(player)
		{
			delete player;
			player = nullptr;
		}

		if(db)
		{
			db->settingsConnector()->storeSettings();
			db->closeDatabase();
			db = nullptr;
		}

		if(metatypeRegistry)
		{
			delete metatypeRegistry;
			metatypeRegistry = nullptr;
		}
	}

	void init_resources()
	{
		Q_INIT_RESOURCE(Broadcasting);
		Q_INIT_RESOURCE(Database);
		Q_INIT_RESOURCE(Icons);
		Q_INIT_RESOURCE(Lyrics);
		Q_INIT_RESOURCE(Resources);

#ifdef Q_OS_WIN
		Q_INIT_RESOURCE(IconsWindows);
#endif
	}
};

#ifdef Q_OS_WIN
void global_key_handler()
{
	if(!RegisterHotKey(NULL, 1, MOD_NOREPEAT, VK_MEDIA_PLAY_PAUSE)){
		return false;
	}

	MSG msg = {0};
	while (GetMessage(&msg, NULL, 0, 0) != 0)
	{
		if (msg.message == WM_HOTKEY)
		{
			UINT modifiers = msg.lParam;
			UINT key = msg.wParam;
		}
	}
}
#endif

Application::Application(int& argc, char** argv) :
	QApplication(argc, argv)
{
	m = Pimpl::make<Private>(this);
	m->timer->start();

	this->setQuitOnLastWindowClosed(false);
}

Application::~Application()
{
	if(!m->shutdownTriggered)
	{
		shutdown();
	}
}

bool Application::init(const QStringList& files_to_play, bool force_show)
{
	{
		measure("Settings")

		QString version = QString(SAYONARA_VERSION);
		SetSetting(Set::Player_Version, version);
	}

	{
		measure("Theme")
		Gui::Icons::changeTheme();
	}

	{
		measure("Proxy")
		Proxy::init();
	}

	initEngine();
	initPlayer(force_show);

#ifdef SAYONARA_WITH_DBUS
	{
		measure("DBUS")
		new DBusHandler(m->player, this);
	}
#endif

	{
		ListenSetting(Set::Remote_Active, Application::remoteControlActivated);
	}

	if(GetSetting(Set::Notification_Show))
	{
		NotificationHandler::instance()->notify("Sayonara Player",
		                                        Lang::get(Lang::Version) + " " + SAYONARA_VERSION,
		                                        QString(":/Icons/logo.png")
		);
	}

	initLibraries();
	initPlugins();
	initPreferences();

	initPlaylist(files_to_play);
	initSingleInstanceThread();
	spLog(Log::Debug, this) << "Initialized: " << m->timer->elapsed() << "ms";
	delete m->timer;
	m->timer = nullptr;

	//connect(this, &Application::commitDataRequest, this, &Application::session_end_requested);

	ListenSetting(SetNoDB::Player_MetaStyle, Application::skinChanged);

	if(!GetSetting(Set::Player_StartInTray))
	{
		m->player->show();
	}

	else
	{
		m->player->hide();
	}

	return true;
}

void Application::initPlayer(bool force_show)
{
	measure("Player")

	if(force_show)
	{
		SetSetting(Set::Player_StartInTray, false);
	}

	m->player = new GUI_Player();
	Gui::Util::setMainWindow(m->player);

	connect(m->player, &GUI_Player::sigClosed, this, &QCoreApplication::quit);
}

void Application::initPlaylist(const QStringList& filesToPlay)
{
	auto* plh = Playlist::Handler::instance();
	plh->loadOldPlaylists();

	if(!filesToPlay.isEmpty())
	{
		const QString playlistName = plh->requestNewPlaylistName();
		plh->createPlaylist(filesToPlay, playlistName);
	}
}

void Application::initPreferences()
{
	measure("Preferences")

	auto* preferences = new GUI_PreferenceDialog(m->player);

	preferences->registerPreferenceDialog(new GUI_PlayerPreferences("application"));
	preferences->registerPreferenceDialog(new GUI_LanguagePreferences("language"));
	preferences->registerPreferenceDialog(new GUI_UiPreferences("user-interface"));
	preferences->registerPreferenceDialog(new GUI_ShortcutPreferences("shortcuts"));

	preferences->registerPreferenceDialog(new GUI_PlaylistPreferences("playlist"));
	preferences->registerPreferenceDialog(new GUI_LibraryPreferences("library"));
	preferences->registerPreferenceDialog(new GUI_CoverPreferences("covers"));
	preferences->registerPreferenceDialog(new GUI_EnginePreferences("engine"));
	preferences->registerPreferenceDialog(new GUI_SearchPreferences("search"));

	preferences->registerPreferenceDialog(new GUI_ProxyPreferences("proxy"));
	preferences->registerPreferenceDialog(new GUI_StreamPreferences("streams"));
	preferences->registerPreferenceDialog(new GUI_StreamRecorderPreferences("streamrecorder"));
	preferences->registerPreferenceDialog(new GUI_BroadcastPreferences("broadcast"));
	preferences->registerPreferenceDialog(new GUI_RemoteControlPreferences("remotecontrol"));

	preferences->registerPreferenceDialog(new GUI_NotificationPreferences("notifications"));
	preferences->registerPreferenceDialog(new GUI_LastFmPreferences("lastfm", new LastFM::Base()));

	m->player->registerPreferenceDialog(preferences->action());
}

void Application::initLibraries()
{
	measure("Libraries")

	auto* local_library_watcher = new Library::LocalLibraryWatcher(this);
	auto* library_plugin_loader = Library::PluginHandler::instance();

	QList<Library::AbstractContainer*> library_containers = local_library_watcher->getLocalLibraryContainers();
	//auto* directory_container = new Library::DirectoryContainer(this);
	auto* soundcloud_container = new SC::LibraryContainer(this);
	auto* somafm_container = new SomaFM::LibraryContainer(this);
	auto* history_container = new HistoryContainer(this);

	//library_containers << static_cast<Library::Container*>(directory_container);
	library_containers << static_cast<Library::AbstractContainer*>(somafm_container);
	library_containers << static_cast<Library::AbstractContainer*>(soundcloud_container);
	library_containers << static_cast<Library::AbstractContainer*>(history_container);

	library_plugin_loader->init(library_containers, new EmptyLibraryContainer());
}

void Application::initEngine()
{
	measure("Engine")
	Engine::Handler::instance()->isValid();
}

void Application::initPlugins()
{
	measure("Plugins")

	PlayerPlugin::Handler* pph = PlayerPlugin::Handler::instance();

	pph->addPlugin(new GUI_LevelPainter());
	pph->addPlugin(new GUI_Spectrum());
	pph->addPlugin(new GUI_Equalizer());
	pph->addPlugin(new GUI_Stream());
	pph->addPlugin(new GUI_Podcasts());
	pph->addPlugin(new GUI_PlaylistChooser());
	pph->addPlugin(new GUI_AudioConverter());
	pph->addPlugin(new GUI_Bookmarks());
	pph->addPlugin(new GUI_Speed());
	pph->addPlugin(new GUI_Broadcast());
	pph->addPlugin(new GUI_Crossfader());
	pph->addPlugin(new GUI_SpectrogramPainter());

	spLog(Log::Debug, this) << "Plugins finished: " << m->timer->elapsed() << "ms";
}

void Application::initSingleInstanceThread()
{
	m->instanceThread = new InstanceThread(this);

	connect(m->instanceThread, &InstanceThread::sigPlayerRise, m->player, &GUI_Player::raise);
	connect(m->instanceThread, &InstanceThread::sigCreatePlaylist, this, &Application::createPlaylist);

	m->instanceThread->start();
}

void Application::sessionEndRequested(QSessionManager& manager)
{
	Q_UNUSED(manager)
	shutdown();

	if(m->db)
	{
		m->db->settingsConnector()->storeSettings();
		m->db->closeDatabase();
	}

	if(m->player)
	{
		m->player->requestShutdown();
	}
}

void Application::shutdown()
{
	PlayerPlugin::Handler::instance()->shutdown();
	Engine::Handler::instance()->shutdown();
	Playlist::Handler::instance()->shutdown();
	PlayManager::instance()->shutdown();

	m->shutdownTriggered = true;
}

void Application::remoteControlActivated()
{
	if(GetSetting(Set::Remote_Active) && !m->remoteControl)
	{
		m->remoteControl = new RemoteControl(this);
	}
}

void Application::createPlaylist()
{
	auto* instanceThread = dynamic_cast<InstanceThread*>(sender());
	if(!instanceThread)
	{
		return;
	}

	const QStringList paths = instanceThread->paths();

	auto* playlistGenerator = ExternTracksPlaylistGenerator::instance();
	playlistGenerator->addPaths(paths);

	if(playlistGenerator->isPlayAllowed())
	{
		playlistGenerator->changeTrack();
	}
}

void Application::skinChanged()
{
	Style::applyCurrentStyle(this);
}
