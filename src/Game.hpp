#ifndef GAME_HPP
#define GAME_HPP

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <set>
#include <queue>
#include <limits>


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

    /** BFS depuis start en tenant compte de la gravité : on ne peut aller que sur des cases supportées.
     *  Évite blocked, n’ajoute un voisin que s’il est supporté (sol ou solide en dessous). */
    void bfs_with_gravity(const Point& start, const std::set<Point>& blocked,
                         const std::set<Point>& solid,
                         std::map<Point, int>& dist, std::map<Point, Point>& parent) const {
        dist.clear();
        parent.clear();
        dist[start] = 0;
        std::queue<Point> q;
        q.push(start);
        static const std::vector<Point> deltas = {{0,-1},{0,1},{-1,0},{1,0}};
        while (!q.empty()) {
            Point cur = q.front();
            q.pop();
            int d = dist[cur];
            for (const Point& delta : deltas) {
                Point n = cur + delta;
                if (!in_bounds(n)) continue;
                if (blocked.count(n)) continue;
                if (dist.count(n)) continue;
                if (!is_supported(n, solid)) continue;  // gravité : pas de case en l’air
                dist[n] = d + 1;
                parent[n] = cur;
                q.push(n);
            }
        }
    }

    /** Trouve l’énergie atteignable la plus proche en tenant compte de la gravité
     *  (sans traverser murs ni autres snakes, uniquement sur cases supportées).
     *  Retourne la première étape depuis head vers cette énergie, ou point invalide si aucune. */
    Point first_step_toward_nearest_reachable_energy(const Point& head,
            const std::set<Point>& my_body_set, const Point& my_tail,
            const std::set<Point>& others_body, const std::set<Point>& energy) const {
        std::set<Point> blocked = blocked_cells(my_body_set, my_tail, others_body);
        std::set<Point> solid = solid_cells(my_body_set, others_body, energy);
        std::map<Point, int> dist;
        std::map<Point, Point> parent;
        bfs_with_gravity(head, blocked, solid, dist, parent);

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

        if (best_dist == 0) return head;
        Point cur = best_energy;
        while (parent.count(cur) && parent.at(cur) != head) {
            cur = parent.at(cur);
        }
        return (parent.count(cur) && parent.at(cur) == head) ? cur : Point(-1, -1);
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

            Point next_cell = first_step_toward_nearest_reachable_energy(
                head, my_body_set, tail, others_body, energy);

            std::string best_dir = cur_dir;
            bool found = false;
            if (next_cell.x >= 0 && next_cell.y >= 0) {
                Point delta(next_cell.x - head.x, next_cell.y - head.y);
                std::string dir = point_to_dir(delta);
                Point test = head + dir_to_point(dir);
                if (test.x == next_cell.x && test.y == next_cell.y) {
                    best_dir = dir;
                    found = true;
                }
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
