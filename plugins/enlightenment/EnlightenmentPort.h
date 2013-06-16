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


#ifndef PLUGINS_ENLIGHTENMENT_ENLIGHTENMENTPORT_H_
#define PLUGINS_ENLIGHTENMENT_ENLIGHTENMENTPORT_H_

#include <ola/BaseTypes.h>
#include <ola/DmxBuffer.h>

extern "C" {
  #include <usbdmx.h>
}

#include <string>

#include "plugins/enlightenment/EnlightenmentDevice.h"

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
      memset(m_output_array, 0, sizeof(TDMXArray));
      memset(m_lastvalue_buffer, 0, sizeof(TDMXArray));

      m_description = "Output of device '" + parent->InterfaceSerialStr() + "'";
    }

    bool WriteDMX(const DmxBuffer &buffer, uint8_t priority);
    TDMXArray* getTDMXArray() { return &m_output_array; }
    string Description() const { return m_description; }

  private:
    TDMXArray m_output_array;
    TDMXArray m_lastvalue_buffer;

    std::string m_description;
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
      m_read_buffer.SetRangeToValue(0, 0, DMX_UNIVERSE_SIZE);
      memset(m_input_array, 0, sizeof(TDMXArray));

      m_description = "Input of device '" + parent->InterfaceSerialStr() + "'";
    }

    const DmxBuffer &ReadDMX() const;
    bool UpdateData();
    TDMXArray* getTDMXArray() { return &m_input_array; }
    string Description() const { return m_description; }

  private:
    TDMXArray m_input_array;
    DmxBuffer m_read_buffer;

    std::string m_description;
};

}  // namespace enlightenment
}  // namespace plugin
}  // namespace ola

#endif  // PLUGINS_ENLIGHTENMENT_ENLIGHTENMENTPORT_H_
