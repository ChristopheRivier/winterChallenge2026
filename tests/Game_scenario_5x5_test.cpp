#include <gtest/gtest.h>
#include "../src/Game.hpp"
#include <algorithm>
#include <set>
#include <string>

/**
 * Grille 5x5 : dernière ligne = plateforme (#).
 * Tête au centre (2,2), corps vertical sur la colonne 2.
 * Mur en (2,1) pour interdire UP (sinon UP puis LEFT atteint l’énergie en 2 pas).
 * Énergie en (1,1) = décalage (-1,-1) par rapport à la tête.
 * Le premier pas optimal vers l’énergie doit être LEFT.
 */
class GameScenario5x5 : public ::testing::Test {
protected:
    void SetUp() override {
        g.my_id = 0;
        g.width = 5;
        g.height = 5;
        g.grid = {
            ".....",
            ".....", 
            ".....",  // tête (2,2), corps (2,3),(2,4) — définis via parse_body
            ".....",
            "#####",
        };
        g.my_snakebots = {
            Snakebot(0, parse_body("2,1:2,2:2,3"), LEFT),
        };
        g.opp_snakebots.clear();
        g.last_direction[0] = RIGHT;
        energy = {{1, 0}};
    }

    Game g;
    std::set<Point> energy;
};

TEST_F(GameScenario5x5, FirstStepTowardEnergyIsLeft) {
    Snake snake(parse_body("2,2:2,3:2,4"));
    std::set<Point> others_body;
    Point step = g.first_step_toward_nearest_reachable_energy(snake, others_body, energy);
    EXPECT_EQ(step, Point(-1, 0)) << "Premier pas vers l’énergie (1,1) doit être LEFT";
}

TEST_F(GameScenario5x5, RecalculateActionsSnake0Left) {
    g.recalculate_possible_actions(energy);
    auto it = std::find_if(g.actions.begin(), g.actions.end(),
        [](const std::string& a) { return a.size() >= 2 && a[0] == '0' && a[1] == ' '; });
    ASSERT_NE(it, g.actions.end()) << "Action pour le bot 0 attendue";
    size_t sp = it->find(' ');
    ASSERT_NE(sp, std::string::npos);
    EXPECT_EQ(it->substr(sp + 1), LEFT);
}
