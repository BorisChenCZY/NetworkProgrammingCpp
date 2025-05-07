//
// Created by Boris Chen on 5/5/25.
//

#pragma once


#include "server_handler.h"
#include "tcp_server_link.h"
#include "reactor_concept.h"
#include "tcp_link.h"

template <ReactorConcept ReactorT, typename ClientT>
class Server {
public:
    int AddClient(int clientFd) {
        // this needs to create a clientHandler
        // create client channel and proecssor
        // add to reactor
        // TCPLink link(clientFd);
        ClientT client;
        auto clientHandler = new ClientHandler<ReactorT, TCPLink, ClientT>(std::make_unique<TCPLink>(clientFd),
            m_reactor,
            std::move(client)
            );
        m_reactor.AddHandler(clientHandler); // ownership transfer to reactor

        return 0;
    }

private:
    ReactorT m_reactor;
    ServerHandler<ReactorT, TCPServerLink, Server> m_handler;
    // std::unordered_set<
};

template <ReactorConcept ReactorT>
class Client {

};
