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
 * StreamRpcChannel.cpp
 * Interface for the UDP RPC Channel
 * Copyright (C) 2005-2008 Simon Newton
 */

#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include <string>

#include <google/protobuf/service.h>
#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/dynamic_message.h>

#include <lla/select_server/Socket.h>
#include "StreamRpcChannel.h"
#include "SimpleRpcController.h"
#include "Rpc.pb.h"

using namespace google::protobuf;
using namespace lla::rpc;
using namespace lla::select_server;
using namespace std;


StreamRpcChannel::StreamRpcChannel(Service *service, ConnectedSocket *socket) :
  m_service(service),
  m_socket(socket),
  m_buffer(NULL),
  m_buffer_size(0),
  m_seq(0),
  m_expected_size(0),
  m_current_size(0) {
    m_socket->SetListener(this);
}


StreamRpcChannel::~StreamRpcChannel() {
  free(m_buffer);
}


/*
 * Receive a message for this RPCChannel. Called when data is available on the socket.
 */
int StreamRpcChannel::SocketReady(ConnectedSocket *socket) {
  if (!m_expected_size) {
    // this is a new msg
    unsigned int version;
    if (ReadHeader(version, m_expected_size) < 0)
      return -1;

    if (!m_expected_size)
      return 0;

    if (version != PROTOCOL_VERSION) {
      printf("protocol mismatch %d != %d\n", version, PROTOCOL_VERSION);
      return -1;
    }
    m_current_size = 0;
    m_buffer_size = AllocateMsgBuffer(m_expected_size);

    if (m_buffer_size < m_expected_size) {
      printf("buffer size to small %d < %d\n", m_buffer_size, m_expected_size);
      return -1;
    }
  }

  unsigned int data_read;
  if (socket->Receive(m_buffer + m_current_size,
                      m_expected_size - m_current_size,
                      data_read) < 0) {
    printf("something went wrong in socket recv\n");
    return -1;
  }

  m_current_size += data_read;

  if (m_current_size == m_expected_size) {
    // we've got all of this message so parse it.
    HandleNewMsg(m_buffer, m_expected_size);
    m_expected_size = 0;
  }
  return 0;
}



void StreamRpcChannel::CallMethod(
    const MethodDescriptor *method,
    RpcController *controller,
    const Message *request,
    Message *reply,
    Closure *done) {
  /*
   * Call a method with the given request and reply
   * TODO(simonn): reduce the number of copies here
   */
  string output;
  RpcMessage message;

  message.set_type(REQUEST);
  message.set_id(m_seq++);
  message.set_name(method->name());

  request->SerializeToString(&output);
  message.set_buffer(output);
  SendMsg(&message);

  OutstandingResponse *response = GetOutstandingResponse(message.id());
  if (response) {
    // fail any outstanding response with the same id
    printf("response %d already pending, failing now\n", response->id);
    response->controller->SetFailed("Duplicate request found");
    InvokeCallbackAndCleanup(response);
  }

  response = new OutstandingResponse();
  response->id = message.id();
  response->controller = controller;
  response->callback = done;
  response->reply = reply;
  m_responses[message.id()] = response;
}


/*
 * Called when a response is ready.
 */
void StreamRpcChannel::RequestComplete(OutstandingRequest *request) {
  string output;
  RpcMessage message;

  if (request->controller->Failed()) {
    SendRequestFailed(request);
    return;
  }

  message.set_type(RESPONSE);
  message.set_id(request->id);
  request->response->SerializeToString(&output);
  message.set_buffer(output);
  SendMsg(&message);
  DeleteOutstandingRequest(request);
}


// private
//-------------------------------------------------------------------------------------
/*
 * Write an RpcMessage to the write socket.
 */
