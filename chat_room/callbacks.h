//
// Created by Boris Chen on 5/6/25.
//

#pragma once

struct HandlerBase { };

struct ReadListener {
    virtual size_t OnRead() = 0;
};
