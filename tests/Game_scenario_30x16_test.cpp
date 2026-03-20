#include <gtest/gtest.h>
#include "../src/Game.hpp"
#include <algorithm>
#include <set>
#include <string>

/** Scénario : my_id=0, width=30, height=16 — grille et snakebots/énergie comme fournis. */
class GameScenario30x16 : public ::testing::Test {
protected:
    void SetUp() override {
        g.my_id = 0;
        g.width = 30;
        g.height = 16;
        g.grid = {
            "....##...##........##...##....",  // row 0
            ".....#........##........#.....",  // row 1
            "...........#......#...........",  // row 2
            "............#....#............",  // row 3
            "..............................",  // row 4
            "..............................",  // row 5
            ".............####.............",  // row 6
            "..............................",  // row 7
            "..............................",  // row 8
            "....#....................#....",  // row 9
            ".....#..................#.....",  // row 10
            "..............................",  // row 11
            "#........##........##........#",  // row 12
            "#.........###....###.........#",  // row 13
            "...#..........##..........#...",  // row 14
            "##############################",  // row 15
        };
        g.my_snakebots = {
            Snakebot(0, parse_body("17,0:17,1:17,2")),
            Snakebot(1, parse_body("10,9:10,10:10,11")),
            Snakebot(2, parse_body("25,12:25,13:25,14")),
            Snakebot(3, parse_body("7,12:7,13:7,14")),
        };
        g.opp_snakebots = {
            Snakebot(4, parse_body("12,0:12,1:12,2")),
            Snakebot(5, parse_body("19,9:19,10:19,11")),
            Snakebot(6, parse_body("4,12:4,13:4,14")),
            Snakebot(7, parse_body("22,12:22,13:22,14")),
        };
        for (int id = 0; id <= 7; ++id) {
            g.last_direction[id] = UP;
        }
        energy = {
            {2, 0}, {27, 0}, {7, 0}, {22, 0},
            {2, 3}, {27, 3},
            {9, 11}, {20, 11},
            {2, 13}, {27, 13},
            {2, 2}, {27, 2}, {8, 2}, {21, 2},
            {2, 6}, {27, 6},
            {5, 7}, {24, 7}, {11, 7}, {18, 7},
            {7, 11}, {22, 11}, {12, 11}, {17, 11},
            {3, 12}, {26, 12},
        };
    }

    Game g;
    std::set<Point> energy;
};

TEST_F(GameScenario30x16, InitDimensions) {
    EXPECT_EQ(g.my_id, 0);
    EXPECT_EQ(g.width, 30);
    EXPECT_EQ(g.height, 16);
    EXPECT_EQ(static_cast<int>(g.grid.size()), 16);
    for (const auto& row : g.grid) {
        EXPECT_EQ(static_cast<int>(row.size()), 30) << "Chaque ligne doit faire 30 caractères";
    }
}

TEST_F(GameScenario30x16, EnergyCount) {
    EXPECT_EQ(energy.size(), 26u);
}

/** Snake 2 (tête 25,12) : premier pas vers l’énergie la plus proche = RIGHT. */
TEST_F(GameScenario30x16, Snake2_FirstActionIsRight) {
    g.recalculate_possible_actions(energy);
    bool snake2_right = std::any_of(g.actions.begin(), g.actions.end(),
        [](const std::string& a) {
            return a.size() >= 1 && a[0] == '2' && a.find("RIGHT") != std::string::npos;
        });
    EXPECT_TRUE(snake2_right) << "Snake 2 doit choisir RIGHT";
}

TEST_F(GameScenario30x16, RecalculatePossibleActionsValidFormat) {
    g.recalculate_possible_actions(energy);
    ASSERT_EQ(g.actions.size(), 4u);
    
    for (const std::string& a : g.actions) {
        if (a == "WAIT") continue;
        size_t space = a.find(' ');
        ASSERT_NE(space, std::string::npos) << "Action doit contenir un espace: " << a;
        std::string dir = a.substr(space + 1);
        EXPECT_TRUE(dir == UP || dir == DOWN || dir == LEFT || dir == RIGHT)
            << "Direction invalide dans: " << a;
    }
}