int StreamRpcChannel::SendMsg(RpcMessage *msg) {
  string output;
  msg->SerializeToString(&output);
  int length = output.length();
  uint32_t header;
  StreamRpcHeader::EncodeHeader(header, PROTOCOL_VERSION, length);

  int ret = m_socket->Send((uint8_t*) &header, sizeof(header));
  ret = m_socket->Send((uint8_t*) output.c_str(), length);

  if (-1 == ret) {
    printf("%s\n", strerror(errno));
    //Logger::instance()->log(Logger::CRIT, "Send failed: %s", strerror(errno) );
    return -1;
  } else if (length != ret) {
    //Logger::instance()->log(Logger::CRIT, "Failed to send full datagram");
    return -1;
  }
  return 0;
}


/**
 * Allocate an incomming message buffer
 *
 * Params:
 *   size: the size of the new buffer to allocate
 *
 * Returns:
 *   The size of the new buffer
 */
int StreamRpcChannel::AllocateMsgBuffer(unsigned int size) {
  int requested_size = size;
  uint8_t *new_buffer;

  if (size < m_buffer_size)
    return size;

  if (m_buffer_size == 0 && size < INITIAL_BUFFER_SIZE)
    requested_size = INITIAL_BUFFER_SIZE;

  if (requested_size > MAX_BUFFER_SIZE)
    return m_buffer_size;

  if ((new_buffer = (uint8_t*) realloc(m_buffer, requested_size)) < 0)
    return m_buffer_size;

  m_buffer = new_buffer;
  m_buffer_size = requested_size;
  return requested_size;
}


/*
 * Read 4 bytes and decode the header fields.
 *
 * Returns:
 *  -1 on error
 *  If no data is available, version and size are 0
 */
int StreamRpcChannel::ReadHeader(unsigned int &version, unsigned int &size) const {
  static const int HEADER_SIZE = 4;
  uint8_t header[HEADER_SIZE];
  unsigned int data_read = 0;
  version = size = 0;

  if (m_socket->Receive((uint8_t*) header, HEADER_SIZE, data_read)) {
    printf("read header error: %s\n", strerror(errno));
    return -1;
  }

  if (!data_read)
    return 0;

  StreamRpcHeader::DecodeHeader(*((uint32_t*) header), version, size);
  return 0;
}


/*
 * Parse a new message and handle it.
 */
void StreamRpcChannel::HandleNewMsg(uint8_t *data, unsigned int size) {
  RpcMessage msg;
  if (!msg.ParseFromArray(data, size)) {
    printf("parsing failed\n");
    return;
  }

  switch (msg.type()) {
    case REQUEST:
      HandleRequest(&msg);
      break;
    case RESPONSE:
      HandleResponse(&msg);
      break;
    case RESPONSE_CANCEL:
      HandleCanceledResponse(&msg);
      break;
    case RESPONSE_FAILED:
      HandleFailedResponse(&msg);
      break;
    case RESPONSE_NOT_IMPLEMENTED:
      HandleNotImplemented(&msg);
      break;
    default:
      printf("not sure of msg type %d\n", msg.type());
      break;
  }
}


/*
 * Handle a new RPC method call.
 */
void StreamRpcChannel::HandleRequest(RpcMessage *msg) {
  if (!m_service) {
    printf("no service registered\n");
    return;
  }

  const ServiceDescriptor *service = m_service->GetDescriptor();
  if (!service) {
    printf("failed to get service descriptor\n");
    return;
  }
  const MethodDescriptor *method = service->FindMethodByName(msg->name());
  if (!method) {
    printf("failed to get method descriptor\n");
    SendNotImplemented(msg->id());
    return;
  }

  Message* request_pb = m_service->GetRequestPrototype(method).New();
  Message* response_pb = m_service->GetResponsePrototype(method).New();

  if (!request_pb || !response_pb) {
    printf("failed to get request or response objects\n");
    return;
  }

  if (!request_pb->ParseFromString(msg->buffer())) {
    printf("parsing of request pb failed\n");
    return;
  }

  OutstandingRequest *request = new OutstandingRequest();
  request->id = msg->id();
  request->controller = new SimpleRpcController();
  request->response = response_pb;

  if (m_requests.find(msg->id()) != m_requests.end()) {
    printf("dup sequence number for request %d!!\n", msg->id());
    SendRequestFailed(m_requests[msg->id()]);
  }

  m_requests[msg->id()] = request;
  Closure *callback = NewCallback(this, &StreamRpcChannel::RequestComplete, request);
  m_service->CallMethod(method, request->controller, request_pb, response_pb, callback);
  delete request_pb;
}


