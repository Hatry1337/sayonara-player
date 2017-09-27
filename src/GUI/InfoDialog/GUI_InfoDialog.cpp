/* GUI_InfoDialog.cpp

 * Copyright (C) 2011-2017 Lucio Carreras
 *
 * This file is part of sayonara-player
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * created by Lucio Carreras,
 * Jul 19, 2012
 *
 */

#include "GUI_InfoDialog.h"
#include "GUI/InfoDialog/ui_GUI_InfoDialog.h"

#include "InfoDialogContainer.h"

#include "GUI/TagEdit/GUI_TagEdit.h"
#include "GUI/InfoDialog/GUI_Lyrics.h"
#include "GUI/Helper/IconLoader/IconLoader.h"

#include "Components/Covers/CoverLocation.h"
#include "Components/TagEdit/TagEdit.h"

#include "Helper/MetaData/MetaDataList.h"
#include "Helper/MetaDataInfo/MetaDataInfo.h"
#include "Helper/MetaDataInfo/AlbumInfo.h"
#include "Helper/MetaDataInfo/ArtistInfo.h"
#include "Helper/FileHelper.h"
#include "Helper/Language.h"
#include "Helper/globals.h"

#include <algorithm>

struct GUI_InfoDialog::Private
{
	InfoDialogContainer*	info_dialog_container=nullptr;
	GUI_TagEdit*			ui_tag_edit=nullptr;
	GUI_Lyrics*				ui_lyrics=nullptr;
	CoverLocation			cl;
	MetaDataList			v_md;
	MD::Interpretation		md_interpretation;
};


GUI_InfoDialog::GUI_InfoDialog(InfoDialogContainer* container, QWidget* parent) :
	SayonaraDialog(parent)
{
	ui = nullptr;
	m = Pimpl::make<GUI_InfoDialog::Private>();

	m->info_dialog_container = container;
	m->md_interpretation = MD::Interpretation::None;
}

GUI_InfoDialog::~GUI_InfoDialog() {}

void GUI_InfoDialog::language_changed()
{
	if(!ui){
		return;
	}

	ui->retranslateUi(this);

	prepare_info(m->md_interpretation);
	ui->tab_widget->setTabText(0, Lang::get(Lang::Info));
	ui->tab_widget->setTabText(1, Lang::get(Lang::Lyrics));
	ui->tab_widget->setTabText(2, Lang::get(Lang::Edit));
	ui->btn_close1->setText(Lang::get(Lang::Close));
}

void GUI_InfoDialog::skin_changed()
{
	if(!ui){
		return;
	}

	QTabBar* tab_bar = ui->tab_widget->tabBar();
	if(tab_bar)
	{
		IconLoader* icon_loader = IconLoader::getInstance();
		tab_bar->setTabIcon(0, icon_loader->get_icon("dialog-info", "info"));
		tab_bar->setTabIcon(1, icon_loader->get_icon("document-properties", "lyrics"));
		tab_bar->setTabIcon(2, icon_loader->get_icon("accessories-text-editor", "edit"));
	}
}

void GUI_InfoDialog::prepare_info(MD::Interpretation md_interpretation)
{
	if(!ui){
		return;
	}

	MetaDataInfo* info;

	switch (md_interpretation)
	{
		case MD::Interpretation::Artists:
			info = new ArtistInfo(m->v_md);
			break;
		case MD::Interpretation::Albums:
			info = new AlbumInfo(m->v_md);
			break;

		case MD::Interpretation::Tracks:
			info = new MetaDataInfo(m->v_md);
			break;

		default:
			return;
	}

	QString info_text = info->get_info_as_string() + CAR_RET + CAR_RET +
			info->get_additional_info_as_string();

	ui->lab_title->setText(info->get_header());
	ui->lab_subheader->setText(info->get_subheader());
	ui->lab_info->setText(info_text);
	ui->lab_paths->setOpenExternalLinks(true);
	ui->lab_paths->setText(info->get_paths_as_string());

	m->cl = info->get_cover_location();

	prepare_cover(m->cl);

	delete info; info = nullptr;
}


void GUI_InfoDialog::set_metadata(const MetaDataList& v_md, MD::Interpretation md_interpretation)
{
	m->md_interpretation = md_interpretation;
	m->v_md = v_md;

	prepare_info(md_interpretation);
	if(ui){
		m->ui_lyrics->set_metadata(v_md.first());
	}
}

bool GUI_InfoDialog::has_metadata() const
{
	return (m->v_md.size() > 0);
}

void GUI_InfoDialog::tab_index_changed_int(int idx)
{
	idx = std::min( (int) GUI_InfoDialog::Tab::Edit, idx);
	idx = std::max( (int) GUI_InfoDialog::Tab::Info, idx);

	tab_index_changed( (GUI_InfoDialog::Tab) idx );
}

