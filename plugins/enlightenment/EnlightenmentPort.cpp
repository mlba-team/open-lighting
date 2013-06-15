
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <ola/BaseTypes.h>
#include <ola/Logging.h>

#include "EnlightenmentPort.h"
#include "EnlightenmentDevice.h"

namespace ola {
namespace plugin {
namespace enlightenment {


/*
 * Write operation
 * @param buffer the DmxBuffer to write
 * @return true on success, false on failure
 */
bool EnlightenmentOutputPort::WriteDMX(const DmxBuffer &buffer,
                                       uint8_t priority) {

  assert(buffer.Size() == DMX_UNIVERSE_SIZE);

  // NOTE: Simply copying the entire universe
  //       would not trigger the driver properly
  for(size_t i = 0; i < DMX_UNIVERSE_SIZE; ++i) {
    // NOTE: The test is to reduce the data sent to
    //       the interface, might be removed in the future
    if(_lastvalue_buffer[i] != buffer.Get(i))
      _lastvalue_buffer[i] = _output_array[i] = buffer.Get(i);
  }

  // TODO: Do we need to use this?
  (void)priority;

  return true;
}

/*
 * Read operation
 * @return a DmxBuffer with the data
 */
const DmxBuffer &EnlightenmentInputPort::ReadDMX() const {
  return _read_buffer;
}


/*
 * Process new Data
 */
bool EnlightenmentInputPort::UpdateData() {

  DmxBuffer tmp_buffer;
  tmp_buffer.Set(_input_array, DMX_UNIVERSE_SIZE);

  // only send on changes
  if (!(tmp_buffer == _read_buffer)) {
    _read_buffer.Set(tmp_buffer);
    DmxChanged();
  }

  return true;
}


}  // enlightenment
}  // plugin
}  // ola

