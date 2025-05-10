//
// Created by Boris Chen on 5/5/25.
//

#pragma once


#include "server_handler.h"
#include "tcp_server_link.h"
#include "reactor_concept.h"
#include "tcp_link.h"

template <ReactorConcept ReactorT>
class Server {
public:
    Server(ReactorT & reactor):  m_reactor(reactor) {}

    int AddClient(int clientFd) {
        // this needs to create a clientHandler
        // create client channel and proecssor
        // add to reactor
        // TCPLink link(clientFd);
        std::cout << "Adding client: " << clientFd << std::endl;
        auto clientHandler = new ClientHandler<TCPLink, Server>(std::make_unique<TCPLink>(clientFd),
            *this
            );
        m_reactor.AddHandler(clientHandler); // ownership transfer to reactor

        return 0;
    }

    size_t Process(std::span<const uint8_t> buffer) {
        std::print("Message: {}", std::string((char*)buffer.data(), buffer.size()));
        return buffer.size();
    }

private:
    ReactorT& m_reactor;
};