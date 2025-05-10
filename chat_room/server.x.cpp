//
// Created by Boris Chen on 5/5/25.
//

#include "server.h"
#include "select_reactor.h"

int main(int argc, char *argv[]) {

    SelectReactor reactor;
    Server<SelectReactor> server(reactor);
    auto link = std::make_unique<TCPServerLink>();
    link->Init("6550");

    ServerHandler<SelectReactor, TCPServerLink, Server<SelectReactor>> serverHandler(std::move(link), &reactor, server);

    reactor.Run();
    return 0;
}
