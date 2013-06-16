/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 */


#include <ola/Logging.h>

extern "C" {
#include <usbdmx.h>
}

#include <string>

#include "plugins/enlightenment/EnlightenmentPlugin.h"
#include "plugins/enlightenment/EnlightenmentDevice.h"
#include "plugins/enlightenment/EnlightenmentPort.h"

namespace ola {
namespace plugin {
namespace enlightenment {

using ola::Device;

/*
 * Create a new device
 * @param owner the plugin that owns this device
 * @param name  the device name
 * @param device_id the device id
 */
EnlightenmentDevice::EnlightenmentDevice(class EnlightenmentPlugin *owner,
                                         const string &name,
                                         char* device_id,
                                         int device_mode
                                         )
  : Device(owner, name),
    m_input(NULL),
    m_output(NULL),
    m_device_mode(device_mode),
    m_fd(0),
    m_allow_multiport_patching(true) {
  // this is the id the device is attached to
  memcpy(m_device_serial, device_id, 16);
  m_device_serial[16] = '\0';
}

unsigned int EnlightenmentDevice::InterfaceVersion() {
  return GetDeviceVersion(m_device_serial);
}

bool EnlightenmentDevice::openInterface(class PluginAdaptor *plugin_adaptor) {
  // create the ports (according to requested interface mode)
  // 0: Do nothing - Standby
  // 1: DMX In -> DMX Out
  // 2: PC Out -> DMX Out
  // 3: DMX In + PC Out -> DMX Out
  // 4: DMX In -> PC In
  // 5: DMX In -> DMX Out & DMX In -> PC In
  // 6: PC Out -> DMX Out & DMX In -> PC In
  // 7: DMX In + PC Out -> DMX Out & DMX In -> PC In
  switch (m_device_mode) {
    case 4:
    case 5:
    case 6:
    case 7:
      m_input = new EnlightenmentInputPort(
                  this,
                  plugin_adaptor,
                  0 /* TODO: Which id goes here? */);
      OLA_DEBUG << "created new input port";
      if (m_input == NULL)
        return false;
      this->AddPort(m_input);
      break;
  }

  switch (m_device_mode) {
    case 2:
    case 3:
    case 6:
    case 7:
      m_output = new EnlightenmentOutputPort(
                  this,
                  0 /* TODO: Which id goes here? */);
      OLA_DEBUG << "created new output port";
      if (m_output == NULL)
        return false;
      this->AddPort(m_output);
      break;
  }

  // open the interface
  int fd = OpenLink(
              m_device_serial,
              m_output ? m_output->getTDMXArray() : NULL,
              m_input ? m_input->getTDMXArray() : NULL);
  if (fd != 1) {
    OLA_WARN << "Unable to open Interface, did you install the udev-rule?\n";
    return false;
  } else {
    m_fd = fd;
  }
  // configure the mode
  SetInterfaceMode(m_device_serial, m_device_mode);

  // mode 7 is the only mode, multport patching is not allowed
  if (m_device_mode == 7)
    m_allow_multiport_patching = false;

  return true;
}

EnlightenmentDevice::~EnlightenmentDevice() {
  CloseLink(m_device_serial);
}

}  // namespace enlightenment
}  // namespace plugin
}  // namespace ola

