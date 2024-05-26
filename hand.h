#pragma once

#include <algorithm>
#include <span>
#include <ranges>

#include <range/v3/view/chunk_by.hpp>
#include <range/v3/to_container.hpp>
#include <range/v3/view/drop.hpp>
#include <range/v3/view/concat.hpp>
#include <range/v3/view/single.hpp>
#include <range/v3/algorithm/all_of.hpp>
#include <range/v3/action/push_back.hpp>
#include <range/v3/view/transform.hpp>
#include <range/v3/algorithm/equal.hpp>
#include <range/v3/view/join.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/iota.hpp>
#include <range/v3/view/take.hpp>
#include <range/v3/algorithm/find.hpp>

#include "types.h"
#include "display.h"

using namespace ranges;
using namespace ranges::view;
using std::ranges::is_sorted;
using std::views::adjacent;

/*
 * Triple = Three of a kind
 * Triplet = Sequence of three ascending tiles
 */

struct Hand {
    std::vector<Tile> triplets;
    std::vector<Tile> triples;
    std::vector<Tile> free_tiles;
    Tile pair = None;

    auto operator==(const Hand &other) const -> bool {
        return equal(triples, other.triples)
               && equal(triplets, other.triplets)
               && equal(free_tiles, other.free_tiles)
               && pair == other.pair;
    }

    friend auto operator<<(std::ostream &os, const Hand &h) -> std::ostream & {
        return os << "Runs: " << h.triplets << ", 3x: " << h.triples << ", Free: " << h.free_tiles << ", 2x: " << h.pair;
    }

    inline auto non_free_tiles() const {
        return concat(triplets, triples, single(pair));
    }

    inline auto all_tiles() const {
        return concat(triplets, triples, single(pair), free_tiles);
    }
};

template<typename F>
concept TileCompare = requires(F f, Tile t)
{
    { f(t, t) } -> std::same_as<bool>;
};

auto chunk_by_at_most_three(std::span<const Tile> tiles, TileCompare auto pred) -> std::vector<std::vector<Tile> > {
    if (tiles.empty()) {
        return {};
    }
    assert(!tiles.empty());
    std::vector<std::vector<Tile> > result;
    std::vector<Tile> *next_group = &result.emplace_back();
    for (auto [prev, curr]: concat(tiles, single(Tile::None)) | std::views::pairwise) {
        next_group->push_back(prev);
        if (!pred(prev, curr) && curr != None) {
            next_group = &result.emplace_back();
        }
    }
    return result;
}

template<std::size_t Target>
constexpr auto is_sized = [](const auto &r) -> bool { return size(r) == Target; };
template<std::size_t Target>
constexpr auto is_not_sized = [](const auto &r) -> bool { return !is_sized<Target>(r); };

inline void process_triples(Hand &hand) {
    constexpr auto front = [](auto &r) -> auto { return *begin(r); };
    assert(hand.triples.empty());
    auto triples = chunk_by_at_most_three(hand.free_tiles, std::equal_to{});
    assert(all_of(triples, [](auto& t){ return all_of(t, std::bind_front(std::equal_to{}, *begin(t))); }));
    hand.triples |= push_back(triples | filter(is_sized<3>) | transform(front));
    hand.free_tiles.clear();
    hand.free_tiles |= push_back(triples | filter(is_not_sized<3>) | join);
}

inline void process_triplets(Hand &hand) {
    auto &tiles = hand.free_tiles;
    if (tiles.empty()) return;
    assert(is_sorted(tiles));

    std::remove_reference_t<decltype(tiles)>::iterator first, second, third;
    for (first = begin(tiles); first < end(tiles) - 2;) {
        if (!is_normal(*first)) break;
        second = next(first);
        while (second != end(tiles) - 1 && *first == *second) ++second;
        if (*second - *first != 1) { ++first; continue; };
        third = next(second);
        while (third != end(tiles) && *third == *second) ++third;
        if (*third - *second != 1) { ++first; continue; }

        hand.triplets.push_back(*first);
        tiles.erase(third);
        tiles.erase(second);
        tiles.erase(first);
        first = begin(tiles);
    }
}

inline void process_pair(Hand &hand) {
    auto &tiles = hand.free_tiles;
    auto it = begin(tiles);
    for (const auto &[first, second]: hand.free_tiles | std::views::pairwise) {
        if (first == second) {
            hand.pair = first;
            tiles.erase(it + distance(&*it, &second));
            tiles.erase(it + distance(&*it, &first));
            break;
        }
    }
}

inline auto process_triples_first(std::span<const Tile> tiles) -> Hand {
    assert(!tiles.empty());
    Hand hand;
    hand.free_tiles |= push_back(tiles);
    process_triples(hand);
    process_triplets(hand);
    process_pair(hand);
    return hand;
}

inline auto process_triplets_first(std::span<const Tile> tiles) -> Hand {
    assert(!tiles.empty());
    Hand hand;
    hand.free_tiles |= push_back(tiles);
    process_triplets(hand);
    process_triples(hand);
    process_pair(hand);
    return hand;
}

inline auto process_group(std::span<const Tile> tiles) -> Hand {
    const auto triples_first_hand = process_triples_first(tiles);
    const auto triplets_first_hand = process_triplets_first(tiles);
    return triples_first_hand.free_tiles.size() > triplets_first_hand.free_tiles.size()
               ? triplets_first_hand
               : triples_first_hand;
}

inline auto process_hand(std::vector<Tile> tiles) -> Hand {
    using namespace ranges::view;
    using namespace ranges;

    std::ranges::sort(tiles);
    Hand h = process_group(tiles);
    std::ranges::sort(h.free_tiles);
    std::ranges::sort(h.triples);
    std::ranges::sort(h.triplets);
    return h;
}
