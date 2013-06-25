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
                                         const string &device_id,
                                         DeviceMode device_mode
                                         )
  : Device(owner, name),
    m_input(NULL),
    m_output(NULL),
    m_device_serial(device_id),
    m_device_mode(device_mode),
    m_fd(ola::io::INVALID_DESCRIPTOR),
    m_allow_multiport_patching(true) { }

unsigned int EnlightenmentDevice::InterfaceVersion() {
    return GetDeviceVersion(InterfaceSerial());
}

bool EnlightenmentDevice::openInterface(class PluginAdaptor *plugin_adaptor) {
  // create the input if required by device mode
  switch (m_device_mode) {
    case DMXIN_TO_PCIN:
    case DMXIN_TO_PCOUT_AND_DMXIN_TO_PCIN:
    case PCOUT_TO_DMXOUT_AND_DMXIN_TO_PCIN:
    case DMXIN_PCOUT_TO_DMXOUT_AND_DMXIN_TO_PCIN:
      m_input = new EnlightenmentInputPort(
                  this,
                  plugin_adaptor,
                  0);
      OLA_DEBUG << "created new input port";
      if (m_input == NULL)
        return false;
      this->AddPort(m_input);
      break;
  default:
      // no input needed
      break;
  }

  // create the output if required by device mode
  switch (m_device_mode) {
    case PCOUT_TO_DMXOUT:
    case DMXIN_PCIN_TO_DMXOUT:
    case PCOUT_TO_DMXOUT_AND_DMXIN_TO_PCIN:
    case DMXIN_PCOUT_TO_DMXOUT_AND_DMXIN_TO_PCIN:
      m_output = new EnlightenmentOutputPort(
                  this,
                  0);
      OLA_DEBUG << "created new output port";
      if (m_output == NULL)
        return false;
      this->AddPort(m_output);
      break;
  default:
      // no output needed
      break;
  }

  // open the interface
  int fd = OpenLink(
              InterfaceSerial(),
              m_output ? m_output->getTDMXArray() : NULL,
              m_input ? m_input->getTDMXArray() : NULL);
  if (fd != 1) {
    OLA_WARN << "Unable to open Interface, did you install the udev-rule?\n";
    return false;
  } else {
    m_fd = fd;
  }
  // configure the mode
  SetInterfaceMode(InterfaceSerial(), m_device_mode);

  // mode 7 is the only mode, multport patching is not allowed
  if (m_device_mode == 7)
    m_allow_multiport_patching = false;

  return true;
}

EnlightenmentDevice::~EnlightenmentDevice() {
    CloseLink(InterfaceSerial());
}

void EnlightenmentDevice::IncomingChanges() {
    if (m_input)
        m_input->UpdateData();
}

}  // namespace enlightenment
}  // namespace plugin
}  // namespace ola

