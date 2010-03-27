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
 * ArduinoRGBDevice.h
 * The Ardunio RGB Mixer device.
 * Copyright (C) 2010 Simon Newton
 */

#include <algorithm>
#include <string>
#include "ola/BaseTypes.h"
#include "ola/Logging.h"
#include "ola/network/NetworkUtils.h"
#include "plugins/usbpro/ArduinoRGBDevice.h"

namespace ola {
namespace plugin {
namespace usbpro {

using std::string;
using ola::network::NetworkToHost;


/*
 * New Arduino RGB Device
 */
ArduinoRGBDevice::ArduinoRGBDevice(ola::AbstractPlugin *owner,
                                   const string &name,
                                   UsbWidget *widget,
                                   uint16_t esta_id,
                                   uint16_t device_id,
                                   uint32_t serial):
    UsbDevice(owner, name, widget) {
  std::stringstream str;
  str << std::hex << esta_id << "-" << device_id << "-" <<
    NetworkToHost(serial);
  m_device_id = str.str();
  ArduinoRGBOutputPort *output_port = new ArduinoRGBOutputPort(this);
  AddPort(output_port);
  m_enabled = true;
}


/*
 * Stop this device
 */
bool ArduinoRGBDevice::Stop() {
  if (!m_enabled)
    return true;

  DeleteAllPorts();
  m_enabled = false;
  return true;
}


/*
 * Send a dmx msg
 * @returns true if we sent ok, false otherwise
 */
bool ArduinoRGBDevice::SendDMX(const DmxBuffer &buffer) const {
  struct {
    uint8_t start_code;
    uint8_t dmx[DMX_UNIVERSE_SIZE];
  } widget_dmx;

  if (!m_enabled)
    return true;

  widget_dmx.start_code = 0;
  unsigned int length = DMX_UNIVERSE_SIZE;
  buffer.Get(widget_dmx.dmx, &length);
  return m_widget->SendMessage(UsbWidget::DMX_LABEL,
                               length + 1,
                               reinterpret_cast<uint8_t*>(&widget_dmx));
}
}  // usbpro
}  // plugin
}  // ola