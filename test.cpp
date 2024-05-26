#include <boost/ut.hpp>

#include "predicated.h"

using namespace boost::ut;

#include "hand.h"
#include "display.h"

#define MAIN int main() {
#define MAIN_END }

auto operator<<(std::ostream &os, std::vector<std::vector<Tile> > ts) -> std::ostream & {
    for (auto &t: ts) {
        os << t << ',';
    }
    return os;
}

MAIN

[] {
    expect(eq(
        chunk_by_at_most_three(std::vector{Character1, Character2, Character2, Character9}, std::equal_to{}),
        std::vector<std::vector<Tile> >{{Character1}, {Character2, Character2}, {Character9}}
    ));
}();

[] {
    Hand hand;
    hand.free_tiles = {Character1, Character1, Character2, Character2, Character3, Character4};
    process_triplets(hand);
    expect(eq(
        hand,
        Hand{
            .triplets = {Character1},
            .triples = {},
            .free_tiles = {Character1, Character2, Character4},
        }
    ));
}();

[] {
    Hand hand;
    hand.free_tiles = {Character1, Character2, Character2, Character2, Character3, Character5, Character5};
    process_triples(hand);
    expect(eq(
        hand,
        Hand{
            .triplets = {},
            .triples = {Character2},
            .free_tiles = {Character1, Character3, Character5, Character5},
        }
    ));
}();

[] {
    Hand hand;
    hand.free_tiles = {Character1, Character2, Character2, Character3, Character3};
    process_pair(hand);
    expect(eq(
        hand,
        Hand{
            .triplets = {},
            .triples = {},
            .free_tiles = {Character1, Character3, Character3},
            .pair = Character2
        }
    ));
}();

[] {
    expect(eq(
        process_hand({Character1, Character2, Character3,
            Dots1, Dots1,
            Dots3, Dots4, Dots5,
            Bamboo5, Bamboo6, Bamboo7,
            WindNorth, WindNorth, WindNorth
        }),
        Hand{
            .triplets = { Character1, Dots3, Bamboo5 },
            .triples = { WindNorth },
            .pair = Dots1
        }
    ));
}();

[] {
    expect(pure_double_seq(process_hand({
        {
            Character1, Character2, Character3,
            Character1, Character2, Character3,
            WindNorth, WindNorth, WindNorth,
            WindEast, WindEast, WindEast,
            WindSouth, WindSouth
        }
    })));
}();

[] {
    expect(triple_trilpets(process_hand({
        {
            Character1, Character1, Character1,
            Dots1, Dots1, Dots1,
            WindEast, WindEast, WindEast,
            Bamboo1, Bamboo1, Bamboo1,
            WindSouth, WindSouth
        }
    })));
}();

[] {
    expect(twice_pure_double_seq(process_hand({
        {
            Character1, Character2, Character3,
            Character1, Character2, Character3,
            Bamboo2, Bamboo3, Bamboo4,
            Bamboo2, Bamboo3, Bamboo4,
            WindSouth, WindSouth
        }
    })));
}();

[] {
    expect(fully_outside_hand(process_hand({
        {
            Character1, Character2, Character3,
            Character7, Character8, Character9,
            Bamboo1, Bamboo1, Bamboo1,
            Bamboo7, Bamboo8, Bamboo9,
            Dots9, Dots9
        }
    })));
}();

[] {
    expect(thirteen_orphans(process_hand({
        {
            Character1, Character1, Character9,
            Dots1, Dots9,
            Bamboo1, Bamboo9,
            WindEast, WindSouth, WindWest, WindNorth,
            DragonWhite, DragonRed, DragonGreen
        }
    })));
}();

MAIN_END
