#ifndef GAME_HPP
#define GAME_HPP

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <limits>
#include <functional>


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
    bool in_bounds(const Point& p) const {
        return p.x >= 0 && p.x < width && p.y >= 0 && p.y < height;
    }

    bool is_platform(int x, int y) const {
        if (x < 0 || x >= width || y < 0 || y >= height) return false;
        char c = grid[y][x];
        return c == '#' || c == '1' || c == 'X';
    }

    /** Cellules à ne pas traverser : murs + corps des autres snakes + notre corps (sauf la queue). */
    std::set<Point> blocked_cells(const std::set<Point>& my_body_set, const Point& my_tail,
                                  const std::set<Point>& others_body) const {
        std::set<Point> blocked;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (is_platform(x, y)) blocked.insert(Point(x, y));
            }
        }
        for (const Point& p : others_body) blocked.insert(p);
        for (const Point& p : my_body_set) {
            if (p != my_tail) blocked.insert(p);
        }
        return blocked;
    }

    /** Cellules solides (qui peuvent servir de support avec la gravité) : plateformes, énergie, tous les corps. */
    std::set<Point> solid_cells(const std::set<Point>& my_body_set,
                                const std::set<Point>& others_body,
                                const std::set<Point>& energy) const {
        std::set<Point> solid;
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                if (is_platform(x, y)) solid.insert(Point(x, y));
            }
        }
        for (const Point& p : energy) solid.insert(p);
        for (const Point& p : my_body_set) solid.insert(p);
        for (const Point& p : others_body) solid.insert(p);
        return solid;
    }

    /** Une case est supportée (on peut s’y tenir avec la gravité) si elle est au sol ou a du solide en dessous. */
    bool is_supported(const Point& p, const std::set<Point>& solid) const {
        if (!in_bounds(p)) return false;
        if (p.y == height - 1) return true;  // dernière ligne = sol
        Point below(p.x, p.y + 1);
        return solid.count(below) != 0;
    }

    /** Déplace la tête d'une case dans la direction delta. S'arrête (n'entre pas) si la case
     *  est un mur (blocked) ou si le snake n'est plus sur un mur (case non supportée). */
    bool can_step(Point cur, const Point& delta, const std::set<Point>& blocked,
                  const std::set<Point>& solid, Point& out_next) const {
        Point n = cur + delta;
        if (!in_bounds(n)) return false;
        if (blocked.count(n)) return false;  // un mur arrête le mouvement
        if (!is_supported(n, solid)) return false;  // snake n'est plus sur un mur
        out_next = n;
        return true;
    }

    /** BFS récursif depuis start avec gravité : on bouge le snake case par case.
     *  Condition d'arrêt : un mur arrête le mouvement ; on n'entre pas sur une case non supportée
     *  (snake plus sur un mur). parent_direction[n] = vecteur unitaire parent -> n. */
    void bfs_with_gravity(const Point& start, const std::set<Point>& blocked,
                         const std::set<Point>& solid,
                         std::map<Point, int>& dist, std::map<Point, Point>& parent,
                         std::map<Point, Point>* parent_direction = nullptr) const {
        dist.clear();
        parent.clear();
        if (parent_direction) parent_direction->clear();
        static const std::vector<Point> deltas = {{0,-1},{0,1},{-1,0},{1,0}};

        std::function<void(Point, int)> rec;
        rec = [&](Point cur, int d) {
            auto it = dist.find(cur);
            if (it != dist.end() && it->second <= d) return;
            dist[cur] = d;

            Point n;
            for (const Point& delta : deltas) {
                if (!can_step(cur, delta, blocked, solid, n)) continue;
                int nd = d + 1;
                auto nit = dist.find(n);
                if (nit == dist.end() || nit->second > nd) {
                    parent[n] = cur;
                    if (parent_direction) (*parent_direction)[n] = delta;
                    rec(n, nd);
                }
            }
        };
        rec(start, 0);
    }

    /** Trouve l’énergie atteignable la plus proche en tenant compte de la gravité
     *  (sans traverser murs ni autres snakes, uniquement sur cases supportées).
     *  Retourne le vecteur unitaire de la direction à prendre (premier slide), ou (-1,-1) si aucune. */
    Point first_step_toward_nearest_reachable_energy(const Point& head,
            const std::set<Point>& my_body_set, const Point& my_tail,
            const std::set<Point>& others_body, const std::set<Point>& energy) const {
        std::set<Point> blocked = blocked_cells(my_body_set, my_tail, others_body);
        std::set<Point> solid = solid_cells(my_body_set, others_body, energy);
        std::map<Point, int> dist;
        std::map<Point, Point> parent;
        std::map<Point, Point> parent_direction;
        bfs_with_gravity(head, blocked, solid, dist, parent, &parent_direction);

        Point best_energy;
        int best_dist = std::numeric_limits<int>::max();
        for (const Point& e : energy) {
            auto it = dist.find(e);
            if (it == dist.end()) continue;
            if (it->second < best_dist) {
                best_dist = it->second;
                best_energy = e;
            }
        }
        if (best_dist == std::numeric_limits<int>::max()) return Point(-1, -1);
        if (best_dist == 0) return Point(0, 0);  // déjà sur l'énergie

        Point cur = best_energy;
        while (parent.count(cur) && parent.at(cur) != head) {
            cur = parent.at(cur);
        }
        // cur = première cellule après head (après un slide) ; direction = parent_direction[cur]
        if (parent.count(cur) && parent.at(cur) == head && parent_direction.count(cur))
            return parent_direction.at(cur);
        return Point(-1, -1);
    }

    /** Recalcule les actions possibles et remplit `actions` pour les miens.
     *  Chaque snakebot vise l’énergie atteignable la plus proche (sans murs ni autres snakes). */
    void recalculate_possible_actions(const std::set<Point>& energy) {
        clear_actions();

        for (const Snakebot& bot : my_snakebots) {
            if (bot.empty()) continue;

            const std::vector<Point>& body = bot.body;
            Point head = body[0];
            Point tail = body.back();
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

            Point step_dir = first_step_toward_nearest_reachable_energy(
                head, my_body_set, tail, others_body, energy);

            std::string best_dir = cur_dir;
            bool found = false;
            // step_dir = vecteur unitaire (UP/DOWN/LEFT/RIGHT) ou (-1,-1) si aucune cible
            if (std::abs(step_dir.x) + std::abs(step_dir.y) == 1) {
                best_dir = point_to_dir(step_dir);
                found = true;
            }

            if (!found) {
                std::vector<std::string> dirs = {UP, DOWN, LEFT, RIGHT};
                for (const std::string& d : dirs) {
                    Point delta = dir_to_point(d);
                    Point next_head = head + delta;
                    if (!in_bounds(next_head)) continue;
                    if (is_platform(next_head.x, next_head.y)) continue;
                    if (my_body_set.count(next_head) && next_head != tail) continue;
                    if (others_body.count(next_head)) continue;
                    best_dir = d;
                    found = true;
                    break;
                }
            }

            last_direction[bot.id] = best_dir;
            if (found) {
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
