#pragma once

#include <set>
#include <range/v3/algorithm/contains.hpp>
#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/algorithm/any_of.hpp>
#include <range/v3/view/chunk_by.hpp>
#include <range/v3/view/filter.hpp>

#include "types.h"
#include "hand.h"

using namespace ranges;
using namespace ranges::views;

auto seat_wind                  (const Hand&, Tile) -> bool;
auto prevelant_wind             (const Hand&, Tile) -> bool;

auto all_simples                (const Hand&) -> bool;
auto dragons                    (const Hand&) -> bool;
auto pinfu                      (const Hand&) -> bool;
auto pure_double_seq            (const Hand&) -> bool;

std::array han_1{all_simples, dragons, pinfu,  pure_double_seq};

auto triple_trilpets            (const Hand&) -> bool;
auto three_concealed_triplets   (const Hand&) -> bool;
auto little_three_dragons       (const Hand&) -> bool;
auto all_terminals_and_honors   (const Hand&) -> bool;
auto seven_pairs                (const Hand&) -> bool;
auto pure_straight              (const Hand&) -> bool;
auto mixed_triple_sequence      (const Hand&) -> bool;

// No seven pairs because it relies on Hand::free_tiles not being empty
std::array han_2{triple_trilpets , three_concealed_triplets, little_three_dragons, all_terminals_and_honors, pure_straight, mixed_triple_sequence};

auto twice_pure_double_seq      (const Hand&) -> bool;
auto fully_outside_hand         (const Hand&) -> bool;
auto half_flush                 (const Hand&) -> bool;

std::array han_3{twice_pure_double_seq, fully_outside_hand, half_flush};

// Han 6
auto full_flush                 (const Hand&) -> bool;

auto big_three_dragons          (const Hand&) -> bool;
auto four_concealed_triplets    (const Hand&) -> bool;
auto all_honors                 (const Hand&) -> bool;
auto all_green                  (const Hand&) -> bool;
auto all_terminals              (const Hand&) -> bool;
auto thirteen_orphans           (const Hand&) -> bool;
auto four_little_winds          (const Hand&) -> bool;

// No thirteen_orphans because it relies on Hand::free_tiles not begin empty
std::array yakuman{big_three_dragons , four_concealed_triplets, all_honors, all_green, all_terminals, four_little_winds};

inline auto seat_wind(const Hand &hand, Tile seat_wind) -> bool {
    const bool b = contains(hand.triples, seat_wind);
    return b;
}

inline auto prevelant_wind(const Hand &hand, Tile prevelant_wind) -> bool {
    const bool b = contains(hand.triples, prevelant_wind);
    return b;
}

inline auto all_simples(const Hand &hand) -> bool {
    static const auto simple_triple = [](Tile t) {
        const auto m = mag(t);
        return 2 <= m && m <= 8;
    };
    static const auto simple_triplet = [](Tile t) {
        const auto m = mag(t);
        return 2 <= m && m <= 6;
    };
    const bool b = all_of(concat(hand.triples, hand.free_tiles, single(hand.pair)), simple_triple)
                && all_of(hand.triplets, simple_triplet);
    return b;
}

inline auto dragons(const Hand &hand) -> bool {
    const bool b = any_of(hand.triples, is_dragon);
    return b;
}

inline auto pinfu(const Hand &hand) -> bool {
    const bool b = hand.triplets.size() == 4 && is_normal(hand.pair);
    return b;
}

inline auto pure_double_seq(const Hand &hand) -> bool {
    const bool b = any_of(hand.triplets | chunk_by(std::equal_to{}), [](auto&& r) { return size(r) > 1; });
    return b;
}

inline auto triple_trilpets(const Hand& hand) -> bool {
    auto normal_triples = hand.triples | filter(is_normal) | transform(mag) | to_vector;
    std::ranges::sort(normal_triples);
    const bool b = any_of(normal_triples | chunk_by(std::equal_to{}), [](auto&& r){ return size(r) == 3; });
    return b;
}

