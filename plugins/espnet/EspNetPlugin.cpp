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
 *
 * EspNetPlugin.cpp
 * The Esp Net plugin for lla
 * Copyright (C) 2005  Simon Newton
 */

#include <stdlib.h>
#include <stdio.h>

#include <llad/Preferences.h>

#include "EspNetPlugin.h"
#include "EspNetDevice.h"

/*
 * Entry point to this plugin
 */
extern "C" lla::AbstractPlugin* create(const lla::PluginAdaptor *plugin_adaptor) {
  return new lla::plugin::EspNetPlugin(plugin_adaptor);
}

/*
 * Called when the plugin is unloaded
 */
extern "C" void destroy(lla::AbstractPlugin* plugin) {
  delete plugin;
}


namespace lla {
namespace plugin {

const string EspNetPlugin::ESPNET_NODE_NAME = "lla-EspNet";
const string EspNetPlugin::ESPNET_DEVICE_NAME = "EspNet Device";
const string EspNetPlugin::PLUGIN_NAME = "EspNet Plugin";
const string EspNetPlugin::PLUGIN_PREFIX = "espnet";


/*
 * Start the plugin
 *
 * For now we just have one device.
 */
bool EspNetPlugin::StartHook() {
  m_device = new EspNetDevice(this,
                              ESPNET_DEVICE_NAME,
                              m_preferences,
                              m_plugin_adaptor);

  if (!m_device)
    return false;

  if(!m_device->Start()) {
    delete m_device;
    return false;
  }
  m_plugin_adaptor->RegisterDevice(m_device);
  return true;
}


/*
 * Stop the plugin
 *
 * @return 0 on sucess, -1 on failure
 */
bool EspNetPlugin::StopHook() {
  m_device->Stop();
  m_plugin_adaptor->UnregisterDevice(m_device);
  delete m_device;
  return true;
}


/*
 * Return the description for this plugin
 */
string EspNetPlugin::Description() const {
  return
"EspNet Plugin\n"
"----------------------------\n"
"\n"
"This plugin creates a single device with five input and five output ports.\n"
"\n"
"Esp supports up to 255 universes. As ESP has no notion of ports, we provide "
"a fixed number of ports which can be patched to any universe. When sending "
"data from a port, the data is addressed to the universe the port is patched "
"to. For example if port 0 is patched to universe 10, the data will be sent to "
"ESP universe 10.\n"
"\n"
"--- Config file : lla-espnet.conf ---\n"
"\n"
"ip = a.b.c.d\n"
"The ip address to bind to. If not specified it will use the first non-loopback ip.\n"
"\n"
"name = lla-EspNet\n"
"The name of the node.\n";

}

/*
 * load the plugin prefs and default to sensible values
 *
 */
int EspNetPlugin::SetDefaultPreferences() {
  if (!m_preferences)
    return -1;

  // we don't worry about ip here
  // if it's non existant it will choose one
  if (m_preferences->GetValue("name").empty()) {
    m_preferences->SetValue("name", ESPNET_NODE_NAME);
    m_preferences->Save();
  }

  // check if this save correctly
  // we don't want to use it if null
  if (m_preferences->GetValue("name").empty())
    return -1;

  return 0;
}

} //plugin
} //lla