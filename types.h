#pragma once

#include <cstdint>

constexpr auto hand_size_init = 13;
constexpr auto hand_size_max = 14;
constexpr auto wall_size = 10;
constexpr auto num_game_tiles = 136;
constexpr auto number_of_games = static_cast<int>(1e5);

constexpr std::uint8_t tile_suit_mask   = 0xF0;
constexpr std::uint8_t tile_mag_mask    = 0x0F;

enum Tile : std::uint8_t {
    None = 0,

    // Characters
    Character1 = 0x10,
    Character2,
    Character3,
    Character4,
    Character5,
    Character6,
    Character7,
    Character8,
    Character9,

    // Dots
    Dots1 = 0x20,
    Dots2,
    Dots3,
    Dots4,
    Dots5,
    Dots6,
    Dots7,
    Dots8,
    Dots9,

    // Bamboo
    Bamboo1 = 0x30,
    Bamboo2,
    Bamboo3,
    Bamboo4,
    Bamboo5,
    Bamboo6,
    Bamboo7,
    Bamboo8,
    Bamboo9,

    // Dragons
    DragonWhite = 0x70,
    DragonGreen = 0x80,
    DragonRed = 0x90,

    // Winds
    WindEast = 0xC0,
    WindSouth = 0xD0,
    WindWest = 0xE0,
    WindNorth = 0xF0,
};

inline auto operator++(Tile &t) -> Tile & {
    t = static_cast<Tile>(t + 1);
    return t;
}

inline auto operator++(Tile t, int) -> Tile {
    const auto copy = t;
    ++t;
    return copy;
}


static_assert(std::weakly_incrementable<Tile>);

std::array possible_tiles{
    Character1, Character2, Character3, Character4, Character5, Character6, Character7, Character8, Character9,
    Dots1, Dots2, Dots3, Dots4, Dots5, Dots6, Dots7, Dots8, Dots9,
    Bamboo1, Bamboo2, Bamboo3, Bamboo4, Bamboo5, Bamboo6, Bamboo7, Bamboo8, Bamboo9,
    DragonWhite, DragonGreen, DragonRed,
    WindEast, WindSouth, WindWest, WindNorth,
};

enum Players {
    Player1 = 0,
    Player2,
    Player3,
    Player4,

    N_Players
};

template<int HanNumber>
struct Han {
    constexpr static auto value = HanNumber;
};

struct Mangan {
};

struct Yakuman {
};

struct DoubleYakuman {
};

using Yaku = std::variant<Han<1>, Han<2>, Han<3>, Han<6>, Mangan, Yakuman, DoubleYakuman>;

inline auto is_same_suit(Tile t1, Tile t2) -> bool { return (t1 & tile_suit_mask) == (t2 & tile_suit_mask); }

inline auto is_bamboo(Tile t) -> bool { return (t & tile_suit_mask) == Bamboo1; }
inline auto is_character(Tile t) -> bool { return (t & tile_suit_mask) == Character1; }
inline auto is_dots(Tile t) -> bool { return (t & tile_suit_mask) == Dots1; }
inline auto is_dragon(Tile t) -> bool { return t >= DragonWhite && t <= DragonRed; }
inline auto is_wind(Tile t) -> bool { return t >= WindEast && t <= WindNorth; }
inline auto is_honor(Tile t) -> bool { return t >= DragonWhite && t <= WindNorth; }
inline auto is_normal(Tile t) -> bool { return !is_honor(t) && t != None; }

inline auto mag(Tile t) -> int {
    if (!is_normal(t)) return 0;
    return (t & tile_mag_mask) + 1;
}

inline auto is_terminal(Tile t) -> bool { int m = mag(t); return m == 1 || m == 9; }

