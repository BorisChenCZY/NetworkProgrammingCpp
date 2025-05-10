//
// Created by Boris Chen on 5/6/25.
//

#pragma once

#include <expected>
#include <memory>
#include <netdb.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/select.h>
#include <fcntl.h>
#include <iostream>
#include <print>
#include <ranges>
#include <unordered_map>
#include <_stdio.h>
#include <vector>


#include "reactor_concept.h"
// reactor calls the select/wait
// if it has messages, it calls to handler
class SelectReactor {
public:
    SelectReactor() {
        FD_ZERO(&m_readFds);
        FD_ZERO(&m_readReadyFds);
    }

    void Stop() {
        m_running = false;
    }

    template <typename T>
    void AddHandler(T* handler) {
        if constexpr (std::is_base_of<ReadListener, T>::value) {
            auto fd = handler->Fd();
            m_readListeners[fd] = std::unique_ptr<ReadListener>(static_cast<ReadListener*>(handler));
            FD_SET(fd, &m_readFds);
            std::cout << "fd set: " << fd << std::endl;
            m_maxFd = std::max(m_maxFd, fd);
        }

        // TODO can add WriteListener and ExceptionListener here
    }

    void Run() {
        // thread looping here
        while (m_running) {
            auto result = Wait();
            if (not result.has_value()) {
                m_running = false;
                std::print(stderr, "Errno: {}", result.error());
                return;
            }

            std::cout << "processing..." << std::endl;

            // else process
            Process();
        }
    }

protected:
    std::expected<int, int> Wait() {
        // here we call select
        m_readReadyFds = m_readFds;
        std::cout << "waiting..." << std::endl;
        if (int error; ( error = select(m_maxFd + 1, &m_readReadyFds, nullptr, nullptr, nullptr)) == -1) {
            // something is wrong
            std::cout <<"failed..." << std::endl;
            return std::unexpected<int>(errno);
        }
        std::cout << "waited" << std::endl;

        return 0;
    }

    std::expected<int, int> Process() {
        auto readFds = std::ranges::views::iota(0, m_maxFd) | std::ranges::views::filter([&](int i) {
            // TODO this is not efficient
            return FD_ISSET(i, &m_readReadyFds);
        });

        // this has to be a map
        std::ranges::for_each(readFds, [&](int fdIdx) {
            int ret = m_readListeners[fdIdx]->OnRead();
            if (ret < 0) {
                // this needs to be removed
                // we would not expect a lot here
                FD_CLR(fdIdx, &m_readFds);
                m_readListeners[fdIdx].reset(nullptr);
            }
        });

        return 0;
    }

private:
    std::unordered_map<FileDescriptorT, std::unique_ptr<ReadListener>> m_readListeners{};
    fd_set m_readFds;
    fd_set m_readReadyFds;
    FileDescriptorT m_maxFd;
    bool m_running{true};
};
static_assert(ReactorConcept<SelectReactor>);
