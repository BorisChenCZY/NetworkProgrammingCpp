//
// Created by Boris Chen on 4/13/25.
//

#include <netdb.h>
#include <sys/socket.h>
#include <cstdio>
#include <print>
#include <memory>
#include <unistd.h>

using namespace std::literals::string_view_literals;

constexpr const std::string_view DEST_ADDR = "localhost"sv;
constexpr const std::string_view DEST_PORT = "3080"sv;
constexpr const uint64_t MTU = 1460;

int prepareSocket(std::string_view dest_addr, std::string_view port) {
    addrinfo hint, *result;
    memset(&hint, 0, sizeof(addrinfo));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(dest_addr.data(), port.data(), &hint, &result) != 0) {
        perror("prepareSocket::getaddrinfo");
        return -1;
    }
    auto freeResult = [](addrinfo* result) {
        freeaddrinfo(result);
    };
    std::unique_ptr<addrinfo, decltype(freeResult)> guard(result, freeResult);

    for (auto p = result; p != nullptr; p = p->ai_next) {
        auto fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd == -1) {
            perror("prepareSocket::socket");
            continue;
        }

        // here we skip bind because we don't care which port to use
        // connect will help us find a usable port, which is what we really want
        if (connect(fd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("prepareSocket::connect");
            continue;
        }

        // early termination here
        return fd;
    }

    // default is failure
    // otherwise we have early termination from above
    return -1;
}

int main() {
    int clientFd = prepareSocket(DEST_ADDR, DEST_PORT);
    if (clientFd == -1) {
        return -1;
    }

    auto fdCloser = [](int *fd) {
        close(*fd);
    };
    std::unique_ptr<int, decltype(fdCloser)> guard(new int(clientFd), fdCloser);

    int size;
    char buffer[MTU];

    // now the socket is connected.
    // we want to receive
    // note that this is blocking mode
    while ((size = recv(clientFd, buffer, MTU, 0)) > 0) {
        std::print("{}", std::string(buffer, size));
    }
    // if recv returns 0, means its closed
    if (size == 0) {
        std::println("remote closed the connection.");
    }
    else {
        perror("main::recv");
    }

    return size;
}