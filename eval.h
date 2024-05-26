#pragma once

#include <cmath>

#include "predicated.h"

#include <range/v3/numeric/accumulate.hpp>

using Points = int;
using Fu = int;
using HanNum = int;

enum Situation {
    Ron,
    Tsumo,
};

struct Eval {
    HanNum han{};
    Points points{};
};

constexpr int points_yakuman = 32'000;
constexpr int points_sanbaiman = 24'000;
constexpr int points_baiman = 16'000;
constexpr int points_haneman = 12'000;
constexpr int points_mangan = 12'000;

auto eval_hand(const Hand &hand, Situation situation, Tile wind) -> Eval;

auto calculate_fu(const Hand &hand, Situation situation, Tile wind) -> Fu;

inline auto wind_mult(Tile wind) {
    return (1 + (0.5 * (wind == WindEast)));
}

inline auto process_and_eval_hand(std::vector<Tile> hand, Situation situation, Tile wind) -> Eval {
    return eval_hand(process_hand(std::move(hand)), situation, wind);
}

inline auto eval_hand(const Hand &hand, Situation situation, Tile wind) -> Eval {
    int han = 0;
    if (!hand.free_tiles.empty()) {
        if (thirteen_orphans(hand)) {
            return {13, static_cast<int>(points_yakuman * wind_mult(wind))};
        }
        if (seven_pairs(hand)) {
            han += 2;
        }
    }
    for (auto pred: han_1) if (pred(hand)) han += 1;
    for (auto pred: han_2) if (pred(hand)) han += 2;
    for (auto pred: han_3) if (pred(hand)) han += 3;
    if (full_flush(hand)) han += 6;
    for (auto pred: yakuman) if (pred(hand)) han += 13;

    if (!hand.free_tiles.empty()) {
        return {han, 0};
    }

    if (han == 0) return {0, 0};
    if (han >= 13) return {han, static_cast<int>(points_yakuman * wind_mult(wind))};
    if (han >= 11) return {han, static_cast<int>(points_sanbaiman * wind_mult(wind))};
    if (han >= 8) return {han, static_cast<int>(points_baiman * wind_mult(wind))};
    if (han >= 6) return {han, static_cast<int>(points_haneman * wind_mult(wind))};
    if (han >= 5) return {han, static_cast<int>(points_mangan * wind_mult(wind))};

    const int fu = calculate_fu(hand, situation, wind);
    const int basic_points = std::ceil(fu * std::pow(2, 2 + han) / 100.0) * 100.0 * wind_mult(wind) * 4;
    return {han, basic_points};
}

inline auto calculate_fu(const Hand &hand, Situation situation, Tile wind) -> Fu {
    constexpr auto fuutei = 20;
    const auto menzen_kafu = situation == Ron ? 10 : 0;
    const auto toitsu = is_dragon(hand.pair) ? 2 : ((hand.pair == WindEast) + (hand.pair == wind)) * 2;
    // Not distinguishing between this and minkou, too much of a hastle
    const auto ankou = accumulate(
        hand.triples | transform([](Tile t) { return is_honor(t) || is_terminal(t) ? 4 : 2; }), 0);
    // Also not counting ankan nor minkan, because you cannot kan right now
    // Also also not counting waiting fu, because who cares actually
    return static_cast<int>(std::ceil((fuutei + menzen_kafu + toitsu + ankou) / 10.0) * 10.0);
}
