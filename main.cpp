#include <cstdint>
#include <array>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <random>
#include <iostream>
#include <thread>

#include "display.h"
#include "player.h"
#include "types.h"
#include "utils.h"
#include "predicated.h"
#include "eval.h"
#include "py_comms.h"

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

struct Game {
    std::vector<Tile> game_tiles;
    std::array<Player, 4> players{Player{0, WindEast}, Player{1, WindSouth}, Player{2, WindWest}, Player{3, WindNorth}};

    std::size_t players_index_turn = 0;
    std::size_t players_index_east = 0;

    void progress_to_next_player();
    auto current_player() -> Player&;
    auto non_current_players() -> std::vector<std::reference_wrapper<Player>>;
};

auto start_game() -> Game;

void display_game();

auto prompt_tile() -> std::underlying_type_t<Tile>;

void win();

void open_kan();

int main() {
    srand(time(nullptr));
    // Stats
    std::array<int, N_Players> player_wins{0, 0, 0, 0};
    int wins = 0;
    int draws = 0;

    for (auto i = 0; i < number_of_games; ++i) {
        try {
            Game g = start_game();
            while (1) {
                g.current_player().draw(pop_back_ret(g.game_tiles));
                if (auto points = process_and_eval_hand(g.current_player().tiles_hand, Tsumo, g.current_player().self_wind); points != 0) {
                    player_wins[g.players_index_turn] += 1;
                    goto NextGame;
                }
                Tile discarded = g.current_player().discard(prompt_tile());
                for (auto& player : g.non_current_players()) {
                    player.get().draw(discarded);
                    if (auto points = process_and_eval_hand(player.get().tiles_hand, Ron, player.get().self_wind); points != 0) {
                        player_wins[player.get().id] += 1;
                        goto NextGame;
                    }
                    player.get().discard(discarded);
                }
                g.progress_to_next_player();
            }
        } catch (game_end &end) {
            draws += 1;
        }
    NextGame:;
    }
    std::cout << "Wins: ";
    for (auto &p : player_wins) {
        std::cout << p << ',';
    }
    std::cout << '\n';
    std::cout << "Draws: " << draws<< '\n';
}

auto start_game() -> Game {
    Game g;
    g.game_tiles = tiles_game;
    static std::random_device rd;
    static std::default_random_engine re{rd()};
    std::ranges::shuffle(g.game_tiles, re);
    assert(g.game_tiles.size() == num_game_tiles);

    for (auto &player: g.players) {
        move_tiles(player.tiles_hand, g.game_tiles, hand_size_init);
        std::ranges::sort(player.tiles_hand);
    }
    return g;
}

void display_game(const Game &g) {
    [[maybe_unused]] auto _ = system("clear");

    std::cout << "Player `" << g.players_index_east << "` is east\n\n";
    std::cout << "Player `" << g.players_index_turn << "` is currently thinking\n\n";

    for (const auto &player: g.players) {
        player.display();
    }
}

void win() {
    assert(false && "Not implemented");
}

auto prompt_tile() -> std::underlying_type_t<Tile> {
    return rand();
}
