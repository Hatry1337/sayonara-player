/* LameConverter.h */

/* Copyright (C) 2011-2019  Lucio Carreras
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



#ifndef LAMECONVERTER_H
#define LAMECONVERTER_H

#include "Converter.h"

class LameConverter :
		public Converter
{
	Q_OBJECT
	PIMPL(LameConverter)

	public:
		LameConverter(bool cbr, int quality, QObject* parent);
		~LameConverter() override;

		QStringList supported_input_formats() const override;

		// Converter interface
	protected:
		QString binary() const override;
		QStringList process_entry(const MetaData& md) const override;
		QString extension() const override;
};

#endif // LAMECONVERTER_H
