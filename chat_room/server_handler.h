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
    ServerHandler(std::unique_ptr<LinkT>&& link, ReactorT* reactor, ProcessorT& processor):  m_processor(processor) {
        m_link.swap(link);
        reactor->AddHandler(this);
    }

    size_t OnRead() override {
        auto result = m_link->Accept();
        if (result.has_value()) {
            FileDescriptorT clientFd = result.value();
            return m_processor.AddClient(clientFd);
        }
        return result.error();
    }

    int Fd() const {
        return m_link->Fd();
    }

private:
    std::unique_ptr<LinkT> m_link;
    ProcessorT& m_processor;
};

template <LinkConcept LinkT, ProcessorConcept ProcessorT>
class ClientHandler: public ReadListener, HandlerBase {
public:
    ClientHandler(std::unique_ptr<LinkT>&& link, ProcessorT& processor):  m_processor(processor) {
        m_link.swap(link);
    }

    size_t OnRead() override {
        // auto data = m_link->
        auto result = m_link->Read(m_buffer);
        if (not result.has_value())
        {
            // will get this handler removed
            return result.error();
        }
        auto size = result.value();

        // process
        return m_processor.Process(std::span<uint8_t>(m_buffer.data(), size));
    }

    FileDescriptorT Fd() const {
        return m_link->Fd();
    }

private:
    std::unique_ptr<LinkT> m_link;
    ProcessorT& m_processor;
    std::array<uint8_t, 1500 /*MTU*/> m_buffer;
};