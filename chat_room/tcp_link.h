//
// Created by Boris Chen on 5/7/25.
//

#pragma once
#include <unistd.h>
#include "types.h"
#include "array"

class TCPLink {
public:
    TCPLink(int fd): m_fd(fd) {}

    ErrorTypeT Close();

    FileDescriptorT Fd() const;

    std::expected<size_t, ErrorTypeT> Read(std::span<uint8_t> buffer);

    ~TCPLink() {
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

inline FileDescriptorT TCPLink::Fd() const {
    return m_fd;
}

inline std::expected<size_t, ErrorTypeT> TCPLink::Read(std::span<uint8_t> buffer) {
    if (ssize_t size; (size = ::read(m_fd, reinterpret_cast<void*>(buffer.data()), buffer.size())) != 0) {
        return std::unexpected<ErrorTypeT>(-errno);
    }
    else {
        return size;
    }
}