// server side
/*
 * Notify the caller that the request failed.
 */
void StreamRpcChannel::SendRequestFailed(OutstandingRequest *request) {
  RpcMessage message;
  message.set_type(RESPONSE_FAILED);
  message.set_id(request->id);
  message.set_buffer(request->controller->ErrorText());
  SendMsg(&message);
  DeleteOutstandingRequest(request);
}


/*
 * Sent if we get a request for a non-existant method.
 */
void StreamRpcChannel::SendNotImplemented(int msg_id) {
  RpcMessage message;
  message.set_type(RESPONSE_NOT_IMPLEMENTED);
  message.set_id(msg_id);
  SendMsg(&message);
}


/*
 * Cleanup an outstanding request after the response has been returned
 */
void StreamRpcChannel::DeleteOutstandingRequest(OutstandingRequest *request) {
  m_requests.erase(request->id);
  delete request->controller;
  delete request->response;
  delete request;
}


// client side methods
/*
 * Handle a RPC response by invoking the callback.
 */
void StreamRpcChannel::HandleResponse(RpcMessage *msg) {
  OutstandingResponse *response = GetOutstandingResponse(msg->id());
  if (response) {
    response->reply->ParseFromString(msg->buffer());
    InvokeCallbackAndCleanup(response);
  }
}


/*
 * Handle a RPC response by invoking the callback.
 */
void StreamRpcChannel::HandleFailedResponse(RpcMessage *msg) {
  OutstandingResponse *response = GetOutstandingResponse(msg->id());
  if (response) {
    response->controller->SetFailed(msg->buffer());
    InvokeCallbackAndCleanup(response);
  }
}


/*
 * Handle a RPC response by invoking the callback.
 */
void StreamRpcChannel::HandleCanceledResponse(RpcMessage *msg) {
  printf("we've been canceled\n");
  OutstandingResponse *response = GetOutstandingResponse(msg->id());
  if (response) {
    response->controller->SetFailed(msg->buffer());
    InvokeCallbackAndCleanup(response);
  }
}


/*
 * Handle a NOT_IMPLEMENTED by invoking the callback.
 */
void StreamRpcChannel::HandleNotImplemented(RpcMessage *msg) {
  printf("not implemented\n");
  OutstandingResponse *response = GetOutstandingResponse(msg->id());
  if (response) {
    response->controller->SetFailed("Not Implemented");
    InvokeCallbackAndCleanup(response);
  }
}


/*
 * Find the outstanding response with id msg_id.
 */
OutstandingResponse *StreamRpcChannel::GetOutstandingResponse(int msg_id) {
  if (m_responses.find(msg_id) != m_responses.end()) {
    return m_responses[msg_id];
  }
  return NULL;
}


/*
 * Run the callback for a request.
 */
void StreamRpcChannel::InvokeCallbackAndCleanup(OutstandingResponse *response) {
  if (response) {
    int id = response->id;
    response->callback->Run();
    delete response;
    m_responses.erase(id);
  }
}


// StreamRpcHeader
//--------------------------------------------------------------

/**
 * Encode a header
 */
void StreamRpcHeader::EncodeHeader(uint32_t &header, unsigned int version, unsigned int size) {
  header = (version << 28) & VERSION_MASK;
  header |= size & SIZE_MASK;
}


/**
 * Decode a header
 */
void StreamRpcHeader::DecodeHeader(uint32_t header, unsigned int &version, unsigned int &size) {
  version = (header & VERSION_MASK) >> 28;
  size = header & SIZE_MASK;
}