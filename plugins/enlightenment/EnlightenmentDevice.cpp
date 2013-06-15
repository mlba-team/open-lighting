#include <string>
#include "EnlightenmentPlugin.h"
#include "EnlightenmentDevice.h"
#include "EnlightenmentPort.h"

#include <ola/Logging.h>

extern "C" {
#include <usbdmx.h>
}

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
                                         char* device_id,
                                         int device_mode
                                         )
  : Device(owner, name), _input(NULL), _output(NULL), _device_mode(device_mode),
    _fd(0), _allow_multiport_patching(true) {
  // this is the id the device is attached to
  memcpy(_device_serial, device_id, 16 );
  _device_serial[16] = '\0';
}

unsigned int EnlightenmentDevice::InterfaceVersion() {
   return GetDeviceVersion(_device_serial);
}

bool EnlightenmentDevice::openInterface(class PluginAdaptor *plugin_adaptor) {
  // create the ports (according to requested interface mode)
  // 0: Do nothing - Standby
  // 1: DMX In -> DMX Out
  // 2: PC Out -> DMX Out
  // 3: DMX In + PC Out -> DMX Out
  // 4: DMX In -> PC In
  // 5: DMX In -> DMX Out & DMX In -> PC In
  // 6: PC Out -> DMX Out & DMX In -> PC In
  // 7: DMX In + PC Out -> DMX Out & DMX In -> PC In
  switch(_device_mode) {
    case 4:
    case 5:
    case 6:
    case 7:
      _input = new EnlightenmentInputPort(this, plugin_adaptor, 0 /* TODO: Which id goes here? */);
      OLA_DEBUG << "created new input port";
      if(_input == NULL)
        return false;
      this->AddPort(_input);
      break;
  }
  switch(_device_mode) {
    case 2:
    case 3:
    case 6:
    case 7:
      _output = new EnlightenmentOutputPort(this, 0 /* TODO: Which id goes here? */);
      OLA_DEBUG << "created new output port";
      if(_output == NULL)
        return false;
      this->AddPort(_output);
      break;
  }

  // open the interface
  int fd = OpenLink(_device_serial, _output ? _output->getTDMXArray() : NULL, _input ? _input->getTDMXArray() : NULL);
  if(fd != 1) {
    OLA_WARN << "Unable to open Interface, did you install the udev-rule?\n";
    return false;
  } else {
    _fd = fd;
  }
  // configure the mode
  SetInterfaceMode(_device_serial, _device_mode);

  // mode 7 is the only mode, multport patching is not allowed
  if(_device_mode==7)
    _allow_multiport_patching = false;

  return true;
}

EnlightenmentDevice::~EnlightenmentDevice() {
  CloseLink( _device_serial );
}

}  // enlightenment
}  // plugin
}  // ola

