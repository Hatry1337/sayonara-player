#include "GUI_ControlsBase.h"
#include "SearchSlider.h"

#include "Gui/Covers/CoverButton.h"

#include "Gui/Utils/GuiUtils.h"
#include "Gui/Utils/Style.h"
#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Shortcuts/Shortcut.h"
#include "Gui/Utils/Shortcuts/ShortcutHandler.h"
#include "Gui/Utils/PreferenceAction.h"
#include "Gui/Utils/ContextMenu/LibraryContextMenu.h"
#include "Gui/Utils/Widgets/RatingLabel.h"
#include "Gui/Utils/Widgets/FloatingLabel.h"

#include "Interfaces/PlayManager.h"
#include "Components/Covers/CoverLocation.h"
#include "Components/Engine/EngineHandler.h"
#include "Components/PlayManager/PlayManagerProvider.h"
#include "Components/Tagging/ChangeNotifier.h"

#include "Utils/Algorithm.h"
#include "Utils/Utils.h"
#include "Utils/Settings/Settings.h"
#include "Utils/MetaData/MetaDataList.h"
#include "Utils/Logger/Logger.h"

#include <QToolTip>
#include <QImage>
#include <QIcon>
#include <QDateTime>

#include <cmath>

namespace
{
	void setIcon(QPushButton* btn, const QIcon& icon)
	{
		const auto width = Gui::Util::textWidth(btn->fontMetrics(), "MMn");

		auto sz = QSize(width, width);
		btn->setFixedSize(sz);

		sz.setWidth((width * 800) / 1000);
		sz.setHeight((width * 800) / 1000);

		btn->setIconSize(sz);
		btn->setIcon(icon);
	}

	QString getPositionStringFromSliderValue(int sliderValue, int max, MilliSeconds duration)
	{
		sliderValue = std::max(sliderValue, 0);
		sliderValue = std::min(max, sliderValue);

		const auto percent = (sliderValue * 1.0) / max;

		const auto currentPositionMs = static_cast<MilliSeconds>(percent * duration);
		return Util::msToString(currentPositionMs, "$M:$S");
	}

	void setFloatingText(QLabel* label, const QString& text)
	{
		auto* floatingLabel = dynamic_cast<Gui::FloatingLabel*>(label);
		if(floatingLabel)
		{
			floatingLabel->setFloatingText(text);
		}
		else
		{
			label->setText(text);
		}

	}
}

struct GUI_ControlsBase::Private
{
	Library::ContextMenu* contextMenu = nullptr;
	PlayManager* playManager;

	Private() :
		playManager(PlayManagerProvider::instance()->playManager()) {}
};

GUI_ControlsBase::GUI_ControlsBase(QWidget* parent) :
	Gui::Widget(parent)
{
	m = Pimpl::make<Private>();
}

GUI_ControlsBase::~GUI_ControlsBase() = default;

void GUI_ControlsBase::init()
{
	const auto version = GetSetting(Set::Player_Version);

	labSayonara()->setText(tr("Sayonara Player"));
	labVersion()->setText(version);
	labWrittenBy()->setText(tr("Written by %1").arg("Michael Lugmair"));
	labCopyright()->setText(tr("Copyright") + " 2011 - " + QString::number(QDateTime::currentDateTime().date().year()));
	btnRecord()->setVisible(false);

	volumeChanged(m->playManager->volume());
	muteChanged(m->playManager->isMuted());

	const auto playstate = m->playManager->playstate();
	playstateChanged(playstate);
	if(playstate != PlayState::Stopped)
	{
		currentTrackChanged(m->playManager->currentTrack());
		currentPositionChanged(m->playManager->initialPositionMs());
	}

	setupConnections();
	setupShortcuts();

	btnCover()->setAlternativeSearchEnabled(false);
	connect(btnCover(), &Gui::CoverButton::sigRejected, this, &GUI_ControlsBase::coverClickRejected);

	ListenSetting(Set::Engine_SR_Active, GUI_ControlsBase::streamRecorderActiveChanged);
	ListenSettingNoCall(Set::Engine_Pitch, GUI_ControlsBase::refreshCurrentTrack);
	ListenSettingNoCall(Set::Engine_SpeedActive, GUI_ControlsBase::refreshCurrentTrack);

	skinChanged();
}

