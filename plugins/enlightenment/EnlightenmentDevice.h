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

#ifndef OLA_ENLIGHTENMENT_DEVICE_H_
#define OLA_ENLIGHTENMENT_DEVICE_H_

#include <string>
#include <olad/Device.h>

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
    char* InterfaceSerial() { return _device_serial; }
    string InterfaceSerialStr() const { return string(_device_serial); }
    unsigned int InterfaceVersion();
    int getFd() const { return _fd; }
    EnlightenmentInputPort* getInputPort() const { return _input; }
    EnlightenmentOutputPort* getOutputPort() const { return _output; }


    string DeviceId() const { return InterfaceSerialStr(); }

    bool AllowLooping() const { return _allow_multiport_patching; }
    bool AllowMultiPortPatching() const { return _allow_multiport_patching; }

  private:
    EnlightenmentInputPort* _input;
    EnlightenmentOutputPort* _output;
    char _device_serial[17];
    int _device_mode;
    int _fd;

    bool _allow_multiport_patching;
};

}  // enlightenment
}  // plugin
}  // ola
#endif  // OLA_ENLIGHTENMENT_DEVICE_H_
