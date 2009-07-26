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
 * EspNetNode.cpp
 * A EspNet node
 * Copyright (C) 2005-2009 Simon Newton
 */

#include <string.h>
#include <algorithm>
#include <lla/Logging.h>
#include "EspNetNode.h"


namespace lla {
namespace espnet {

using std::string;
using std::map;
using lla::network::UdpSocket;
using lla::Closure;

const string EspNetNode::NODE_NAME = "LLA Node";

/*
 * Create a new node
 * @param ip_address the IP address to prefer to listen on, if NULL we choose
 * one.
 */
EspNetNode::EspNetNode(const string &ip_address):
  m_running(false),
  m_options(DEFAULT_OPTIONS),
  m_tos(DEFAULT_TOS),
  m_ttl(DEFAULT_TTL),
  m_universe(0),
  m_type(ESPNET_NODE_TYPE_IO),
  m_node_name(NODE_NAME),
  m_preferred_ip(ip_address),
  m_socket(NULL) {
}


/*
 * Cleanup
 */
EspNetNode::~EspNetNode() {
  Stop();

  std::map<uint8_t, universe_handler>::iterator iter;
  for (iter = m_handlers.begin(); iter != m_handlers.end(); ++iter) {
    delete iter->second.closure;
  }
  m_handlers.clear();
}


/*
 * Start this node
 */
bool EspNetNode::Start() {
  if (m_running)
    return false;

  if (!m_interface_picker.ChooseInterface(m_interface, m_preferred_ip)) {
    LLA_INFO << "Failed to find an interface";
    return false;
  }

  if (!InitNetwork())
    return false;

  m_running = true;
  return true;
}


/*
 * Stop this node
 */
bool EspNetNode::Stop() {
  if (!m_running)
    return false;

  if (m_socket) {
    delete m_socket;
    m_socket = NULL;
  }

  m_running = false;
  return true;
}


/*
 * Called when there is data on this socket
 */
int EspNetNode::SocketReady() {
  espnet_packet_union_t packet;
  memset(&packet, 0, sizeof(packet));
  struct sockaddr_in source;
  socklen_t source_length = sizeof(source);

  ssize_t packet_size = sizeof(packet);
  if(!m_socket->RecvFrom((uint8_t*) &packet, packet_size, source,
                          source_length))
    return -1;

  if (packet_size < (ssize_t) sizeof(packet.poll.head)) {
    LLA_WARN << "Small espnet packet received, discarding";
    return -1;
  }

  // skip packets sent by us
  if (source.sin_addr.s_addr == m_interface.ip_address.s_addr) {
    return 0;
  }

  switch (ntohl(packet.poll.head)) {
    case ESPNET_POLL:
      HandlePoll(packet.poll, source.sin_addr);
      break;
    case ESPNET_REPLY:
      HandleReply(packet.reply, source.sin_addr);
      break;
    case ESPNET_DMX:
      HandleData(packet.dmx, source.sin_addr);
      break;
    case ESPNET_ACK:
      HandleAck(packet.ack, source.sin_addr);
      break;
    default:
      LLA_INFO << "Skipping a packet with invalid header" << packet.poll.head;
  }

  return 0;
}


/*
 * Set the closure to be called when we receive data for this universe.
 * @param universe the universe to register the handler for
 * @param handler the Closure to call when there is data for this universe.
 * Ownership of the closure is transferred to the node.
 */
bool EspNetNode::SetHandler(uint8_t universe, Closure *closure) {
  if (!closure)
    return false;

  map<uint8_t, universe_handler>::iterator iter =
    m_handlers.find(universe);

  if (iter == m_handlers.end()) {
    universe_handler handler;
    handler.closure = closure;
    handler.buffer.Blackout();
    m_handlers[universe] = handler;
  } else {
    Closure *old_closure = iter->second.closure;
    iter->second.closure = closure;
    delete old_closure;
  }
  return true;
}


/*
 * Remove the handler for this universe
 * @param universe the universe handler to remove
 * @param true if removed, false if it didn't exist
 */
bool EspNetNode::RemoveHandler(uint8_t universe) {
  map<uint8_t, universe_handler>::iterator iter =
    m_handlers.find(universe);

  if (iter != m_handlers.end()) {
    Closure *old_closure = iter->second.closure;
    m_handlers.erase(iter);
    delete old_closure;
    return true;
  }
  return false;
}


/*
 * Get the DMX data for this universe.
 */
DmxBuffer EspNetNode::GetDMX(uint8_t universe) {
  map<uint8_t, universe_handler>::const_iterator iter =
    m_handlers.find(universe);

  if (iter != m_handlers.end())
    return iter->second.buffer;
  else {
    DmxBuffer buffer;
    return buffer;
  }
}


/*
 * Send an Esp Poll
 * @param full_poll
 */
bool EspNetNode::SendPoll(bool full_poll) {
  if (!m_running)
    return false;

  return SendEspPoll(m_interface.bcast_address, full_poll);
}


/*
 * Send some DMX data
 * @param universe the id of the universe to send
 * @param buffer the DMX data
 * @return true if it was send successfully, false otherwise
 */
bool EspNetNode::SendDMX(uint8_t universe, const lla::DmxBuffer &buffer) {
  if (!m_running)
    return false;

  return SendEspData(m_interface.bcast_address, universe, buffer);
}


/*
 * Setup the networking compoents.
 */
bool EspNetNode::InitNetwork() {
  m_socket = new UdpSocket();

  if (!m_socket->Init()) {
    LLA_WARN << "Socket init failed";
    delete m_socket;
    return false;
  }

  if (!m_socket->Bind(ESPNET_PORT)) {
    LLA_WARN << "Failed to bind to:" << ESPNET_PORT;
    delete m_socket;
    return false;
  }

  if (!m_socket->EnableBroadcast()) {
    LLA_WARN << "Failed to enable broadcasting";
    delete m_socket;
    return false;
  }

  m_socket->SetOnData(NewClosure(this, &EspNetNode::SocketReady));
  return true;
}


/*
 * Handle an Esp Poll packet
 */
void EspNetNode::HandlePoll(const espnet_poll_t &poll,
                            const struct in_addr &source) {
  LLA_DEBUG << "Got ESP Poll " << poll.type;
  if (poll.type)
    SendEspPollReply(source);
  else
    SendEspAck(source, 0, 0);
}

/*
 * Handle an Esp reply packet
 */
void EspNetNode::HandleReply(const espnet_poll_reply_t &reply,
                             const struct in_addr &source) {
  //TODO: Call a handler here
}


/*
 * Handle a Esp Ack packet
 */
void EspNetNode::HandleAck(const espnet_ack_t &ack,
                           const struct in_addr &source) {

}


/*
 * Handle an Esp data packet
 */
void EspNetNode::HandleData(const espnet_data_t &data,
                            const struct in_addr &source) {

  map<uint8_t, universe_handler>::iterator iter =
    m_handlers.find(data.universe);

  if (iter == m_handlers.end()) {
    LLA_DEBUG << "Not interested in universe " << int(data.universe) <<
      ", skipping ";
    return;
  }

  // we ignore the start code
  switch (data.type) {
    case DATA_RAW:
      iter->second.buffer.Set(data.data, ntohs(data.size));
      break;
    case DATA_PAIRS:
      LLA_WARN << "espnet data pairs aren't supported";
      return;
    case DATA_RLE:
      m_decoder.Decode(iter->second.buffer, data.data, ntohs(data.size));
      break;
    default:
      LLA_WARN << "unknown espnet data type " << data.type;
      return;
  }

  iter->second.closure->Run();
}


/*
 * Send an EspNet poll
 */
bool EspNetNode::SendEspPoll(const struct in_addr &dst, bool full) {
  espnet_packet_union_t packet;
  packet.poll.head = htonl(ESPNET_POLL);
  packet.poll.type = full;
  return SendPacket(dst, packet, sizeof(packet.poll));
}


/*
 * Send an EspNet Ack
 */
bool EspNetNode::SendEspAck(const struct in_addr &dst,
                            uint8_t status,
                            uint8_t crc) {
  espnet_packet_union_t packet;
  packet.ack.head = htonl(ESPNET_ACK);
  packet.ack.status = status;
  packet.ack.crc = crc;
  return SendPacket(dst, packet, sizeof(packet.ack));
}


/*
 * Send an EspNet Poll Reply
 */
bool EspNetNode::SendEspPollReply(const struct in_addr &dst) {
  espnet_packet_union_t packet;
  packet.reply.head = htonl(ESPNET_REPLY);

  memcpy(packet.reply.mac, m_interface.hw_address, lla::network::MAC_LENGTH) ;
  packet.reply.type = htons(m_type);
  packet.reply.version = FIRMWARE_VERSION;
  packet.reply.sw = SWITCH_SETTINGS;
  memcpy(packet.reply.name, m_node_name.data(), ESPNET_NAME_LENGTH);
  packet.reply.name[ESPNET_NAME_LENGTH-1] = 0;

  packet.reply.option = m_options;
  packet.reply.option = 0x01;
  packet.reply.tos = m_tos;
  packet.reply.ttl = m_ttl;
  packet.reply.config.listen = 0x04;
  memcpy(&packet.reply.config.ip, &m_interface.ip_address.s_addr,
         sizeof(packet.reply.config.ip));
  packet.reply.config.universe = m_universe;
  return SendPacket(dst, packet, sizeof(packet.reply));
}


/*
 * Send an EspNet data packet
 */
bool EspNetNode::SendEspData(const struct in_addr &dst,
                             uint8_t universe,
                             const DmxBuffer &buffer) {

  espnet_packet_union_t packet;
  memset(&packet.dmx, 0, sizeof(packet.dmx));
  packet.dmx.head = htonl(ESPNET_DMX);
  packet.dmx.universe = universe;
  packet.dmx.start = START_CODE;
  packet.dmx.type = DATA_RAW;
  unsigned int size = ESPNET_DMX_LENGTH;
  buffer.Get(packet.dmx.data, size);
  packet.dmx.size = htons(size);

  return SendPacket(dst, packet, sizeof(packet.dmx));
}


/*
 * Send an EspNet packet
 */
bool EspNetNode::SendPacket(const struct in_addr &dst,
                            const espnet_packet_union_t &packet,
                            unsigned int size) {
  struct sockaddr_in m_destination;
  memset(&m_destination, 0, sizeof(m_destination));
  m_destination.sin_family = AF_INET;
  m_destination.sin_port = htons(ESPNET_PORT);
  m_destination.sin_addr = dst;

  ssize_t bytes_sent = m_socket->SendTo((uint8_t*) &packet,
                                        size,
                                        m_destination);
  if (bytes_sent != (ssize_t) size) {
    LLA_WARN << "Only sent " << bytes_sent << " of " << size;
    return false;
  }
  return true;
}

} //espnet
} //lla