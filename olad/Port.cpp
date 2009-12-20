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
 * Port.cpp
 * Base implementation of the Port class.
 * Copyright (C) 2005-2009 Simon Newton
 */

#include <string>
#include "olad/Device.h"
#include "olad/Port.h"

namespace ola {

string Port::UniqueId() const {
  if (m_port_string.empty()) {
    std::stringstream str;
    if (m_device)
      str << m_device->UniqueId() << "-" << PortPrefix() << "-" << m_port_id;
    m_port_string = str.str();
  }
  return m_port_string;
}


bool Port::SetUniverse(Universe *new_universe) {
  Universe *old_universe = GetUniverse();
  if (old_universe == new_universe)
    return true;

  if (PreSetUniverse(new_universe, old_universe)) {
    m_universe = new_universe;
    PostSetUniverse(new_universe, old_universe);
    return true;
  }
  return false;
}


/*
 * This allows switching based on Port type.
 */
template<class PortClass>
bool IsInputPort() {
  return true;
}

template<>
bool IsInputPort<OutputPort>() {
  return false;
}

template<>
bool IsInputPort<InputPort>() {
  return true;
}
}  // ola