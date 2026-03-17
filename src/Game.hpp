#ifndef GAME_HPP
#define GAME_HPP

#include <string>
#include <vector>
#include <map>
#include <set>


const std::string UP = "UP";
const std::string DOWN = "DOWN";
const std::string LEFT = "LEFT";
const std::string RIGHT = "RIGHT";

struct Point {
    int x, y;
    Point() : x(0), y(0) {}
    Point(int x, int y) : x(x), y(y) {}
    Point operator+(const Point& o) const { return Point(x + o.x, y + o.y); }
    bool operator<(const Point& o) const {
        if (x != o.x) return x < o.x;
        return y < o.y;
    }
    bool operator==(const Point& o) const { return x == o.x && y == o.y; }
    bool operator!=(const Point& o) const { return x != o.x || y != o.y; }
};

struct Snakebot {
    int id;
    std::vector<Point> body;
    std::string last_direction;

    Snakebot() : id(-1), last_direction(UP) {}
    Snakebot(int id, const std::vector<Point>& body, const std::string& last_dir = UP)
        : id(id), body(body), last_direction(last_dir) {}

    bool empty() const { return body.empty(); }
    Point head() const { return body.empty() ? Point() : body[0]; }
};



std::vector<Point> parse_body(const std::string& body) {
    std::vector<Point> segs;
    std::string s = body;
    for (char& c : s) if (c == ';' || c == ':') c = ' ';
    std::istringstream iss(s);
    std::string token;
    while (iss >> token) {
        size_t comma = token.find(',');
        if (comma != std::string::npos) {
            int x = std::stoi(token.substr(0, comma));
            int y = std::stoi(token.substr(comma + 1));
            segs.push_back(Point(x, y));
        }
    }
    return segs;
}

Point dir_to_point(const std::string& d) {
    if (d == UP) return Point(0, -1);
    if (d == DOWN) return Point(0, 1);
    if (d == LEFT) return Point(-1, 0);
    if (d == RIGHT) return Point(1, 0);
    return Point(0, -1);
}

std::string point_to_dir(const Point& p) {
    if (p.x == 0 && p.y == -1) return UP;
    if (p.x == 0 && p.y == 1) return DOWN;
    if (p.x == -1 && p.y == 0) return LEFT;
    if (p.x == 1 && p.y == 0) return RIGHT;
    return UP;
}

int manhattan(const Point& a, const Point& b) {
    return std::abs(a.x - b.x) + std::abs(a.y - b.y);
}

class Game {
public:
    int width = 0;
    int height = 0;
    int my_id = 0;
    std::vector<std::string> grid;
    std::vector<Snakebot> my_snakebots;
    std::vector<Snakebot> opp_snakebots;
    std::vector<std::string> actions;

    Game() = default;

    /** Vide la liste des actions (à appeler en début de tour). */
    void clear_actions() {
        actions.clear();
    }

    /** Vide les listes de snakebots (pour les re-remplir avec l’état du tour). */
    void clear_snakebots() {
        my_snakebots.clear();
        opp_snakebots.clear();
    }
    bool in_bounds(const Game& g, const Point& p) {
        return p.x >= 0 && p.x < g.width && p.y >= 0 && p.y < g.height;
    }


    bool is_platform(const Game& g, int x, int y) {
        if (x < 0 || x >= g.width || y < 0 || y >= g.height) return false;
        char c = g.grid[y][x];
        return c == '#' || c == '1' || c == 'X';
    }
    /** Recalcule les actions possibles et remplit `actions` pour les miens. */
    void recalculate_possible_actions(const std::set<Point>& energy) {
        clear_actions();
    
        for (const Snakebot& bot : my_snakebots) {
            if (bot.empty()) continue;
    
            const std::vector<Point>& body = bot.body;
            Point head = body[0];
            std::string cur_dir = last_direction.count(bot.id) ? last_direction[bot.id] : UP;
            std::set<Point> my_body_set(body.begin(), body.end());
    
            std::set<Point> others_body;
            for (const Snakebot& other : my_snakebots) {
                if (other.id == bot.id) continue;
                for (const Point& p : other.body) others_body.insert(p);
            }
            for (const Snakebot& other : opp_snakebots) {
                for (const Point& p : other.body) others_body.insert(p);
            }
    
            std::string best_dir = cur_dir;
            int best_score = -2000000000;
            std::vector<std::string> dirs = {UP, DOWN, LEFT, RIGHT};
    
            for (const std::string& d : dirs) {
                Point delta = dir_to_point(d);
                Point next_head = head + delta;
                if (!in_bounds(*this, next_head)) continue;
                if (is_platform(*this, next_head.x, next_head.y)) continue;
                if (my_body_set.count(next_head) && next_head != body.back()) continue;
                if (others_body.count(next_head)) continue;
    
                int score = 0;
                if (energy.count(next_head)) score += 10000;
                int nearest = 1000000000;
                for (const Point& e : energy) {
                    nearest = std::min(nearest, manhattan(next_head, e));
                }
                score -= nearest * 2;
                if (next_head.y >= height - 2) score -= 100;
                if (score > best_score) {
                    best_score = score;
                    best_dir = d;
                }
            }
    
            last_direction[bot.id] = best_dir;
            if (best_score > -2000000000) {
                actions.push_back(std::to_string(bot.id) + " " + best_dir);
            }
        }
    
        if (actions.empty()) {
            actions.push_back("WAIT");
        }
    }

    /** Dernière direction envoyée par snakebot id (pour éviter les demi-tours). */
    std::map<int, std::string> last_direction;
};




#endif
