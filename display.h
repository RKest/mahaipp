#pragma once

#include "types.h"

inline auto tile_to_char32(Tile tile) -> char32_t {
    switch (tile) {
        case None:
            return U' ';
        case Character1:
            return U'🀇';
        case Character2:
            return U'🀈';
        case Character3:
            return U'🀉';
        case Character4:
            return U'🀊';
        case Character5:
            return U'🀋';
        case Character6:
            return U'🀌';
        case Character7:
            return U'🀍';
        case Character8:
            return U'🀎';
        case Character9:
            return U'🀏';
        case Dots1:
            return U'🀙';
        case Dots2:
            return U'🀚';
        case Dots3:
            return U'🀛';
        case Dots4:
            return U'🀜';
        case Dots5:
            return U'🀝';
        case Dots6:
            return U'🀞';
        case Dots7:
            return U'🀟';
        case Dots8:
            return U'🀠';
        case Dots9:
            return U'🀡';
        case Bamboo1:
            return U'🀐';
        case Bamboo2:
            return U'🀑';
        case Bamboo3:
            return U'🀒';
        case Bamboo4:
            return U'🀓';
        case Bamboo5:
            return U'🀔';
        case Bamboo6:
            return U'🀕';
        case Bamboo7:
            return U'🀖';
        case Bamboo8:
            return U'🀗';
        case Bamboo9:
            return U'🀘';
        case DragonWhite:
            return U'🀆';
        case DragonGreen:
            return U'🀅';
        case DragonRed:
            return U'🀄';
        case WindEast:
            return U'🀀';
        case WindSouth:
            return U'🀁';
        case WindWest:
            return U'🀂';
        case WindNorth:
            return U'🀃';
    }
    return ' ';
}

inline auto char_utf32_to_utf8(char32_t utf32, const char *buffer) -> char * {
    char *end = const_cast<char *>(buffer);
    if (utf32 < 0x7F) *(end++) = static_cast<unsigned>(utf32);
    else if (utf32 < 0x7FF) {
        *(end++) = 0b1100'0000 + static_cast<unsigned>(utf32 >> 6);
        *(end++) = 0b1000'0000 + static_cast<unsigned>(utf32 & 0b0011'1111);
    } else if (utf32 < 0x10000) {
        *(end++) = 0b1110'0000 + static_cast<unsigned>(utf32 >> 12);
        *(end++) = 0b1000'0000 + static_cast<unsigned>((utf32 >> 6) & 0b0011'1111);
        *(end++) = 0b1000'0000 + static_cast<unsigned>(utf32 & 0b0011'1111);
    } else if (utf32 < 0x110000) {
        *(end++) = 0b1111'0000 + static_cast<unsigned>(utf32 >> 18);
        *(end++) = 0b1000'0000 + static_cast<unsigned>((utf32 >> 12) & 0b0011'1111);
        *(end++) = 0b1000'0000 + static_cast<unsigned>((utf32 >> 6) & 0b0011'1111);
        *(end++) = 0b1000'0000 + static_cast<unsigned>(utf32 & 0b0011'1111);
    } else
        assert(false && "encoding error");
    *end = '\0';
    return end;
}

inline auto operator<<(std::ostream &os, const char32_t s) -> std::ostream & {
    char buffer[5]{0}; // That's the famous "big-enough buffer"
    char_utf32_to_utf8(s, buffer);
    os << buffer;
    return os;
}

inline auto operator<<(std::ostream& os, Tile t) -> std::ostream& {
    return os << tile_to_char32(t);
}

inline auto operator<<(std::ostream& os, std::vector<Tile> tiles) -> std::ostream& {
    std::ranges::for_each(tiles, [&os](Tile tile){ os << tile << ' '; });
    return os;
}
