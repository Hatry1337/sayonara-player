
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




#ifndef MTP_RAWDEVICE_H
#define MTP_RAWDEVICE_H

#include "MTP_Typedefs.h"
#include <QThread>

/**
 * @brief The MTP_RawDevice class
 * @ingroup MTP
 */
class MTP_RawDevice
{

public:
	explicit MTP_RawDevice(MTPIntern_RawDevice* raw_device=nullptr);
	virtual ~MTP_RawDevice();

	MTP_DevicePtr open();
	QString get_device_string() const;

private:
	MTPIntern_RawDevice* _raw_device=nullptr;
	MTP_DevicePtr		 _open_device;

};



#endif // MTP_RAWDEVICE_H
