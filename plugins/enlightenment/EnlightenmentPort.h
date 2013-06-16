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


#ifndef OLA_ENLIGHTENMENT_PORT_H_
#define OLA_ENLIGHTENMENT_PORT_H_

#include <string>
#include <ola/BaseTypes.h>
#include <ola/DmxBuffer.h>
#include "EnlightenmentDevice.h"

extern "C" {
  #include <usbdmx.h>
}

namespace ola {
namespace plugin {
namespace enlightenment {


/*
 * An output port
 */
class EnlightenmentOutputPort: public BasicOutputPort {
  public:
    EnlightenmentOutputPort(EnlightenmentDevice *parent,
                            int port_id)
      : BasicOutputPort(parent, port_id) {
      memset(_output_array, 0, sizeof(TDMXArray));
      memset(_lastvalue_buffer, 0, sizeof(TDMXArray));

      _description = "Output of device '" + parent->InterfaceSerialStr() + "'";
    }

    bool WriteDMX(const DmxBuffer &buffer, uint8_t priority);
    TDMXArray* getTDMXArray(){ return &_output_array; }
    string Description() const { return _description; }

  private:
    TDMXArray _output_array;
    TDMXArray _lastvalue_buffer;

    std::string _description;
};


/*
 * An input port
 */
class EnlightenmentInputPort: public BasicInputPort {
  public:
    explicit EnlightenmentInputPort(EnlightenmentDevice *parent,
                                    class PluginAdaptor *plugin_adaptor,
                                    int port_id):
      BasicInputPort(parent, port_id, plugin_adaptor) {
      _read_buffer.SetRangeToValue(0, 0, DMX_UNIVERSE_SIZE);
      memset(_input_array, 0, sizeof(TDMXArray));

      _description = "Input of device '" + parent->InterfaceSerialStr() + "'";
    }

    const DmxBuffer &ReadDMX() const;
    bool UpdateData();
    TDMXArray* getTDMXArray(){ return &_input_array; }
    string Description() const { return _description; }

  private:
    TDMXArray _input_array;
    DmxBuffer _read_buffer;

    std::string _description;
};

}  // enlightenment
}  // plugin
}  // ola

#endif  // OLA_ENLIGHTENMENT_PORT_H_