Gui::RatingEditor* GUI_ControlsBase::labRating() const
{
	return nullptr;
}

QSize GUI_ControlsBase::buttonSize() const
{
	return btnCover()->size();
}

// new track
void GUI_ControlsBase::currentTrackChanged(const MetaData& track)
{
	labSayonara()->hide();
	labVersion()->hide();
	labWrittenBy()->hide();
	labCopyright()->hide();

	labTitle()->show();
	labArtist()->show();
	labAlbum()->show();
	widgetDetails()->show();

	refreshCurrentPosition(0);
	refreshLabels(track);

	setCoverLocation(track);
	setRadioMode(track.radioMode());

	sliProgress()->setEnabled((track.durationMs() / 1000) > 0);
}

void GUI_ControlsBase::playstateChanged(PlayState state)
{
	switch(state)
	{
		case PlayState::Stopped:
			stopped();
			break;
		case PlayState::Playing:
			played();
			break;
		case PlayState::Paused:
			paused();
			break;
		default:
			paused();
			break;
	}

	checkRecordButtonVisible();
}

QIcon GUI_ControlsBase::icon(Gui::Icons::IconName name)
{
	using namespace Gui;

	Icons::changeTheme();
	return (Style::isDark())
	       ? Icons::icon(name, Icons::ForceSayonaraIcon)
	       : Icons::icon(name, Icons::Automatic);
}

void GUI_ControlsBase::played()
{
	labCurrentTime()->setVisible(true);
	btnPlay()->setIcon(icon(Gui::Icons::Pause));
}

void GUI_ControlsBase::paused()
{
	labCurrentTime()->setVisible(true);
	btnPlay()->setIcon(icon(Gui::Icons::Play));
}

void GUI_ControlsBase::stopped()
{
	setWindowTitle("Sayonara");

	btnPlay()->setIcon(icon(Gui::Icons::Play));
	sliProgress()->set_buffering(-1);

	labTitle()->hide();
	labArtist()->hide();
	labAlbum()->hide();
	widgetDetails()->hide();

	labCurrentTime()->setText("00:00");
	labCurrentTime()->hide();
	labMaxTime()->clear();
	labMaxTime()->setVisible(false);

	labSayonara()->show();
	labWrittenBy()->show();
	labVersion()->show();
	labCopyright()->show();

	sliProgress()->setValue(0);
	sliProgress()->setEnabled(false);

	setStandardCover();

	if(labRating())
	{
		labRating()->hide();
	}
}

void GUI_ControlsBase::recordChanged(bool b)
{
	btnRecord()->setChecked(b);
}

void GUI_ControlsBase::buffering(int progress)
{
	// buffering
	if(progress > 0 && progress < 100)
	{
		sliProgress()->set_buffering(progress);

		labCurrentTime()->setText(QString("%1%").arg(progress));
		labMaxTime()->setVisible(false);
	}

		//buffering stopped
	else if(progress == 0)
	{
		sliProgress()->set_buffering(-1);
		labMaxTime()->setVisible(false);
	}

		// no buffering
	else
	{
		sliProgress()->set_buffering(-1);
		labMaxTime()->setVisible(m->playManager->currentTrack().durationMs() > 0);
	}
}

void GUI_ControlsBase::progressMoved(int val)
{
	val = std::max(val, 0);

	refreshCurrentPosition(val);

	const auto percent = (val * 1.0) / sliProgress()->maximum();
	m->playManager->seekRelative(percent);
}

void GUI_ControlsBase::currentPositionChanged(MilliSeconds posMs)
{
	spLog(Log::Crazy, this) << "Current position: " << posMs;

	const auto duration = m->playManager->durationMs();
	const auto max = sliProgress()->maximum();
	const auto percent = (posMs * 1.0) / duration;

	auto newValue = 0;
	if(duration > 0)
	{
		newValue = static_cast<int>(max * percent);
	}

	else if(posMs > duration)
	{
		newValue = 0;
	}

	else
	{
		return;
	}

	if(!sliProgress()->is_busy())
	{
		const auto currentPositionString = Util::msToString(posMs, "$M:$S");
		labCurrentTime()->setText(currentPositionString);
		sliProgress()->setValue(newValue);
	}
}

void GUI_ControlsBase::refreshCurrentPosition(int val)
{
	const auto text = getPositionStringFromSliderValue(val, sliProgress()->maximum(), m->playManager->durationMs());
	labCurrentTime()->setText(text);
}

