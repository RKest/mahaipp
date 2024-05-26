#pragma once

#include <random>

#include <pybind11/embed.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <range/v3/view/transform.hpp>
#include <range/v3/algorithm/for_each.hpp>
#include <range/v3/view/enumerate.hpp>
#include <range/v3/action/push_back.hpp>

#include "types.h"
#include "player.h"
#include "eval.h"

using namespace ranges::view;
using namespace ranges;

namespace py {
    const std::vector tiles_game{
        Character1, Character1, Character1, Character1,
        Character2, Character2, Character2, Character2,
        Character3, Character3, Character3, Character3,
        Character4, Character4, Character4, Character4,
        Character5, Character5, Character5, Character5,
        Character6, Character6, Character6, Character6,
        Character7, Character7, Character7, Character7,
        Character8, Character8, Character8, Character8,
        Character9, Character9, Character9, Character9,
        Dots1, Dots1, Dots1, Dots1,
        Dots2, Dots2, Dots2, Dots2,
        Dots3, Dots3, Dots3, Dots3,
        Dots4, Dots4, Dots4, Dots4,
        Dots5, Dots5, Dots5, Dots5,
        Dots6, Dots6, Dots6, Dots6,
        Dots7, Dots7, Dots7, Dots7,
        Dots8, Dots8, Dots8, Dots8,
        Dots9, Dots9, Dots9, Dots9,
        Bamboo1, Bamboo1, Bamboo1, Bamboo1,
        Bamboo2, Bamboo2, Bamboo2, Bamboo2,
        Bamboo3, Bamboo3, Bamboo3, Bamboo3,
        Bamboo4, Bamboo4, Bamboo4, Bamboo4,
        Bamboo5, Bamboo5, Bamboo5, Bamboo5,
        Bamboo6, Bamboo6, Bamboo6, Bamboo6,
        Bamboo7, Bamboo7, Bamboo7, Bamboo7,
        Bamboo8, Bamboo8, Bamboo8, Bamboo8,
        Bamboo9, Bamboo9, Bamboo9, Bamboo9,
        DragonWhite, DragonWhite, DragonWhite, DragonWhite,
        DragonGreen, DragonGreen, DragonGreen, DragonGreen,
        DragonRed, DragonRed, DragonRed, DragonRed,
        WindEast, WindEast, WindEast, WindEast,
        WindSouth, WindSouth, WindSouth, WindSouth,
        WindWest, WindWest, WindWest, WindWest,
        WindNorth, WindNorth, WindNorth, WindNorth,
    };

    enum Result {
        Nothing, Win, Loss, Draw
    };

    struct Step {
        int free_tiles_diff = 0;
        int han_diff = 0;
        Result result = Nothing;
    };

    struct Game {
        void render();

        void reset();

        auto step(std::uint8_t tile_index) -> Step;

        auto discard_obs() -> pybind11::array_t<std::uint8_t>;

        auto hand_obs() -> pybind11::array_t<std::uint8_t>;

        std::vector<Tile> game_tiles;
        std::array<Player, 4> players{
            Player{0, WindEast}, Player{1, WindSouth}, Player{2, WindWest}, Player{3, WindNorth}
        };

        std::size_t players_index_turn = 0;
        std::size_t players_index_east = 0;

        std::random_device rd;
        std::default_random_engine re{rd()};
        std::uniform_int_distribution<std::uint8_t> distr{0, 13};

    private:
        void progress_to_next_player();

        auto current_player() -> Player &;

        auto non_current_players() -> std::vector<std::reference_wrapper<Player> >;
    };

    inline void Game::reset() {

        players_index_turn = 0;
        players_index_east = 0;
        players = std::array{Player{0, WindEast}, Player{1, WindSouth}, Player{2, WindWest}, Player{3, WindNorth}};
        game_tiles = tiles_game;
        std::ranges::shuffle(game_tiles, re);
        assert(game_tiles.size() == num_game_tiles);

        for (auto &player: players) {
            move_tiles(player.tiles_hand, game_tiles, hand_size_init);
            std::ranges::sort(player.tiles_hand);
        }
        current_player().draw(pop_back_ret(game_tiles));
        assert(current_player().tiles_hand.size() == hand_size_max);
    }

    inline auto Game::discard_obs() -> pybind11::array_t<std::uint8_t> {
        std::vector<std::uint8_t> data;
        static_assert(std::same_as<std::underlying_type_t<Tile>, decltype(data)::value_type>);
        for_each(players, [&data](Player &player) {
            data |= push_back(player.tiles_discard);
        });
        data.resize(std::max(data.size(), 87UL)); // std::max is to avoid hiding invarint violations
        return pybind11::array{pybind11::cast(data)};
    }

