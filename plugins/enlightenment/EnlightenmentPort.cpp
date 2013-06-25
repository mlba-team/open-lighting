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


#include <string.h>
#include <errno.h>
#include <assert.h>
#include <ola/BaseTypes.h>
#include <ola/Logging.h>

#include "plugins/enlightenment/EnlightenmentPort.h"
#include "plugins/enlightenment/EnlightenmentDevice.h"

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
  // NOTE: Simply copying the entire universe
  //       would not trigger the driver properly
  for (size_t i = 0; i < buffer.Size(); ++i) {
    // NOTE: The test is to reduce the data sent to
    //       the interface, might be removed in the future
    if (m_lastvalue_buffer[i] != buffer.Get(i))
      m_lastvalue_buffer[i] = m_output_array[i] = buffer.Get(i);
  }

  // mark as used to suppress compiler warning
  (void)priority;

  return true;
}

/*
 * Read operation
 * @return a DmxBuffer with the data
 */
const DmxBuffer &EnlightenmentInputPort::ReadDMX() const {
  return m_read_buffer;
}


/*
 * Process new Data
 */
bool EnlightenmentInputPort::UpdateData() {
  DmxBuffer tmp_buffer;
  tmp_buffer.Set(m_input_array, DMX_UNIVERSE_SIZE);

  // only send on changes
  if (!(tmp_buffer == m_read_buffer)) {
    m_read_buffer.Set(tmp_buffer);
    DmxChanged();
  }

  return true;
}


}  // namespace enlightenment
}  // namespace plugin
}  // namespace ola

