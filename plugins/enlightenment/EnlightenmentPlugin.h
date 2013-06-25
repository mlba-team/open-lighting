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

#ifndef PLUGINS_ENLIGHTENMENT_ENLIGHTENMENTPLUGIN_H_
#define PLUGINS_ENLIGHTENMENT_ENLIGHTENMENTPLUGIN_H_

#include <ola/DmxBuffer.h>
#include <olad/Plugin.h>
#include <ola/network/Socket.h>
#include <ola/plugin_id.h>

#include <set>
#include <string>

namespace ola {
namespace plugin {
namespace enlightenment {

class EnlightenmentDevice;
class EnlightenmentInputPort;

class EnlightenmentPlugin: public ola::Plugin {
  public:
    explicit EnlightenmentPlugin(PluginAdaptor *plugin_adaptor)
        : Plugin(plugin_adaptor),
          m_device_mode(DEFAULT_DEVICEMODE) {}
    ~EnlightenmentPlugin();

    enum {
        DEFAULT_DEVICEMODE = 6,
        SERIAL_LENGTH = 17
    };

    string Name() const { return PLUGIN_NAME; }
    string Description() const;
    ola_plugin_id Id() const { return OLA_PLUGIN_ENLIGHTENMENT; }

    void IncomingChanges();
    string PluginPrefix() const { return PLUGIN_PREFIX; }

  private:
    bool StartHook();
    bool StopHook();
    bool SetDefaultPreferences();

    bool SetupDevices();
    int CleanupDevices();

    std::set<EnlightenmentDevice*> m_devices;
    int m_device_mode;  // the mode of the device

    static const char DEVICE_MODE_KEY[];
    static const char DEFAULT_DEVICE_MODE[];
    static const char PLUGIN_NAME[];
    static const char PLUGIN_PREFIX[];
};

}  // namespace enlightenment
}  // namespace plugin
}  // namespace ola

#endif  // PLUGINS_ENLIGHTENMENT_ENLIGHTENMENTPLUGIN_H_
