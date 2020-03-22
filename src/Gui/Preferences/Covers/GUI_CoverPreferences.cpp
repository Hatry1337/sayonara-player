/* GUI_CoverPreferences.cpp */

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

#include "Gui/Preferences/ui_GUI_CoverPreferences.h"
#include "GUI_CoverPreferences.h"

#include "Database/Connector.h"
#include "Database/CoverConnector.h"

#include "Components/Covers/CoverFetchManager.h"
#include "Components/Covers/CoverChangeNotifier.h"
#include "Components/Covers/CoverUtils.h"
#include "Components/Covers/Fetcher/CoverFetcher.h"

#include "Utils/Settings/Settings.h"
#include "Utils/Language/Language.h"
#include "Utils/FileUtils.h"
#include "Utils/Utils.h"
#include "Utils/Logger/Logger.h"

#include "Gui/Utils/Icons.h"
#include "Gui/Utils/Delegates/StyledItemDelegate.h"

#include <QListWidgetItem>
#include <QList>
#include <QDir>
#include <QFileInfo>
#include <QCheckBox>

using namespace Cover;

GUI_CoverPreferences::GUI_CoverPreferences(const QString& identifier) :
	Base (identifier) {}


GUI_CoverPreferences::~GUI_CoverPreferences()
{
	if(ui){
		delete ui; ui = nullptr;
	}
}

static bool checkCoverTemplate(const QString& coverTemplate)
{
	if(coverTemplate.trimmed().isEmpty()){
		return false;
	}

	QString str(coverTemplate);
	str.remove("<h>");

	QList<QChar> invalid_chars
	{
		'/', '\\', '|', ':', '\"', '?', '$', '<', '>', '*', '#', '%', '&'
	};

	for(const QChar& c : invalid_chars)
	{
		if(str.contains(c)){
			return false;
		}
	}

	return true;
}

bool GUI_CoverPreferences::commit()
{
	QStringList active_items;

	for(int i=0; i<ui->lvCoverSearchers->count(); i++)
	{
		QListWidgetItem* item = ui->lvCoverSearchers->item(i);
		active_items << item->text().toLower();
	}

	SetSetting(Set::Cover_Server, active_items);
	SetSetting(Set::Cover_FetchFromWWW, ui->cbFetchFromWWW->isChecked());
	SetSetting(Set::Cover_SaveToDB, ui->cbSaveToDatabase->isChecked());
	SetSetting(Set::Cover_SaveToLibrary, ui->cbSaveToLibrary->isChecked() && ui->cbSaveToLibrary->isEnabled());
	SetSetting(Set::Cover_SaveToSayonaraDir, ui->cbSaveToSayonaraDir->isChecked() && ui->cbSaveToSayonaraDir->isEnabled());

	QString coverTemplate = ui->leCoverTemplate->text().trimmed();
	if(checkCoverTemplate(coverTemplate))
	{
		if(!Util::File::isImageFile(coverTemplate))
		{
			QString ext = Util::File::getFileExtension(coverTemplate);
			if(ext.isEmpty()){
				coverTemplate.append(".jpg");
				coverTemplate.replace("..jpg", ".jpg");
			}

			else {
				coverTemplate.replace("." + ext, ".jpg");
			}

			ui->leCoverTemplate->setText(coverTemplate);
		}

		SetSetting(Set::Cover_TemplatePath, coverTemplate);
	}

	else
	{
		ui->leCoverTemplate->setText(GetSetting(Set::Cover_TemplatePath));
		ui->labTemplateError->setVisible(false);
	}

	return true;
}

void GUI_CoverPreferences::revert()
{
	Cover::Fetcher::Manager* cfm = Cover::Fetcher::Manager::instance();

	QStringList cover_servers = GetSetting(Set::Cover_Server);

	ui->lvCoverSearchers->clear();
	ui->lvInactiveCoverSearchers->clear();


	QList<Cover::Fetcher::Base*> cover_fetchers = cfm->coverfetchers();
	for(const Cover::Fetcher::Base* b : cover_fetchers)
	{
		QString name = b->identifier();


		if(name.trimmed().isEmpty()) {
			continue;
		}

		if(cover_servers.contains(name))
		{
			ui->lvCoverSearchers->addItem(Util::stringToVeryFirstUpper(name));
		}

		else {
			ui->lvInactiveCoverSearchers->addItem(Util::stringToVeryFirstUpper(name));
		}
	}

	ui->cbFetchFromWWW->setChecked(GetSetting(Set::Cover_FetchFromWWW));
	ui->cbSaveToDatabase->setChecked(GetSetting(Set::Cover_SaveToDB));
	ui->cbSaveToSayonaraDir->setChecked(GetSetting(Set::Cover_SaveToSayonaraDir));
	ui->cbSaveToLibrary->setChecked(GetSetting(Set::Cover_SaveToLibrary));
	ui->leCoverTemplate->setText(GetSetting(Set::Cover_TemplatePath));

	fetchCoversFromWWWTriggered(GetSetting(Set::Cover_FetchFromWWW));
	saveCoverToLibraryToggled(GetSetting(Set::Cover_SaveToLibrary));

	currentRowChanged(ui->lvCoverSearchers->currentRow());
}

QString GUI_CoverPreferences::actionName() const
{
	return Lang::get(Lang::Covers);
}

