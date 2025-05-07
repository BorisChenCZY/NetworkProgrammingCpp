//
// Created by Boris Chen on 5/6/25.
//

#pragma once

#include "callbacks.h"

template <typename T>
concept ReactorConcept = requires (T reactor, HandlerBase* handler)
{
    // TODO boris ?
    // {reactor.AddHandler(HandlerBase* handler)} -> void;
    {reactor.Run()} -> std::same_as<void>;
};