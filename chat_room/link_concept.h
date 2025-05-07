//
// Created by Boris Chen on 5/6/25.
//

#pragma once

template <typename T>
concept LinkConcept = requires (T link)
{
    {link.fd()} -> std::convertible_to<int>;
};