#include <gtest/gtest.h>
#include "../src/Game.hpp"
#include <algorithm>
#include <set>
#include <string>

/** Scénario : my_id=0, width=36, height=20 — une seule équipe (snake 0), 8 sources d’énergie. */
class GameScenario36x20 : public ::testing::Test {
protected:
    void SetUp() override {
        g.my_id = 0;
        g.width = 36;
        g.height = 20;
        g.grid = {
            "....................................",  // row 0
            "...............##..##...............",  // row 1
            "....................................",  // row 2
            ".......#......#..##..#......#.......",  // row 3
            "......#..#...#........#...#..#......",  // row 4
            "##......#..................#......##",  // row 5
            ".....#........................#.....",  // row 6
            "......#......................#......",  // row 7
            ".................##.................",  // row 8
            "#.........#..............#.........#",  // row 9
            "#.#.......#.##........##.#.......#.#",  // row 10
            "..#......#...#........#...#......#..",  // row 11
            "...#.......#..#......#..#.......#...",  // row 12
            "...........##....##....##...........",  // row 13
            ".........#..#...#..#...#..#.........",  // row 14
            ".....###..#.....#..#.....#..###.....",  // row 15
            ".###..##....................##..###.",  // row 16
            ".####..##..###........###..##..####.",  // row 17
            ".#####....#####......#####....#####.",  // row 18
            "####################################",  // row 19
        };
        g.my_snakebots = {
            Snakebot(0, parse_body("34,11:34,12:35,12:35,13:35,14:35,15:34,15:33,15:32,15:32,14:31,14:30,14")),
        };
        g.opp_snakebots.clear();
        g.last_direction[0] = UP;

        energy = {
            {11, 2}, {24, 2}, {17, 7}, {18, 7},
            {3, 1}, {32, 1}, {9, 2}, {26, 2},
        };
    }

    Game g;
    std::set<Point> energy;
};

TEST_F(GameScenario36x20, InitDimensions) {
    EXPECT_EQ(g.my_id, 0);
    EXPECT_EQ(g.width, 36);
    EXPECT_EQ(g.height, 20);
    EXPECT_EQ(static_cast<int>(g.grid.size()), 20);
    for (const auto& row : g.grid) {
        EXPECT_EQ(static_cast<int>(row.size()), 36) << "Chaque ligne doit faire 36 caractères";
    }
}

TEST_F(GameScenario36x20, EnergyCount) {
    EXPECT_EQ(energy.size(), 8u);
}


TEST_F(GameScenario36x20, RecalculatePossibleActionsOneBot) {
    g.recalculate_possible_actions(energy);
    ASSERT_EQ(g.actions.size(), 1u);
    const std::string& a = g.actions[0];
    ASSERT_NE(a.find(' '), std::string::npos) << "Format attendu: \"0 DIRECTION\", reçu: " << a;
    EXPECT_EQ(a[0], '0') << "Action du snake 0: " << a;
    std::string dir = a.substr(a.find(' ') + 1);
    EXPECT_TRUE(dir == UP)
        << "Direction invalide: " << dir;
}
