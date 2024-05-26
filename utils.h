#pragma once

#include <stdexcept>
#include <vector>

#include "types.h"

class game_end : std::runtime_error {
public:
    using runtime_error::runtime_error;
    using runtime_error::what;
};

inline void move_tiles(std::vector<Tile> &dst, std::vector<Tile> &src, std::size_t n) {
    if (src.size() < n) throw game_end{"game end"};

    for (std::size_t i = 0; i < n; ++i) {
        Tile src_back = src.back();
        src.pop_back();
        dst.push_back(src_back);
    }
}

inline auto closest_tile(std::vector<Tile> &tiles,
                         std::underlying_type_t<Tile> target) -> Tile * {
    auto it = std::ranges::lower_bound(tiles, target).base();
    if (it == begin(tiles).base()) {
        return it;
    }
    if (it == end(tiles).base()) {
        return std::prev(it);
    }
    const std::underlying_type_t<Tile> prevVal = *std::prev(it);
    const std::underlying_type_t<Tile> nextVal = *it;
    if (target - prevVal <= nextVal - target)
        return std::prev(it);
    return it;
}

inline auto pop_back_ret(std::vector<Tile> &tiles) -> Tile {
    if (tiles.empty()) throw game_end{"game end"};
    Tile back = tiles.back();
    tiles.pop_back();
    return back;
}