void GUI_ControlsBase::setTotalTimeLabel(MilliSeconds totalTimeMs)
{
	QString lengthStr;
	if(totalTimeMs > 0)
	{
		lengthStr = Util::msToString(totalTimeMs, "$M:$S");
		labMaxTime()->setText(lengthStr);
	}

	labMaxTime()->setVisible(totalTimeMs > 0);
	sliProgress()->setEnabled(totalTimeMs > 0);
}

void GUI_ControlsBase::progressHovered(int val)
{
	const auto text = getPositionStringFromSliderValue(val, sliProgress()->maximum(), m->playManager->durationMs());
	QToolTip::showText(QCursor::pos(), text);
}

void GUI_ControlsBase::volumeChanged(int val)
{
	setupVolumeButton(val);
	sliVolume()->setValue(val);
}

void GUI_ControlsBase::setupVolumeButton(int percent)
{
	using namespace Gui;

	if(percent <= 1 || m->playManager->isMuted())
	{
		setIcon(btnMute(), Icons::icon(Icons::VolMute));
	}

	else if(percent < 40)
	{
		setIcon(btnMute(), Icons::icon(Icons::Vol1));
	}

	else if(percent < 80)
	{
		setIcon(btnMute(), Icons::icon(Icons::Vol2));
	}

	else
	{
		setIcon(btnMute(), Icons::icon(Icons::Vol3));
	}
}

void GUI_ControlsBase::increaseVolume()
{
	m->playManager->volumeUp();
}

void GUI_ControlsBase::decreaseVolume()
{
	m->playManager->volumeDown();
}

void GUI_ControlsBase::changeVolumeByDelta(int val)
{
	if(val > 0)
	{
		increaseVolume();
	}

	else
	{
		decreaseVolume();
	}
}

void GUI_ControlsBase::muteChanged(bool muted)
{
	const auto volume = (muted) ? 0 : m->playManager->volume();

	sliVolume()->setValue(volume);
	setupVolumeButton(volume);

	sliVolume()->setDisabled(muted);
}

// public slot:
// id3 tags have changed
void GUI_ControlsBase::metadataChanged()
{
	const auto& changedTracks = Tagging::ChangeNotifier::instance()->changedMetadata();
	const auto& currentTrack = m->playManager->currentTrack();

	const auto it = Util::Algorithm::find(changedTracks, [&currentTrack](const MetaDataPair& trackPair) {
		const MetaData& oldTrack = trackPair.first;
		return (oldTrack.filepath() == currentTrack.filepath());
	});

	if(it != changedTracks.end())
	{
		const auto& newTrack = it->second;
		refreshLabels(newTrack);
		setCoverLocation(newTrack);

		setWindowTitle(QString("Sayonara - %1").arg(newTrack.title()));
	}
}

void GUI_ControlsBase::refreshCurrentTrack()
{
	refreshLabels(m->playManager->currentTrack());
}

void GUI_ControlsBase::refreshLabels(const MetaData& md)
{
	// title, artist
	setFloatingText(labTitle(), md.title());
	setFloatingText(labArtist(), md.artist());

	{ //album
		const auto sYear = QString::number(md.year());
		auto albumName = md.album();

		labAlbum()->setToolTip("");
		if(md.year() > 1000 && (!albumName.contains(sYear)))
		{
			albumName += QString(" (%1)").arg(md.year());
		}

		else if(md.radioMode() == RadioMode::Station)
		{
			labAlbum()->setToolTip(md.filepath());
		}

		setFloatingText(labAlbum(), albumName);
	}

	{ // bitrate
		const auto text = QString("%1 kBit/s").arg(std::nearbyint(md.bitrate() / 1000.0));
		labBitrate()->setText(text);
		labBitrate()->setVisible(md.bitrate() / 1000 > 0);
	}

	{ // filesize
		const auto filesizeMB = (md.filesize() / 1024) / 1024.0;
		const auto text = QString::number(filesizeMB, 'f', 2) + " MB";
		labFilesize()->setText(text);
		labFilesize()->setVisible(md.filesize() > 0);
	}

	{ // rating
		if(labRating())
		{
			labRating()->setVisible(md.radioMode() == RadioMode::Off);
			labRating()->setRating(md.rating());
		}
	}

	setTotalTimeLabel(md.durationMs());
}