void GUI_CoverPreferences::initUi()
{
	if(ui){
		return;
	}

	setupParent(this, &ui);

	ui->lvCoverSearchers->setItemDelegate(new Gui::StyledItemDelegate(ui->lvCoverSearchers));
	ui->lvInactiveCoverSearchers->setItemDelegate(new Gui::StyledItemDelegate(ui->lvInactiveCoverSearchers));
	ui->labTemplateError->setVisible(false);

	connect(ui->btnUp, &QPushButton::clicked, this, &GUI_CoverPreferences::upClicked);
	connect(ui->btnDown, &QPushButton::clicked, this, &GUI_CoverPreferences::downClicked);
	connect(ui->lvCoverSearchers, &QListWidget::currentRowChanged, this, &GUI_CoverPreferences::currentRowChanged);
	connect(ui->btnDeleteCovers, &QPushButton::clicked, this, &GUI_CoverPreferences::deleteCoversFromDb);
	connect(ui->btnDeleteFiles, &QPushButton::clicked, this, &GUI_CoverPreferences::deleteCoverFiles);
	connect(ui->cbFetchFromWWW, &QCheckBox::toggled, this, &GUI_CoverPreferences::fetchCoversFromWWWTriggered);
	connect(ui->btnAdd, &QPushButton::clicked, this, &GUI_CoverPreferences::addClicked);
	connect(ui->btnRemove, &QPushButton::clicked, this, &GUI_CoverPreferences::removeClicked);
	connect(ui->cbSaveToLibrary, &QCheckBox::toggled, this, &GUI_CoverPreferences::saveCoverToLibraryToggled);
	connect(ui->leCoverTemplate, &QLineEdit::textEdited, this, &GUI_CoverPreferences::coverTemplateEdited);

	ui->cbSaveToSayonaraDir->setToolTip(Cover::Utils::coverDirectory());

	revert();
}

void GUI_CoverPreferences::retranslate()
{
	ui->retranslateUi(this);

	ui->btnUp->setText(Lang::get(Lang::MoveUp));
	ui->btnDown->setText(Lang::get(Lang::MoveDown));
}

void GUI_CoverPreferences::skinChanged()
{
	if(!ui){
		return;
	}

	ui->btnDeleteFiles->setIcon(Gui::Icons::icon(Gui::Icons::Delete));
	ui->btnDeleteCovers->setIcon(Gui::Icons::icon(Gui::Icons::Clear));
}

void GUI_CoverPreferences::upClicked()
{
	int cur_row = ui->lvCoverSearchers->currentRow();

	QListWidgetItem* item = ui->lvCoverSearchers->takeItem(cur_row);
	ui->lvCoverSearchers->insertItem(cur_row - 1, item);
	ui->lvCoverSearchers->setCurrentRow(cur_row - 1);
}

void GUI_CoverPreferences::downClicked()
{
	int cur_row = ui->lvCoverSearchers->currentRow();

	QListWidgetItem* item = ui->lvCoverSearchers->takeItem(cur_row);
	ui->lvCoverSearchers->insertItem(cur_row + 1, item);
	ui->lvCoverSearchers->setCurrentRow(cur_row + 1);
}

void GUI_CoverPreferences::addClicked()
{
	QListWidgetItem* item = ui->lvInactiveCoverSearchers->takeItem(ui->lvInactiveCoverSearchers->currentRow());
	if(!item){
		return;
	}

	ui->lvCoverSearchers->addItem(item->text());
	delete item; item=nullptr;
}

void GUI_CoverPreferences::removeClicked()
{
	QListWidgetItem* item = ui->lvCoverSearchers->takeItem(ui->lvCoverSearchers->currentRow());
	if(!item){
		return;
	}

	ui->lvInactiveCoverSearchers->addItem(item->text());
	delete item; item=nullptr;
}

void GUI_CoverPreferences::currentRowChanged(int row)
{
	ui->btnUp->setDisabled(row <= 0 || row >= ui->lvCoverSearchers->count());
	ui->btnDown->setDisabled(row < 0 || row >= ui->lvCoverSearchers->count() - 1);
}

void GUI_CoverPreferences::deleteCoversFromDb()
{
	DB::Connector::instance()->coverConnector()->clear();
	Cover::ChangeNotfier::instance()->shout();
}

void GUI_CoverPreferences::deleteCoverFiles()
{
	::Util::File::removeFilesInDirectory(Cover::Utils::coverDirectory());
}

void GUI_CoverPreferences::fetchCoversFromWWWTriggered(bool b)
{
	ui->lvCoverSearchers->setEnabled(b);
	ui->lvInactiveCoverSearchers->setEnabled(b);
	ui->btnDown->setEnabled(b);
	ui->btnUp->setEnabled(b);
	ui->btnAdd->setEnabled(b);
	ui->btnRemove->setEnabled(b);

	ui->cbSaveToSayonaraDir->setEnabled(b);

	ui->cbSaveToLibrary->setEnabled(b);
	ui->leCoverTemplate->setEnabled(b);
	ui->labCoverTemplate->setEnabled(b);
}

void GUI_CoverPreferences::saveCoverToLibraryToggled(bool b)
{
	ui->leCoverTemplate->setVisible(b);
	ui->labCoverTemplate->setVisible(b);
}


void GUI_CoverPreferences::coverTemplateEdited(const QString& text)
{
	bool valid = checkCoverTemplate(text);
	ui->labTemplateError->setVisible(!valid);
	ui->labTemplateError->setText(Lang::get(Lang::Error) + ": " + Lang::get(Lang::InvalidChars));
}
