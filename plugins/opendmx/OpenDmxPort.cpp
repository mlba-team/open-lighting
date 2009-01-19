/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * OpenDmxPort.cpp
 * The Open DMX plugin for lla
 * Copyright (C) 2005-2008 Simon Newton
 */

#include "OpenDmxPort.h"
#include "OpenDmxDevice.h"

namespace lla {
namespace plugin {

OpenDmxPort::OpenDmxPort(AbstractDevice *parent, int id, const string &path): Port(parent, id) {
  m_thread = new OpenDmxThread();

  if (m_thread)
    m_thread->Start(path);
}


OpenDmxPort::~OpenDmxPort() {
  if (m_thread) {
    m_thread->Stop();
    delete m_thread;
  }
}


/*
 * Write operation
 *
 * @param  data  pointer to the dmx data
 * @param  length  the length of the data
 *
 */
int OpenDmxPort::WriteDMX(uint8_t *data, unsigned int length) {
  if (!CanWrite())
    return -1;

  m_thread->WriteDmx(data, length);
  return 0;
}

} //plugins
} //lla