void GUI_ControlsBase::skinChanged()
{
	btnPlay()->setObjectName("ControlPlayButton");
	btnRecord()->setObjectName("ControlRecButton");
	btnNext()->setObjectName("ControlFwdButton");
	btnPrevious()->setObjectName("ControlBwdButton");
	btnStop()->setObjectName("ControlStopButton");
	btnMute()->setObjectName("ControlMuteButton");

	Gui::Widget::skinChanged();

	using namespace Gui;

	setIcon(btnNext(), icon(Icons::Forward));
	setIcon(btnPrevious(), icon(Icons::Backward));

	const auto playbackIcon = (m->playManager->playstate() == PlayState::Playing)
	                          ? icon(Icons::Pause)
	                          : icon(Icons::Play);

	setIcon(btnPlay(), playbackIcon);
	setIcon(btnStop(), icon(Icons::Stop));
	setIcon(btnRecord(), icon(Icons::Record));

	setupVolumeButton(sliVolume()->value());
}

void GUI_ControlsBase::streamRecorderActiveChanged()
{
	checkRecordButtonVisible();
	btnRecord()->setChecked(false);
}

void GUI_ControlsBase::checkRecordButtonVisible()
{
	const auto recordingEnabled =
		(
			GetSetting(SetNoDB::MP3enc_found) &&    // Lame Available
			GetSetting(Set::Engine_SR_Active) &&    // Streamrecorder active
			(m->playManager->currentTrack().radioMode() != RadioMode::Off) && // Radio on
			(m->playManager->playstate() == PlayState::Playing)    // Is Playing
		);

	btnPlay()->setVisible(!recordingEnabled);
	btnRecord()->setVisible(recordingEnabled);

	if(!recordingEnabled)
	{
		btnRecord()->setChecked(false);
	}
}

void GUI_ControlsBase::setCoverLocation(const MetaData& md)
{
	const auto coverLocation = Cover::Location::coverLocation(md, false);
	btnCover()->setCoverLocation(coverLocation);
}

void GUI_ControlsBase::setStandardCover()
{
	const auto coverLocation = Cover::Location::invalidLocation();
	btnCover()->setCoverLocation(coverLocation);
}

void GUI_ControlsBase::coverChanged(const QByteArray& data, const QString& mimedata)
{
	btnCover()->setCoverData(data, mimedata);
}

void GUI_ControlsBase::coverClickRejected()
{
	showInfo();
}

void GUI_ControlsBase::setupConnections()
{
	connect(btnPlay(), &QPushButton::clicked, m->playManager, &PlayManager::playPause);
	connect(btnNext(), &QPushButton::clicked, m->playManager, &PlayManager::next);
	connect(btnPrevious(), &QPushButton::clicked, m->playManager, &PlayManager::previous);
	connect(btnStop(), &QPushButton::clicked, m->playManager, &PlayManager::stop);
	connect(btnMute(), &QPushButton::clicked, m->playManager, &PlayManager::toggleMute);
	connect(btnRecord(), &QPushButton::clicked, m->playManager, &PlayManager::record);

	connect(sliVolume(), &Gui::SearchSlider::sig_slider_moved, m->playManager, &PlayManager::setVolume);
	connect(sliProgress(), &Gui::SearchSlider::sig_slider_moved, this, &GUI_ControlsBase::progressMoved);
	connect(sliProgress(), &Gui::SearchSlider::sigSliderHovered, this, &GUI_ControlsBase::progressHovered);

	connect(m->playManager, &PlayManager::sigPlaystateChanged, this, &GUI_ControlsBase::playstateChanged);
	connect(m->playManager, &PlayManager::sigCurrentTrackChanged, this, &GUI_ControlsBase::currentTrackChanged);
	connect(m->playManager, &PlayManager::sigCurrentMetadataChanged, this, &GUI_ControlsBase::refreshCurrentTrack);
	connect(m->playManager, &PlayManager::sigDurationChangedMs, this, &GUI_ControlsBase::refreshCurrentTrack);
	connect(m->playManager, &PlayManager::sigBitrateChanged, this, &GUI_ControlsBase::refreshCurrentTrack);
	connect(m->playManager, &PlayManager::sigPositionChangedMs, this, &GUI_ControlsBase::currentPositionChanged);
	connect(m->playManager, &PlayManager::sigBuffering, this, &GUI_ControlsBase::buffering);
	connect(m->playManager, &PlayManager::sigVolumeChanged, this, &GUI_ControlsBase::volumeChanged);
	connect(m->playManager, &PlayManager::sigMuteChanged, this, &GUI_ControlsBase::muteChanged);
	connect(m->playManager, &PlayManager::sigRecording, this, &GUI_ControlsBase::recordChanged);

	auto* engine = Engine::Handler::instance();
	connect(engine, &Engine::Handler::sigCoverDataAvailable, this, &GUI_ControlsBase::coverChanged);

	auto* mdcn = Tagging::ChangeNotifier::instance();
	connect(mdcn, &Tagging::ChangeNotifier::sigMetadataChanged, this, &GUI_ControlsBase::metadataChanged);
}

