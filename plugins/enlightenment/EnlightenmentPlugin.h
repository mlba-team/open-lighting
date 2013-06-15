#ifndef OLA_ENLIGHTENMENT_PLUGIN_H_
#define OLA_ENLIGHTENMENT_PLUGIN_H_

#include <list>
#include <string>
#include <ola/DmxBuffer.h>
#include <olad/Plugin.h>
#include <ola/network/Socket.h>
#include <ola/plugin_id.h>

namespace ola {
namespace plugin {
namespace enlightenment {

class EnlightenmentDevice;
class EnlightenmentInputPort;

class EnlightenmentPlugin: public ola::Plugin {
  public:
    explicit EnlightenmentPlugin(PluginAdaptor *plugin_adaptor):
      Plugin(plugin_adaptor),
      _device_mode(6) {}
    ~EnlightenmentPlugin();

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

    std::list<EnlightenmentDevice*> _devices;
    std::list<EnlightenmentInputPort*> _input_ports;
    int _device_mode;  // the mode of the device

    static const char DEVICE_MODE_KEY[];
    static const char DEFAULT_DEVICE_MODE[];
    static const char PLUGIN_NAME[];
    static const char PLUGIN_PREFIX[];
};

}  // enlightenment
}  // plugin
}  // ola

#endif  // OLA_ENLIGHTENMENT_PLUGIN_H_
