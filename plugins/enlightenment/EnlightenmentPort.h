
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
