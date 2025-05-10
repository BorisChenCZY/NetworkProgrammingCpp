//
// Created by Boris Chen on 5/6/25.
//

#pragma once

template <typename T>
concept LinkConcept = requires (T link)
{
    {link.Fd()} -> std::same_as<int>;
};