//
// Created by Boris Chen on 5/6/25.
//

#pragma once

#include <memory>
#include "link_concept.h"
#include "processor_concept.h"
#include "reactor_concept.h"
#include <format>
#include <print>

// handler holds the link
// represents
template <ReactorConcept ReactorT, LinkConcept LinkT, ServerProcessorConcept ProcessorT>
class ServerHandler: public ReadListener, HandlerBase {
public:
    ServerHandler(std::unique_ptr<LinkT> link, ReactorT* reactor, ProcessorT&& processor):  m_processor(std::move(processor)) {
        m_link.reset(link);
        reactor->AddHandler(this);
    }

    int OnRead() override {
        auto result = m_link->accept();
        if (result.has_value()) {
            FileDescriptorT clientFd = result.value();
            m_processor.AddClient(clientFd);
        }
    }

    int Fd() const {
        return m_link->fd();
    }

private:
    std::unique_ptr<LinkT> m_link;
    ProcessorT m_processor;
};

template <ReactorConcept ReactorT, LinkConcept LinkT, ProcessorConcept ProcessorT>
class ClientHandler: public ReadListener, HandlerBase {
    ClientHandler(std::unique_ptr<LinkT> link, ReactorT* reactor, ProcessorT&& processor):  m_processor(std::move(processor)) {
        m_link.reset(link);
    }

    int OnRead() override {
        // auto data = m_link->
        if (int error; (error = m_link->Read(m_buffer) < 0))
        {
            // will get this handler removed
            return error;
        }
        else if (error != 0) {
            std::print(stderr, "error not expected: {}", error);
            throw std::exception();
        }

        // process
        m_processor.Process(m_buffer);
    }

private:
    std::unique_ptr<LinkT> m_link;
    ProcessorT m_processor;
    std::array<uint8_t, 1500 /*MTU*/> m_buffer;
};