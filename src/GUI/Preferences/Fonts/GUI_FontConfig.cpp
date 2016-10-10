
/* Copyright (C) 2011-2016  Lucio Carreras
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


#include "GUI_FontConfig.h"
#include "GUI/Preferences/ui_GUI_FontConfig.h"

#include "Helper/Settings/Settings.h"

#include <QApplication>
#include <QFont>
#include <QFontDatabase>

GUI_FontConfig::GUI_FontConfig(QWidget* parent) :
	PreferenceWidgetInterface(parent)
{

}

GUI_FontConfig::~GUI_FontConfig()
{
	if(ui)
	{
		delete ui; ui=nullptr;
	}
}

QString GUI_FontConfig::get_action_name() const
{
	return tr("Fonts");
}

void GUI_FontConfig::init_ui()
{
	setup_parent(this, &ui);

	_font_db = new QFontDatabase();

	connect(ui->combo_fonts, &QFontComboBox::currentFontChanged, this, &GUI_FontConfig::combo_fonts_changed);
	connect(ui->btn_default, &QPushButton::clicked, this, &GUI_FontConfig::default_clicked);

	ui->combo_fonts->setEditable(false);
	ui->combo_fonts->setFontFilters(QFontComboBox::ScalableFonts);

	revert();
}


void GUI_FontConfig::combo_fonts_changed(const QFont& font){

	_cur_font_size = ui->combo_sizes->currentText().toInt();

	QStringList sizes = get_available_font_sizes(font);
	fill_sizes(sizes);

	int font_size = _cur_font_size;
	if(font_size <= 0){
		font_size = QApplication::font().pointSize();
	}

	int cur_font_size_idx = ui->combo_sizes->findText(QString::number(font_size));

	if(cur_font_size_idx >= 0){
		ui->combo_sizes->setCurrentIndex(cur_font_size_idx);
	}

	ui->combo_lib_size->setCurrentIndex(0);
	ui->combo_pl_size->setCurrentIndex(0);

	_is_default = false;
}


QStringList GUI_FontConfig::get_available_font_sizes(const QString& font_name, const QString& style)
{
	QStringList ret;
	QList<int> font_sizes =  _font_db->pointSizes(font_name, style);
	for(int font_size : font_sizes){
		ret << QString::number(font_size);
	}

	return ret;
}

QStringList GUI_FontConfig::get_available_font_sizes(const QFont& font)
{
	return get_available_font_sizes(font.family(), font.styleName());
}

void GUI_FontConfig::fill_sizes(const QStringList& sizes)
{
	ui->combo_sizes->clear();
	ui->combo_lib_size->clear();
	ui->combo_pl_size->clear();

	ui->combo_lib_size->addItem(tr("Inherit"));
	ui->combo_pl_size->addItem(tr("Inherit"));

	for(QString sz : sizes){
		int isz = sz.toInt();
		if(isz < 7 || isz > 14){
			continue;
		}

		ui->combo_sizes->addItem(sz);
		ui->combo_lib_size->addItem(sz);
		ui->combo_pl_size->addItem(sz);
	}
}


void GUI_FontConfig::commit()
{
	bool ok;
	int font_size;

	font_size = ui->combo_sizes->currentText().toInt(&ok);
	_settings->set(Set::Player_FontName, ui->combo_fonts->currentText());

	if(ok){
		_settings->set(Set::Player_FontSize, font_size);
	}

	font_size = ui->combo_lib_size->currentText().toInt(&ok);
	if(!ok){
		font_size = -1;
	}
	_settings->set(Set::Lib_FontSize, font_size);

	font_size = ui->combo_pl_size->currentText().toInt(&ok);
	if(!ok){
		font_size = -1;
	}
	_settings->set(Set::PL_FontSize, font_size);

	_settings->set(Set::Lib_FontBold, ui->cb_lib_bold->isChecked());

	_cur_font_size = font_size;
}

void GUI_FontConfig::revert(){

	QString cur_family = _settings->get(Set::Player_FontName);
	int cur_font_size = _settings->get(Set::Player_FontSize);
	int cur_pl_font_size = _settings->get(Set::PL_FontSize);
	int cur_lib_font_size = _settings->get(Set::Lib_FontSize);
	bool bold = _settings->get(Set::Lib_FontBold);

	int idx = ui->combo_fonts->findText(cur_family);
	if(idx >= 0){
		ui->combo_fonts->setCurrentIndex(idx);
	}

	fill_sizes( get_available_font_sizes(ui->combo_fonts->currentFont()) );

	idx = ui->combo_sizes->findText(QString::number(cur_font_size));
	if(idx >= 0){
		ui->combo_sizes->setCurrentIndex(idx);
	}

	idx = ui->combo_lib_size->findText(QString::number(cur_lib_font_size));
	if(idx >= 0){
		ui->combo_lib_size->setCurrentIndex(idx);
	}
	else{
		ui->combo_lib_size->setCurrentIndex(0);
	}

	idx = ui->combo_pl_size->findText(QString::number(cur_pl_font_size));
	if(idx >= 0){
		ui->combo_pl_size->setCurrentIndex(idx);
	}
	else{
		ui->combo_pl_size->setCurrentIndex(0);
	}

	ui->cb_lib_bold->setChecked(bold);
}


void GUI_FontConfig::default_clicked()
{
	QFont font = QApplication::font();

	int cur_font_idx = ui->combo_fonts->findText(font.family());
	if(cur_font_idx >= 0){
		ui->combo_fonts->setCurrentIndex(cur_font_idx);
	}

	ui->combo_lib_size->setCurrentIndex(0);
	ui->combo_pl_size->setCurrentIndex(0);
	ui->cb_lib_bold->setChecked(true);

	int cur_font_size_idx = ui->combo_sizes->findText(QString::number(font.pointSize()));
	if(cur_font_size_idx >= 0){
		ui->combo_sizes->setCurrentIndex(cur_font_size_idx);
	}

	_is_default = true;
}


void GUI_FontConfig::language_changed()
{
	translate_action();

	if(!is_ui_initialized()){
		return;
	}

	ui->retranslateUi(this);
	PreferenceWidgetInterface::language_changed();
}

void GUI_FontConfig::skin_changed()
{

}
