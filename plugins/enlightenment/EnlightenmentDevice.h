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

#ifndef PLUGINS_ENLIGHTENMENT_ENLIGHTENMENTDEVICE_H_
#define PLUGINS_ENLIGHTENMENT_ENLIGHTENMENTDEVICE_H_

#include <olad/Device.h>
#include <string>

namespace ola {
namespace plugin {
namespace enlightenment {

class EnlightenmentInputPort;
class EnlightenmentOutputPort;

class EnlightenmentDevice: public ola::Device {
  public:
    enum DeviceMode {
        // 0: Do nothing - Standby
       STANDBY = 0,
        // 1: DMX In -> DMX Out
       DMXIN_TO_DMXOUT = 1,
        // 2: PC Out -> DMX Out
       PCOUT_TO_DMXOUT = 2,
        // 3: DMX In + PC Out -> DMX Out
       DMXIN_PCIN_TO_DMXOUT = 3,
        // 4: DMX In -> PC In
       DMXIN_TO_PCIN = 4,
        // 5: DMX In -> DMX Out & DMX In -> PC In
       DMXIN_TO_PCOUT_AND_DMXIN_TO_PCIN = 5,
        // 6: PC Out -> DMX Out & DMX In -> PC In
       PCOUT_TO_DMXOUT_AND_DMXIN_TO_PCIN = 6,
        // 7: DMX In + PC Out -> DMX Out & DMX In -> PC In
       DMXIN_PCOUT_TO_DMXOUT_AND_DMXIN_TO_PCIN = 7
    };

    EnlightenmentDevice(class EnlightenmentPlugin *owner,
                        const string &name,
                        const string &device_serial,
                        DeviceMode device_mode);
    ~EnlightenmentDevice();

    bool openInterface(class PluginAdaptor *plugin_adaptor);
    inline char *InterfaceSerial() {
        return const_cast<char*>(m_device_serial.c_str());
    }
    const string &InterfaceSerialStr() const { return m_device_serial; }
    unsigned int InterfaceVersion();
    int getFd() const { return m_fd; }
    EnlightenmentInputPort* getInputPort() const { return m_input; }
    EnlightenmentOutputPort* getOutputPort() const { return m_output; }


    string DeviceId() const { return InterfaceSerialStr(); }

    bool AllowLooping() const { return m_allow_multiport_patching; }
    bool AllowMultiPortPatching() const { return m_allow_multiport_patching; }

    void IncomingChanges();

  private:
    EnlightenmentInputPort* m_input;
    EnlightenmentOutputPort* m_output;
    string m_device_serial;
    DeviceMode m_device_mode;
    int m_fd;

    bool m_allow_multiport_patching;
};

}  // namespace enlightenment
}  // namespace plugin
}  // namespace ola
#endif  // PLUGINS_ENLIGHTENMENT_ENLIGHTENMENTDEVICE_H_
