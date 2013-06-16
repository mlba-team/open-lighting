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
    EnlightenmentDevice(class EnlightenmentPlugin *owner,
                        const string &name,
                        char* device_serial,
                        int device_mode);
    ~EnlightenmentDevice();

    bool openInterface(class PluginAdaptor *plugin_adaptor);
    char* InterfaceSerial() { return m_device_serial; }
    string InterfaceSerialStr() const { return string(m_device_serial); }
    unsigned int InterfaceVersion();
    int getFd() const { return m_fd; }
    EnlightenmentInputPort* getInputPort() const { return m_input; }
    EnlightenmentOutputPort* getOutputPort() const { return m_output; }


    string DeviceId() const { return InterfaceSerialStr(); }

    bool AllowLooping() const { return m_allow_multiport_patching; }
    bool AllowMultiPortPatching() const { return m_allow_multiport_patching; }

  private:
    EnlightenmentInputPort* m_input;
    EnlightenmentOutputPort* m_output;
    char m_device_serial[17];
    int m_device_mode;
    int m_fd;

    bool m_allow_multiport_patching;
};

}  // namespace enlightenment
}  // namespace plugin
}  // namespace ola
#endif  // PLUGINS_ENLIGHTENMENT_ENLIGHTENMENTDEVICE_H_
