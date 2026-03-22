#include <gtest/gtest.h>
#include "../src/Game.hpp"
#include <set>
#include <string>

/**
 * Scénario : my_id=0, width=26, height=14 — grille (lignes 0–13),
 * power_source_count=3, snakebot_count=3 (0 et 2 alliés, 5 adverse).
 */
class GameScenario26x14 : public ::testing::Test {
protected:
    void SetUp() override {
        g.my_id = 0;
        g.width = 26;
        g.height = 14;
        g.grid = {
            "............##............",  // row 0
            ".........#......#.........",  // row 1
            "..........#....#..........",  // row 2
            "..........................",  // row 3
            "............##............",  // row 4
            "...........####...........",  // row 5
            "..........#....#..........",  // row 6
            "#...#................#...#",  // row 7
            ".#...#..............#...#.",  // row 8
            ".......#..........#.......",  // row 9
            ".......#...#..#...#.......",  // row 10
            "###..#######..#######..###",  // row 11
            ".....################.....",  // row 12
            "##########################",  // row 13
        };
        g.my_snakebots = {
            Snakebot(0, parse_body("25,8:25,9:25,10:24,10:23,10:23,9:23,8:23,7:22,7"), UP),
            Snakebot(2, parse_body("8,6:8,7:8,8:8,9:8,10"), UP),
        };
        g.opp_snakebots = {
            Snakebot(5, parse_body("3,5:3,4:3,3:3,2:4,2:4,3:4,4:4,5:4,6"), UP),
        };
        for (int id : {0, 2, 5}) {
            g.last_direction[id] = UP;
        }
        energy = {{25, 8}, {14, 2}, {11, 2}};
    }

    Game g;
    std::set<Point> energy;
};

TEST_F(GameScenario26x14, InitDimensions) {
    EXPECT_EQ(g.my_id, 0);
    EXPECT_EQ(g.width, 26);
    EXPECT_EQ(g.height, 14);
    EXPECT_EQ(static_cast<int>(g.grid.size()), 14);
    for (const auto& row : g.grid) {
        EXPECT_EQ(static_cast<int>(row.size()), 26) << "Chaque ligne doit faire 26 caractères";
    }
}

TEST_F(GameScenario26x14, EnergyCount) {
    EXPECT_EQ(energy.size(), 3u);
}

TEST_F(GameScenario26x14, RecalculatePossibleActionsTwoMySnakes) {
    g.recalculate_possible_actions(energy);
    /* Le bot 0 a la tête sur l’énergie (25,8) mais aucun pas légal adjacent (murs #, corps, bord) ;
       seul le bot 2 reçoit une direction. */
    ASSERT_EQ(g.actions.size(), 1u);
    EXPECT_NE(g.actions[0].find("2 "), std::string::npos) << "Action attendue pour le bot 2: " << g.actions[0];
    for (const std::string& a : g.actions) {
        size_t space = a.find(' ');
        ASSERT_NE(space, std::string::npos) << "Action doit contenir un espace: " << a;
        std::string dir = a.substr(space + 1);
        EXPECT_TRUE(dir == UP || dir == DOWN || dir == LEFT || dir == RIGHT)
            << "Direction invalide dans: " << a;
    }
}
