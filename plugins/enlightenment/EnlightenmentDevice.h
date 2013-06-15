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
