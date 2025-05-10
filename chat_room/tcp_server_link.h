//
// Created by Boris Chen on 5/6/25.
//

#pragma once
#include <cerrno>
#include <expected>
#include <iostream>
#include <netdb.h>
#include <unistd.h>
#include <sys/fcntl.h>

#include "types.h"


// we only support local address for now
class TCPServerLink {
public:
    TCPServerLink(): m_fd(-1) {}
    TCPServerLink(bool reuse, bool blocking): m_fd(-1), m_reuse(reuse), m_block(blocking) {}

    ErrorTypeT Init(const char* port);
    std::expected<FileDescriptorT, ErrorTypeT> Accept();
    ErrorTypeT Close();

    FileDescriptorT Fd() const;

    ~TCPServerLink() {
        if (m_fd != -1) {
            ::close(m_fd);
        }
    }

private:
    bool is_inited() {
        return m_fd != -1;
    }

    int m_fd{-1};
    bool m_reuse{false};
    bool m_block{false};
};

inline FileDescriptorT TCPServerLink::Fd() const {
    std::cout << "fd: " << m_fd << std::endl;
    return m_fd;
}

inline ErrorTypeT TCPServerLink::Init(const char* port) {
    struct addrinfo hints;
    struct addrinfo *servinfo;
    auto addrDeleter = [](addrinfo *info) {
        freeaddrinfo(info);
    };

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if (getaddrinfo(NULL, port, &hints, &servinfo) != 0) {
        return errno;
    }
    // now we guard it
    std::unique_ptr<addrinfo, decltype(addrDeleter)> guard(servinfo, addrDeleter);

    for (auto p = servinfo; p != nullptr; p = p->ai_next) {
        m_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (m_fd == -1) {
            continue;
        }

        // set socket
        if (m_reuse) {
            int val = 1;
            if (setsockopt(m_fd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)) == -1) {
                continue;
            }
        }

        if (bind(m_fd, p->ai_addr, p->ai_addrlen) != 0) {
            continue;
        }

        if (not m_block) {
            int flags = fcntl(m_fd, F_GETFL, 0);
            fcntl(m_fd, F_SETFL, flags | O_NONBLOCK);
        }

        ::listen(m_fd, 1024);

        return 0;
    }

    return errno; // something is wrong
}

inline std::expected<FileDescriptorT, ErrorTypeT> TCPServerLink::Accept() {
    sockaddr_storage clientSockAddr;
    socklen_t len = sizeof(sockaddr_storage);
    auto clientFd = ::accept(m_fd, reinterpret_cast<sockaddr*>(&clientSockAddr), &len);
    if (clientFd == -1) {
        return std::unexpected<ErrorTypeT>(errno);
    }

    return clientFd;
}
