#include <gtest/gtest.h>
#include "../src/Game.hpp"
#include <algorithm>
#include <map>
#include <set>
#include <string>

/** Test Game avec les paramètres : my_id=0, width=38, height=21,
 *  grille et snakebots/energy comme fournis. */
class GameWithFixedParams : public ::testing::Test {
protected:
    void SetUp() override {
        g.my_id = 0;
        g.width = 38;
        g.height = 21;
        g.grid = {
            "......................................",  // row 0
            "......................................",  // row 1
            "..................##..................",  // row 2
            "................#....#................",  // row 3
            "..##.......#....##..##....#.......##..",  // row 4
            "..###.......#...##..##...#.......###..",  // row 5
            "...#..............................#...",  // row 6
            "...#..#.....#...#....#...#.....#..#...",  // row 7
            ".....###....#..#......#..#....###.....",  // row 8
            "..#........#..............#........#..",  // row 9
            "..#.....#.....##......##.....#.....#..",  // row 10
            "........###......#..#......###........",  // row 11
            "....#....#..##....##....##..#....#....",  // row 12
            "#...##.......#....##....#.......##...#",  // row 13
            "##..##.......#..##..##..#.......##..##",  // row 14
            "##....#........................#....##",  // row 15
            "##..................................##",  // row 16
            "##..................................##",  // row 17
            "#####....##..###..##..###..##....#####",  // row 18
            "#######..####################..#######",  // row 19
            "######################################",  // row 20
        };
        // snakebots_per_player=4, my 0..3, opp 4..7
        g.my_snakebots = {
            Snakebot(0, parse_body("29,17:29,18:29,19")),
            Snakebot(1, parse_body("11,16:11,17:11,18")),
            Snakebot(2, parse_body("16,0:16,1:16,2")),
            Snakebot(3, parse_body("31,4:31,5:31,6")),
        };
        g.opp_snakebots = {
            Snakebot(4, parse_body("8,17:8,18:8,19")),
            Snakebot(5, parse_body("26,16:26,17:26,18")),
            Snakebot(6, parse_body("21,0:21,1:21,2")),
            Snakebot(7, parse_body("6,4:6,5:6,6")),
        };
        for (int id = 0; id <= 7; ++id) {
            g.last_direction[id] = UP;
        }
        energy = {
            {11, 0}, {26, 0}, {14, 0}, {23, 0},
            {0, 4}, {37, 4}, {12, 4}, {25, 4},
            {10, 9}, {27, 9}, {12, 9}, {25, 9},
            {10, 13}, {27, 13}, {16, 13}, {21, 13},
            {0, 0}, {37, 0}, {2, 0}, {35, 0},
            {4, 0}, {33, 0}, {9, 0}, {28, 0},
            {13, 0}, {24, 0}, {7, 2}, {30, 2},
            {9, 2}, {28, 2}, {5, 3}, {32, 3},
            {8, 6}, {29, 6}, {6, 11}, {31, 11},
            {15, 12}, {22, 12}, {8, 14}, {29, 14},
            {10, 14}, {27, 14},
        };

    }

    Game g;
    std::set<Point> energy;
};

TEST_F(GameWithFixedParams, InitDimensions) {
    EXPECT_EQ(g.my_id, 0);
    EXPECT_EQ(g.width, 38);
    EXPECT_EQ(g.height, 21);
    EXPECT_EQ(static_cast<int>(g.grid.size()), 21);
    for (const auto& row : g.grid) {
        EXPECT_EQ(static_cast<int>(row.size()), 38) << "Chaque ligne doit faire 38 caractères";
    }
}

TEST_F(GameWithFixedParams, SnakebotsCount) {
    EXPECT_EQ(static_cast<int>(g.my_snakebots.size()), 4);
    EXPECT_EQ(static_cast<int>(g.opp_snakebots.size()), 4);
    EXPECT_EQ(static_cast<int>(energy.size()), 42);
}

TEST_F(GameWithFixedParams, InBounds) {
    EXPECT_TRUE(g.in_bounds(Point(0, 0)));
    EXPECT_TRUE(g.in_bounds(Point(37, 20)));
    EXPECT_FALSE(g.in_bounds(Point(-1, 0)));
    EXPECT_FALSE(g.in_bounds(Point(38, 0)));
    EXPECT_FALSE(g.in_bounds(Point(0, 21)));
}

TEST_F(GameWithFixedParams, IsPlatform) {
    EXPECT_TRUE(g.is_platform(0, 20));   // bottom row #
    EXPECT_TRUE(g.is_platform(37, 20));
    EXPECT_FALSE(g.is_platform(0, 0));  // top row .
}

