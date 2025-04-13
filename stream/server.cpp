//
// Created by Boris Chen on 4/12/25.
//

#include <cstdio>
#include <cstring>
#include <sys/socket.h>
#include <netdb.h>
#include <functional>
#include <memory>
#include <print>
#include <thread>
#include <string_view>
#include <unistd.h>
#include <csignal>

using namespace std::literals::string_view_literals;

constexpr const char* PORT = "3080";
constexpr const int BACKLOG = 1024;

// Global flag for graceful shutdown
std::atomic<bool> g_should_exit{false};

void handleSignal(int sig) {
    std::println("Received signal {}, shutting down..", sig);
    g_should_exit = true;
}


int createSocket(const char* port) {
    struct addrinfo hints;
    struct addrinfo *servinfo;
    // guard it
    auto addrDeleter = [](addrinfo* info) {
        freeaddrinfo(info);
    };

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port, &hints, &servinfo) != 0) {
        perror("createSocket::getaddrinfo");
        return -1;
    }
    std::unique_ptr<addrinfo, decltype(addrDeleter)> guard(servinfo, addrDeleter);

    for (auto p = servinfo; p != nullptr; p = p->ai_next) {
        auto fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (fd == -1) {
            perror("createSocket::socket");
            continue;
        }

        // set socket
        int val = 1;
        if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1) {
            perror("createSocket::setsockopt");
            continue;
        }

        // bind
        if (bind(fd, p->ai_addr, p->ai_addrlen) != 0) {
            perror("createSocket::bind");
            continue;
        }

        return fd;
    }

    return -1;
}

void processClientFd(int clientFd) {
    // fork?
    // but we'll go with thread
    // TODO can we go with goroutine
    std::jthread client_thread([clientFd] {
        auto closeFd = [](int *fd) { close(*fd); };
        std::unique_ptr<int, decltype(closeFd)> fd_guard(new int(clientFd), closeFd);

        std::string_view msg = "Hello world.\n"sv;
        if (send(clientFd, reinterpret_cast<const void*>(msg.data()), msg.size(), 0) != msg.size()) {
            // we do not expect to exceed MTU here.
            perror("[client_thread] processClientFd::send");
        }

        if (shutdown(clientFd, SHUT_RDWR) == -1) {
            perror("[client_thread] processClientFd::shutdown");
        }
    });
}

void run(int serviceSockFd) {
    auto closeFd = [](int *fd) {
        return close(*fd);
    };
    std::unique_ptr<int, decltype(closeFd)> guard(&serviceSockFd, closeFd);

    while (!g_should_exit) {
        sockaddr_storage clientSockAddr;
        socklen_t len = sizeof(sockaddr_storage);
        auto clientFd = accept(serviceSockFd, reinterpret_cast<sockaddr*>(&clientSockAddr), &len);
        if (clientFd == -1) {
            perror("run::accept");
            continue;
        }

        processClientFd(clientFd);
    }
}

int main() {
    // Set up signal handling
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    auto serviceSockFd = createSocket(PORT);
    if (serviceSockFd == -1) return -1;

    if (listen(serviceSockFd, BACKLOG) != 0) {
        perror("main::listen");
        return -1;
    }

    run(serviceSockFd);

    return 0;
}