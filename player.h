#pragma once

#include <cassert>
#include <vector>
#include <algorithm>
#include <iostream>

#include "types.h"
#include "utils.h"
#include "display.h"
#include "eval.h"

struct FullEval {
    int free_tiles = 14;
    Eval eval{};
};

struct Player {
    int id;

    Tile self_wind;
    std::vector<Tile> tiles_hand;
    std::vector<Tile> tiles_discard;

    auto check_for_ron(Tile t) -> FullEval;

    auto check_for_tsumo() const -> FullEval;

    void draw(Tile t);

    auto discard(std::underlying_type_t<Tile> inexact_tile) -> Tile;

    auto discard_index(std::uint8_t index) -> Tile;

    void display() const;
};

void Player::draw(Tile t) {
    tiles_hand.push_back(t);
    std::ranges::sort(tiles_hand);
}

auto Player::discard(std::underlying_type_t<Tile> inexact_tile) -> Tile {
    assert(!tiles_hand.empty());

    Tile *closest_iter = closest_tile(tiles_hand, inexact_tile);
    Tile to_discard = *closest_iter;
    tiles_hand.erase(begin(tiles_hand) + std::distance(&*begin(tiles_hand), closest_iter));
    return to_discard;
}

inline auto Player::discard_index(std::uint8_t index) -> Tile {
    auto to_discard = tiles_hand.at(index);
    tiles_hand.erase(std::begin(tiles_hand) + index);
    return to_discard;
}


void Player::display() const {
    std::cout << "Player: " << id << '\n';
    std::cout << "Discard: " << tiles_discard << '\n';
    std::cout << "Hand: " << tiles_hand << '\n';
}

inline auto Player::check_for_ron(Tile t) -> FullEval {
    assert(tiles_hand.size() == hand_size_init);
    draw(t);
    const auto hand = process_hand(tiles_hand);
    const auto eval = FullEval{
        .free_tiles = static_cast<int>(ssize(hand.free_tiles)),
        .eval = eval_hand(hand, Ron, self_wind),
    };
    if (eval.eval.points == 0) {
        discard(t);
    }
    return eval;
}

inline auto Player::check_for_tsumo() const -> FullEval {
    assert(tiles_hand.size() == hand_size_max);
    const auto hand = process_hand(tiles_hand);
    const auto eval = FullEval{
        .free_tiles = static_cast<int>(ssize(hand.free_tiles)),
        .eval = eval_hand(hand, Tsumo, self_wind),
    };
    return eval;
}