inline auto three_concealed_triplets(const Hand& hand) -> bool {
    const bool b = hand.triples.size() >= 3;
    return b;
}

inline auto little_three_dragons(const Hand& hand) -> bool {
    const bool b = is_dragon(hand.pair) && size(hand.triples | filter(is_dragon) | to_vector) == 2;
    return b;
}

inline auto all_terminals_and_honors(const Hand& hand) -> bool {
    static const auto terminal_or_honor = [](Tile t){ return is_terminal(t) || is_honor(t); };
    const bool b = all_of(concat(hand.triples, hand.free_tiles, single(hand.pair)), terminal_or_honor) && hand.triplets.empty();
    return b;
}

inline auto seven_pairs(const Hand& hand) -> bool {
    const bool b = hand.triples.empty() && hand.triplets.empty() && hand.pair != None
        && all_of(hand.free_tiles | chunk_by(std::equal_to{}), [](auto&& r){ return size(r) == 2; });
    return b;
}

inline auto pure_straight(const Hand& hand) -> bool {
    const bool b = any_of(hand.triplets | chunk_by(is_same_suit), [](auto&& r) {
        return equal(r | transform(mag), std::array{1, 4, 7});
    });
    return b;
}

inline auto mixed_triple_sequence(const Hand& hand) -> bool {
    auto mags = hand.triplets | transform(mag) | to_vector;
    std::ranges::sort(mags);
    const bool b = any_of(mags | chunk_by(std::equal_to{}), [](auto&& r){ return size(r) >= 3; });
    return b;
}

auto twice_pure_double_seq(const Hand& hand) -> bool {
    const bool b = distance(hand.triplets | chunk_by(std::equal_to{}) | filter([](auto&& r){ return size(r) >= 2; })) == 2;
    return b;
}

auto fully_outside_hand(const Hand& hand) -> bool {
    static const auto terminal_triplet = [](Tile t) {
        const auto m = mag(t);
        return m == 1 || m == 7;
    };
    const bool b = all_of(concat(hand.triples, single(hand.pair), hand.free_tiles), is_terminal) && all_of(hand.triplets, terminal_triplet);
    return b;
}

auto half_flush(const Hand& hand) -> bool {
    const bool b = distance(hand.all_tiles() | filter(is_normal) | chunk_by(is_same_suit)) == 1;
    return b;
}

auto full_flush(const Hand& hand) -> bool {
    const bool b = all_of(hand.all_tiles() | std::views::pairwise_transform(is_same_suit), std::identity{});
    return b;
}

auto big_three_dragons(const Hand& hand) -> bool {
    const bool b = distance(hand.triples | filter(is_dragon)) == 3;
    return b;
}

auto four_concealed_triplets(const Hand& hand) -> bool {
    const bool b = hand.triples.size() == 4;
    return b;
}

auto all_honors(const Hand& hand) -> bool {
    const bool b = all_of(hand.all_tiles(), is_honor);
    return b;
}

auto all_green(const Hand& hand) -> bool {
    static const auto is_green = [](Tile t) {
        return is_bamboo(t) || t == DragonGreen;
    };
    const bool b = all_of(hand.all_tiles(), is_green);
    return b;
}

auto all_terminals(const Hand& hand) -> bool {
    const bool b = all_of(hand.all_tiles(), is_terminal);
    return b;
}

auto thirteen_orphans(const Hand& hand) -> bool {
    const bool b = all_of(hand.all_tiles(), [](Tile t){ return is_honor(t) || is_terminal(t); })
            && distance(hand.all_tiles() | chunk_by(std::equal_to{})) == 13;
    return b;
}

auto four_little_winds(const Hand& hand) -> bool {
    const bool b = is_wind(hand.pair) && distance(hand.triples | filter(is_wind)) == 3;
    return b;
}