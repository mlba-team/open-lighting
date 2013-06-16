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

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <ola/BaseTypes.h>
#include <ola/Logging.h>
#include <ola/StringUtils.h>
#include <olad/PluginAdaptor.h>
#include <olad/Preferences.h>

extern "C" {
#include <usbdmx.h>
}

#include <string>
#include <vector>
#include <list>

#include "plugins/enlightenment/EnlightenmentDevice.h"
#include "plugins/enlightenment/EnlightenmentPlugin.h"
#include "plugins/enlightenment/EnlightenmentPort.h"


static ola::thread::Mutex enlightenment_plugin_mutex;
static std::list<ola::plugin::enlightenment::EnlightenmentPlugin*> enlightenment_plugin_instances;

/*
 * Software Interrupt handler
 */
extern "C" void enlightenment_dmx_handler() {
  using ola::plugin::enlightenment::EnlightenmentPlugin;

  std::list<EnlightenmentPlugin*>::iterator it;
  ola::thread::MutexLocker lock(&enlightenment_plugin_mutex);

  for (it = enlightenment_plugin_instances.begin(); it != enlightenment_plugin_instances.end(); ++it)
    (*it)->IncomingChanges();
}

namespace ola {
namespace plugin {
namespace enlightenment {

const char EnlightenmentPlugin::PLUGIN_NAME[] = "Enlightenment";
const char EnlightenmentPlugin::PLUGIN_PREFIX[] = "enlightenment";
const char EnlightenmentPlugin::DEVICE_MODE_KEY[] = "device_mode";
const char EnlightenmentPlugin::DEFAULT_DEVICE_MODE[] = "6";


EnlightenmentPlugin::~EnlightenmentPlugin() {
  CleanupDevices();
}


/*
 * Start the plugin
 */
bool EnlightenmentPlugin::StartHook() {
  if (!StringToInt(m_preferences->GetValue(DEVICE_MODE_KEY), &m_device_mode))
    StringToInt(DEFAULT_DEVICE_MODE, &m_device_mode);

  // open all connected interfaces
  if (!SetupDevices()) {
    CleanupDevices();
    return false;
  }
  // register the callback to react to incoming changes
  enlightenment_plugin_mutex.Lock();
  enlightenment_plugin_instances.push_back(this);
  enlightenment_plugin_mutex.Unlock();
  RegisterInputChangeNotification(enlightenment_dmx_handler);

  return true;
}


/*
 * Stop the plugin
 * @return true on success, false on failure
 */
bool EnlightenmentPlugin::StopHook() {
  enlightenment_plugin_mutex.Lock();
  enlightenment_plugin_instances.remove(this);
  enlightenment_plugin_mutex.Unlock();
  UnregisterInputChangeNotification();
  CleanupDevices();

  return true;
}


/*
 * Return the description for this plugin
 */
string EnlightenmentPlugin::Description() const {
  return
      "Enlightenment Plugin\n"
      "----------------------------\n"
      "\n"
      "This plugin exposes the enlightenment plugin\n"
      "in the configured mode, the predefined default\n"
      "is mode 6 (PC Out -> DMX Out & DMX In -> PC In)\n"
      "\n"
      "--- Config file : ola-enlightenment.conf ---\n\n"
      "device_mode = 6\n\n"
      "// Valid Modes:\n"
      "// 0: Do nothing - Standby\n"
      "// 1: DMX In -> DMX Out\n"
      "// 2: PC Out -> DMX Out\n"
      "// 3: DMX In + PC Out -> DMX Out\n"
      "// 4: DMX In -> PC In\n"
      "// 5: DMX In -> DMX Out & DMX In -> PC In\n"
      "// 6: PC Out -> DMX Out & DMX In -> PC In\n"
      "// 7: DMX In + PC Out -> DMX Out & DMX In -> PC In";
}


/*
 * Called when there is input for us
 */
void EnlightenmentPlugin::IncomingChanges() {
  std::list<EnlightenmentInputPort*>::iterator iter;

  // check all input ports for new data
  for (iter = m_input_ports.begin(); iter != m_input_ports.end(); ++iter)
    (*iter)->UpdateData();
}


/*
 * load the plugin prefs and default to sensible values
 */
bool EnlightenmentPlugin::SetDefaultPreferences() {
  if (!m_preferences)
    return false;

  bool save = false;

  save |= m_preferences->SetDefaultValue(DEVICE_MODE_KEY,
                                         IntValidator(0, 7),
                                         DEFAULT_DEVICE_MODE);

  if (save)
    m_preferences->Save();

  return true;
}


/*
 * Open the input and output fds
 */
bool EnlightenmentPlugin::SetupDevices() {
  TSERIALLIST interfaces;
  GetAllConnectedInterfaces(&interfaces);

  for (int i = 0;; ++i) {
    const TSERIAL NULL_SERIAL = {'0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0', '0'};

    char terminated_serial[17];
    memcpy(terminated_serial, interfaces[i], 16);
    terminated_serial[16] = '\0';

    string device_serial(terminated_serial);
    // test for the last interface
    if (memcmp(interfaces[i], NULL_SERIAL, 16) == 0) {
      OLA_DEBUG << "found empty serial placeholder, ending device enumeration";
      break;
    }

    OLA_DEBUG << "trying to open device " << device_serial
              << ", mode " << m_device_mode;

    // create the device
    EnlightenmentDevice *device = new EnlightenmentDevice(
                this,
                "Enlightenment [" + device_serial + "]",
                interfaces[i],
                m_device_mode);
    OLA_INFO << "new device : serial = " << device->InterfaceSerialStr()
             << " / version " << device->InterfaceVersion();
    m_devices.push_back(device);
    if (!device->Start()) {
      OLA_WARN << "Failed to start device";
      return false;
    }
    m_plugin_adaptor->RegisterDevice(device);
    if (!device->openInterface(m_plugin_adaptor)) {
      OLA_WARN << "Failed to open interface";
      return false;
    }

    if (device->getInputPort() != NULL)
      m_input_ports.push_back(device->getInputPort());
  }

  if (m_devices.empty())
    OLA_INFO << "found no connected interfaces";
  else
    OLA_INFO << "found " << m_devices.size()
             << " connected interfaces providing " << m_input_ports.size() << " inputs";

  return true;
}


/*
 * Close all fds
 */
int EnlightenmentPlugin::CleanupDevices() {
  m_input_ports.clear();

  for (std::list<EnlightenmentDevice*>::iterator it = m_devices.begin(); it != m_devices.end(); ++it) {
    OLA_INFO << "closing device " << (*it)->DeviceId();
    m_plugin_adaptor->UnregisterDevice(*it);
    (*it)->Stop();
    delete (*it);
  }
  m_devices.clear();


  return 0;
}

}  // namespace enlightenment
}  // namespace plugin
}  // namespace ola
