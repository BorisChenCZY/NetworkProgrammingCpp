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
#include <print>
#include <ranges>
#include <_stdio.h>
#include <vector>


#include "reactor_concept.h"
// reactor calls the select/wait
// if it has messages, it calls to handler
class SelectReactor {
public:
    SelectReactor() {
        FD_ZERO(&m_readFds);
        m_readCount = 0;
    }

    void Stop() {
        m_running = false;
    }

    template <typename T>
    void AddHandler(T* handler) {
        if constexpr (std::is_base_of<ReadListener, T>::value) {
            m_readListeners.emplace_back(handler);
            FD_SET(handler->fd(), &m_readFds);
            m_readCount++;
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

            // else process
            Process();
        }
    }

protected:
    std::expected<int, int> Wait() {
        // here we call select
        m_readReadyFds = m_readFds;
        if (int error; ( error = select(m_readCount + 1, &m_readReadyFds, 0, 0, 0)) == -1) {
            // something is wrong
            return std::unexpected<int>(errno);
        }

        return 0;
    }

    std::expected<int, int> Process() {
        auto readFds = std::ranges::views::iota(0u, m_readCount) | std::ranges::views::filter([&](int i) {
            // TODO this is not efficient
            return FD_ISSET(i, &m_readReadyFds);
        });

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
    std::vector<std::unique_ptr<ReadListener>> m_readListeners{};
    fd_set m_readFds;
    fd_set m_readReadyFds;
    uint16_t m_readCount{0};
    bool m_running{true};
};
static_assert(ReactorConcept<SelectReactor>);