void GUI_InfoDialog::tab_index_changed(GUI_InfoDialog::Tab idx)
{
	if(!ui){
		return;
	}

	ui->ui_info_widget->hide();
	m->ui_lyrics->hide();
	m->ui_tag_edit->hide();

	switch(idx)
	{
		case GUI_InfoDialog::Tab::Edit:

			ui->tab_widget->setCurrentWidget(m->ui_tag_edit);
			{
				MetaDataList local_md;
				for(const MetaData& md : m->v_md){
					if(!Helper::File::is_www(md.filepath())){
						local_md << md;
					}
				}

				if(local_md.size() > 0) {
					m->ui_tag_edit->get_tag_edit()->set_metadata(local_md);
				}
			}

			m->ui_tag_edit->show();
			break;

		case GUI_InfoDialog::Tab::Lyrics:

			ui->tab_widget->setCurrentWidget(m->ui_lyrics);

			m->ui_lyrics->set_metadata(m->v_md.first());
			m->ui_lyrics->show();

			break;

		default:
			ui->tab_widget->setCurrentWidget(ui->ui_info_widget);
			ui->ui_info_widget->show();
			prepare_cover(m->cl);
			break;
	}
}


void GUI_InfoDialog::show(GUI_InfoDialog::Tab tab)
{
	if(!ui){
		init();
	}

	QTabWidget* tab_widget = ui->tab_widget;

	prepare_cover(CoverLocation::getInvalidLocation());

	bool lyric_enabled = (m->v_md.size() == 1);
	bool tag_edit_enabled = std::any_of(m->v_md.begin(), m->v_md.end(), [](const MetaData& md){
		return (!Helper::File::is_www(md.filepath()));
	});

	tab_widget->setTabEnabled((int) GUI_InfoDialog::Tab::Edit, tag_edit_enabled);
	tab_widget->setTabEnabled((int) GUI_InfoDialog::Tab::Lyrics, lyric_enabled);

	if( !tab_widget->isTabEnabled((int) tab) )
	{
		tab = GUI_InfoDialog::Tab::Info;
	}

	if(tab == GUI_InfoDialog::Tab::Edit)
	{
		MetaDataList local_md;
		for(const MetaData& md : m->v_md){
			if(!Helper::File::is_www(md.filepath())){
				local_md << md;
			}
		}

		if(local_md.size() > 0){
			m->ui_tag_edit->get_tag_edit()->set_metadata(local_md);
		}
	}

	if(tab == GUI_InfoDialog::Tab::Lyrics)
	{
		m->ui_lyrics->set_metadata(m->v_md.first());
	}

	tab_widget->setCurrentIndex((int) tab);
	tab_index_changed(tab);

	SayonaraDialog::show();
}


void GUI_InfoDialog::prepare_cover(const CoverLocation& cl)
{
	ui->btn_image->set_cover_location(cl);
}


void GUI_InfoDialog::init()
{
	if(ui){
		return;
	}

	ui = new Ui::InfoDialog();
	ui->setupUi(this);

	QLayout* tab2_layout = ui->tab_2->layout();
	QLayout* tab3_layout = ui->tab_3->layout();
	QTabWidget* tab_widget = ui->tab_widget;

	if(tab2_layout){
		m->ui_lyrics = new GUI_Lyrics(ui->tab_2);
		tab2_layout->addWidget(m->ui_lyrics);
	}

	if(tab3_layout){
		m->ui_tag_edit = new GUI_TagEdit(ui->tab_3);
		tab3_layout->addWidget(m->ui_tag_edit);
	}

	connect(tab_widget, &QTabWidget::currentChanged, this, &GUI_InfoDialog::tab_index_changed_int);
	connect(m->ui_lyrics, &GUI_Lyrics::sig_closed, this, &GUI_InfoDialog::close);
	connect(m->ui_tag_edit, &GUI_TagEdit::sig_cancelled, this, &GUI_InfoDialog::close);

	ui->btn_image->setStyleSheet("QPushButton:hover {background-color: transparent;}");

	tab_widget->setCurrentIndex((int) GUI_InfoDialog::Tab::Info);
	tab_widget->setFocusPolicy(Qt::NoFocus);

	language_changed();
	skin_changed();

	prepare_info(m->md_interpretation);
}


void GUI_InfoDialog::closeEvent(QCloseEvent* e)
{
	SayonaraDialog::closeEvent(e);

	m->info_dialog_container->info_dialog_closed();
}

void GUI_InfoDialog::showEvent(QShowEvent *e)
{
	if(!ui){
		init();
	}

	SayonaraDialog::showEvent(e);
}
