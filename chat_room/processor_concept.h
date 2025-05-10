//
// Created by Boris Chen on 5/6/25.
//

#pragma once
#include <cstdint>
#include <span>

#include "tcp_server_link.h"

// this is zero copy
template <typename T>
concept ProcessorConcept = requires (T processor, std::span<const uint8_t> data)
{
  {processor.Process(data)}  -> std::same_as<size_t>;
};

template <typename T>
concept ServerProcessorConcept = requires (T processor, FileDescriptorT fd)
{
  {processor.AddClient(fd)} -> std::same_as<int>;
};