    inline auto Game::hand_obs() -> pybind11::array_t<std::uint8_t> {
        std::vector<std::uint8_t> data;
        data |= push_back(current_player().tiles_hand);
        return pybind11::array{pybind11::cast(data)};
    }

    inline void Game::progress_to_next_player() {
        players_index_turn = (players_index_turn + 1) % N_Players;
    }

    inline auto Game::current_player() -> Player & {
        return players[players_index_turn];
    }

    inline auto Game::non_current_players() -> std::vector<std::reference_wrapper<Player> > {
        std::vector<std::reference_wrapper<Player> > result;
        result.reserve(size(players) - 1);
        for (auto &&[i, player]: players | ranges::view::enumerate) {
            if (i != players_index_turn) {
                result.push_back(std::ref(player));
            }
        }
        return result;
    }

    inline void Game::render() {
        std::ignore = system("clear");
        for (int i = 1; i < N_Players; ++i) {
            std::cout << "Discard " << i << ": " << players[i].tiles_discard << '\n';
        }

        std::cout << "Hand: " << players[0].tiles_hand << '\n';

        const FullEval eval = players[0].check_for_tsumo();
        std::cout << "Free tiles: " << eval.free_tiles << "\tHan: " << eval.eval.han << std::endl;
    }


    inline auto Game::step(std::uint8_t tile_index) -> Step {
        Player &cp = current_player();
        // Check for Tsumo
        const auto hand = process_hand(cp.tiles_hand);
        const auto pre_eval = cp.check_for_tsumo();
        if (pre_eval.eval.points != 0) {
            render();
            return Step{0, 0, Win};
        }

        Tile discarded = cp.discard_index(tile_index);
        cp.tiles_discard.push_back(discarded);

        // Check for Ron and run non players turns
        for (int i = 0; i < N_Players; ++i) {
            for (auto &p: non_current_players()) {
                if (const FullEval eval = p.get().check_for_ron(discarded); eval.eval.points != 0) {
                    const Result result = p.get().id == 0 ? Win : Loss;
                    if (result == Win) {
                        render();
                    }
                    return Step{0, 0, result};
                }
            }
            progress_to_next_player();
            if (game_tiles.empty()) return Step{0, 0, Draw};
            current_player().draw(pop_back_ret(game_tiles));
            if (current_player().id != 0) {
                if (const auto eval = current_player().check_for_tsumo(); eval.eval.points != 0) {
                    return Step{0, 0, Loss};
                }
                // Have 3 non-controllable players make random moves
                // On the next iteration all players will check for Ron
                discarded = current_player().discard(distr(re));
                current_player().tiles_discard.push_back(discarded);
            }
        }
        const auto post_eval = cp.check_for_tsumo();
        const auto free_tile_diff = pre_eval.free_tiles - post_eval.free_tiles;
        const auto han_diff = post_eval.eval.han - pre_eval.eval.han;
        // State is correct for the next step
        assert(current_player().id == 0);
        return {free_tile_diff, han_diff, Nothing};
    }
}


PYBIND11_MODULE(pymahai, m) {
    m.doc() = "C++ binding for rudimentary mahjong playing capable neural network";

    pybind11::class_<py::Game>(m, "Game")
            .def(pybind11::init<>())
            .def("reset", &py::Game::reset)
            .def("step", &py::Game::step)
            .def("discard_obs", &py::Game::discard_obs)
            .def("hand_obs", &py::Game::hand_obs)
            .def("render", &py::Game::render);

    pybind11::class_<py::Step>(m, "Step")
            .def_readwrite("free_tile_diff", &py::Step::free_tiles_diff)
            .def_readwrite("han_diff", &py::Step::han_diff)
            .def_readwrite("result", &py::Step::result);

    pybind11::enum_<py::Result>(m, "GameResult", pybind11::arithmetic(), "Result of a game")
            .value("Nothing", py::Nothing, "Nothing of interent happened, so no wins, no draws, no losses")
            .value("Win", py::Win, "Player character won")
            .value("Loss", py::Loss, "One of three other players won")
            .value("Draw", py::Draw, "Run out of tiles, nobody won")
            .export_values();
}

static auto _ = []{ std::cout << "V5" << std::endl; return 0; }();