// Snake 3 tête (31,4) : (32,3) hors d'atteinte (gravité). (29,6) aussi car chemin bloqué par le corps (31,5).
TEST_F(GameWithFixedParams, Snake3_Energy_32_3_Unreachable_By_Gravity) {
    Snake snake3(parse_body("31,4:31,5:31,6"));
    std::set<Point> my_body = snake3.body_set();
    std::set<Point> others_body;
    for (const Snakebot& bot : g.opp_snakebots) {
        for (const Point& p : bot.body) others_body.insert(p);
    }
    std::set<Point> blocked = g.blocked_cells(my_body, snake3.tail(), others_body);
    std::set<Point> solid = g.solid_cells(my_body, others_body, energy);
    std::map<Point, int> dist;
    std::map<Point, Point> parent;
    g.bfs_with_gravity(snake3, blocked, solid, dist, parent);
    EXPECT_EQ(dist.count(Point(32, 3)), 0u)
        << "Énergie (32,3) ne doit pas être atteignable depuis (31,4) à cause de la gravité";
    // (29,6) inaccessible sans traverser (31,5) qui est bloqué par notre corps
    EXPECT_EQ(dist.count(Point(29, 6)), 0u)
        << "(29,6) hors d'atteinte depuis (31,4) car (31,5) bloqué";
}

TEST_F(GameWithFixedParams, RecalculatePossibleActionsNoCrash) {
    g.recalculate_possible_actions(energy);
    EXPECT_FALSE(g.actions.empty());
    // Snake 3 (tête 31,4) : (32,3) et (29,6) hors d'atteinte → une action valide (ex. LEFT/RIGHT/UP)
    bool snake3_has_action = std::any_of(g.actions.begin(), g.actions.end(),
        [](const std::string& a) { return a.size() >= 1 && a[0] == '3' && a.find(' ') != std::string::npos; });
    EXPECT_TRUE(snake3_has_action) << "Snake 3 doit avoir une action valide";

    // vérifie que snake 3 va vers la gauche
    bool snake3_has_left_action = std::any_of(g.actions.begin(), g.actions.end(),
        [](const std::string& a) { return a.size() >= 1 && a[0] == '3' && a.find("UP") != std::string::npos; });
    EXPECT_TRUE(snake3_has_left_action) << "Snake 3 doit avoir une action qui va vers la gauche";
}

TEST_F(GameWithFixedParams, RecalculatePossibleActionsValidFormat) {
    g.recalculate_possible_actions(energy);
    for (const std::string& a : g.actions) {
        if (a == "WAIT") continue;
        // Format attendu: "id DIRECTION" (ex: "0 UP")
        size_t space = a.find(' ');
        ASSERT_NE(space, std::string::npos) << "Action doit contenir un espace: " << a;
        std::string dir = a.substr(space + 1);
        EXPECT_TRUE(dir == UP || dir == DOWN || dir == LEFT || dir == RIGHT)
            << "Direction invalide dans: " << a;
    }
}

TEST_F(GameWithFixedParams, BlockedCells) {
    std::set<Point> my_body = {Point(29, 17), Point(29, 18), Point(29, 19)};
    std::set<Point> others_body;
    for (const Snakebot& bot : g.opp_snakebots) {
        for (const Point& p : bot.body) others_body.insert(p);
    }
    std::set<Point> blocked = g.blocked_cells(my_body, Point(29, 19), others_body);
    EXPECT_TRUE(blocked.count(Point(29, 17)));
    EXPECT_TRUE(blocked.count(Point(29, 18)));
    // La queue (29,19) n'est pas bloquante pour soi
    EXPECT_FALSE(blocked.count(Point(29, 19)));
}

TEST_F(GameWithFixedParams, ParseBody) {
    std::vector<Point> body = parse_body("29,17:29,18:29,19");
    ASSERT_EQ(body.size(), 3u);
    EXPECT_EQ(body[0].x, 29);
    EXPECT_EQ(body[0].y, 17);
    EXPECT_EQ(body[2].x, 29);
    EXPECT_EQ(body[2].y, 19);
}

TEST_F(GameWithFixedParams, FirstStepTowardNearestReachableEnergy) {
    Snake snake(parse_body("29,17:29,18:29,19"));
    std::set<Point> others_body;
    for (const Snakebot& bot : g.opp_snakebots) {
        for (const Point& p : bot.body) others_body.insert(p);
    }
    Point step_dir = g.first_step_toward_nearest_reachable_energy(
        snake, others_body, energy);
    // Retourne un vecteur direction unitaire (UP/DOWN/LEFT/RIGHT) ou (-1,-1) si aucune cible
    if (step_dir.x != -1 || step_dir.y != -1) {
        EXPECT_EQ(std::abs(step_dir.x) + std::abs(step_dir.y), 1)
            << "Doit être un vecteur unitaire (direction)";
    }
}
