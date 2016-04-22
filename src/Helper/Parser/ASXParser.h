/* ASXParser.h */

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



#ifndef ASXPARSER_H
#define ASXPARSER_H

#include "AbstractPlaylistParser.h"
#include <QDomNode>

/**
 * @brief The ASXParser class
 * @ingroup PlaylistParser
 */
class ASXParser : public AbstractPlaylistParser
{
public:
	ASXParser(const QString& filename);

private:
	virtual void parse() override;
	QString parse_ref_node(QDomNode node);
};

#endif // ASXPARSER_H