void GUI_ControlsBase::setupShortcuts()
{
	auto* sch = ShortcutHandler::instance();
	sch->shortcut(ShortcutIdentifier::PlayPause).connect(this, m->playManager, SLOT(playPause()));
	sch->shortcut(ShortcutIdentifier::Stop).connect(this, m->playManager, SLOT(stop()));
	sch->shortcut(ShortcutIdentifier::Next).connect(this, m->playManager, SLOT(next()));
	sch->shortcut(ShortcutIdentifier::Prev).connect(this, m->playManager, SLOT(previous()));
	sch->shortcut(ShortcutIdentifier::VolDown).connect(this, m->playManager, SLOT(volumeDown()));
	sch->shortcut(ShortcutIdentifier::VolUp).connect(this, m->playManager, SLOT(volumeUp()));
	sch->shortcut(ShortcutIdentifier::SeekFwd).connect(this, [=]() {
		m->playManager->seekRelativeMs(2000);
	});

	sch->shortcut(ShortcutIdentifier::SeekBwd).connect(this, [=]() {
		m->playManager->seekRelativeMs(-2000);
	});

	sch->shortcut(ShortcutIdentifier::SeekFwdFast).connect(this, [=]() {
		const auto ms = m->playManager->durationMs() / 20;
		m->playManager->seekRelativeMs(ms);
	});

	sch->shortcut(ShortcutIdentifier::SeekBwdFast).connect(this, [=]() {
		const auto ms = m->playManager->durationMs() / 20;
		m->playManager->seekRelativeMs(-ms);
	});
}

void GUI_ControlsBase::setRadioMode(RadioMode radio)
{
	checkRecordButtonVisible();

	if(radio != RadioMode::Off)
	{
		buffering(0);
	}
}

MD::Interpretation GUI_ControlsBase::metadataInterpretation() const
{
	return MD::Interpretation::Tracks;
}

MetaDataList GUI_ControlsBase::infoDialogData() const
{
	return (m->playManager->playstate() != PlayState::Stopped)
	       ? MetaDataList {m->playManager->currentTrack()}
	       : MetaDataList();
}

void GUI_ControlsBase::resizeEvent(QResizeEvent* e)
{
	Widget::resizeEvent(e);
	refreshCurrentTrack();
}

void GUI_ControlsBase::showEvent(QShowEvent* e)
{
	Widget::showEvent(e);
	refreshCurrentTrack();
}

void GUI_ControlsBase::contextMenuEvent(QContextMenuEvent* e)
{
	if(!m->contextMenu)
	{
		using Library::ContextMenu;

		m->contextMenu = new ContextMenu(this);
		m->contextMenu->showActions
			(
				(ContextMenu::EntryInfo |
				 ContextMenu::EntryLyrics |
				 ContextMenu::EntryEdit)
			);

		connect(m->contextMenu, &ContextMenu::sigEditClicked, this, [=]() {
			showEdit();
		});

		connect(m->contextMenu, &ContextMenu::sigInfoClicked, this, [=]() {
			showInfo();
		});

		connect(m->contextMenu, &ContextMenu::sigLyricsClicked, this, [=]() {
			showLyrics();
		});

		m->contextMenu->addPreferenceAction(new Gui::PlayerPreferencesAction(m->contextMenu));
		m->contextMenu->addPreferenceAction(new Gui::CoverPreferenceAction(m->contextMenu));
	}

	m->contextMenu->exec(e->globalPos());
}
