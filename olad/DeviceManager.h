/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 * DeviceManager.h
 * Interface to the DeviceManager class
 * Copyright (C) 2005-2009 Simon Newton
 *
 * The DeviceManager assigns an unsigned int as an alias to each device which
 * remains consistent throughout the lifetime of the DeviceManager. These are
 * used in the user facing portion as '1' is easier to understand/type
 * than 5-02050016. If a device is registered, then unregistered, then
 * registered again, it'll have the same device alias.
 *
 * The DeviceManager is also responsible for restoring the port patchings when
 * devices are registered.
 */

#ifndef OLA_DEVICEMANAGER_H
#define OLA_DEVICEMANAGER_H

#include <map>
#include <string>
#include <olad/Device.h>
#include "UniverseStore.h"
#include <olad/Preferences.h>

namespace ola {

using std::map;
using std::string;


// pair a device with it's alias
typedef struct {
  unsigned int alias;
  AbstractDevice *device;
} device_alias_pair;

bool operator <(const device_alias_pair& left, const device_alias_pair &right);

class DeviceManager {
  public:
    DeviceManager(PreferencesFactory *prefs_factory,
                  UniverseStore *universe_store);
    ~DeviceManager();

    bool RegisterDevice(AbstractDevice *device);
    bool UnregisterDevice(const string &device_id);
    bool UnregisterDevice(const AbstractDevice *device);
    unsigned int DeviceCount() const;
    vector<device_alias_pair> Devices() const;
    AbstractDevice *GetDevice(unsigned int alias) const;
    device_alias_pair GetDevice(const string &unique_id) const;
    void UnregisterAllDevices();

    static const unsigned int MISSING_DEVICE_ALIAS;

  private:
    UniverseStore *m_universe_store;
    Preferences *m_port_preferences;
    map<string, device_alias_pair> m_devices; // map device_ids to devices
    map<unsigned int, AbstractDevice*> m_alias_map; // map alias to devices
    unsigned int m_next_device_alias;

    DeviceManager(const DeviceManager&);
    DeviceManager& operator=(const DeviceManager&);
    void SaveDevicePortPatchings(AbstractDevice *device);
    void RestoreDevicePortPatchings(AbstractDevice *device);

    static const string PORT_PREFERENCES;
    static const unsigned int FIRST_DEVICE_ALIAS = 1;
};


} //ola
#endif