#pragma once

#include <type_traits>

#include "types.h"

struct MoveSource {
    virtual auto select_tile() -> std::underlying_type_t<Tile> = 0;
    virtual ~MoveSource() = default;
};
