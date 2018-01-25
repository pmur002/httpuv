#include "socket.h"
#include "httprequest.h"
#include <later_api.h>
#include <uv.h>

void on_Socket_close(uv_handle_t* pHandle);

void Socket::addConnection(boost::shared_ptr<HttpRequest> request) {
  connections.push_back(request);
}

void Socket::removeConnection(boost::shared_ptr<HttpRequest> request) {
  connections.erase(
    std::remove(connections.begin(), connections.end(), request),
    connections.end());
}

Socket::~Socket() {
  ASSERT_BACKGROUND_THREAD()
  trace("Socket::~Socket");
}

// This tells all the HttpRequests to close and deletes the
// shared_ptr<Socket>. Each HttpRequest also has a shared_ptr to this Socket.
// Once they're all closed, the Socket will be deleted. In some cases, there
// may be an extant HttpRequest object which doesn't get deleted until an R GC
// event occurs, and so this Socket will continue to exist until then.
void Socket::close() {
  ASSERT_BACKGROUND_THREAD()
  trace("Socket::close");
  for (std::vector<boost::shared_ptr<HttpRequest>>::reverse_iterator it = connections.rbegin();
    it != connections.rend();
    it++) {

    // std::cerr << "Request close on " << *it << std::endl;
    (*it)->close();
  }

  uv_handle_t* pHandle = toHandle(&handle.stream);
  uv_close(pHandle, NULL);
}
