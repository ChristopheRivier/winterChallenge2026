#ifndef GAME_HPP
#define GAME_HPP

#include <string>
#include <sstream>
#include <vector>
#include <map>
#include <set>

#include <limits>
#include <functional>
#include <algorithm>


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

/** Serpent : coordonnées de la tête (body[0]) et du corps (body[1..n]). */
class Snake {
public:
    std::vector<Point> body;

    Snake() = default;
    explicit Snake(const std::vector<Point>& body) : body(body) {}

    bool empty() const { return body.empty(); }
    Point head() const { return body.empty() ? Point() : body[0]; }
    Point tail() const { return body.empty() ? Point() : body.back(); }
    void move_head(const Point& p) {
        body.insert(body.begin(), p);
        body.pop_back();
    }
    void move(const Point& delta) {
        body.insert(body.begin(), head() + delta);
        body.pop_back();
    }
    void snake_down() {
        for (auto it = body.begin(); it != body.end(); it++) {
            it->y++;
        }
    }
    /** Ensemble des cases occupées par le corps (tête incluse). */
    std::set<Point> body_set() const {
        return std::set<Point>(body.begin(), body.end());
    }
};

struct Snakebot {
    int id;
    std::vector<Point> body;
    std::string last_direction;

    Snakebot() : id(-1), last_direction(RIGHT) {}
    Snakebot(int id, const std::vector<Point>& body, const std::string& last_dir = RIGHT)
        : id(id), body(body), last_direction(last_dir) {}

    bool empty() const { return body.empty(); }
    Point head() const { return body.empty() ? Point() : body[0]; }
    /** Vue du bot comme Snake (tête + corps). */
    Snake as_snake() const { return Snake(body); }
};



inline std::vector<Point> parse_body(const std::string& body) {
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

inline Point dir_to_point(const std::string& d) {
    if (d == UP) return Point(0, -1);
    if (d == DOWN) return Point(0, 1);
    if (d == LEFT) return Point(-1, 0);
    return Point(1, 0);
}

inline std::string point_to_dir(const Point& p) {
    if (p.x == 0 && p.y == -1) return UP;
    if (p.x == 0 && p.y == 1) return DOWN;
    if (p.x == -1 && p.y == 0) return LEFT;
    if (p.x == 1 && p.y == 0) return RIGHT;
    return RIGHT;
}

inline int manhattan(const Point& a, const Point& b) {
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

    /** True si au moins un segment repose sur le sol, un mur, une énergie ou un corps (support en dessous). */
    bool has_platform_contact(const Snake& path_snake, const std::set<Point>& energy,
                              const std::set<Point>& others_body) const {
        std::set<Point> my_seg(path_snake.body.begin(), path_snake.body.end());
        if(energy.count(path_snake.head())) return true;
        for (const Point& p : path_snake.body) {
            if (p.y == height - 1) return true;
            Point below(p.x, p.y + 1);
            if (!in_bounds(below)) continue;
            if (is_platform(below.x, below.y)) return true;
            if (energy.count(below)) return true;
            if (others_body.count(below)) return true;
            //if (my_seg.count(below)) return true;
        }
        return false;
    }

    /** Déplace la tête d'une case dans la direction delta. S'arrête (n'entre pas) si la case
     *  est un mur (blocked) ou si le snake n'est plus sur un mur (case non supportée). */
    bool can_step(Point cur, const Point& delta, const std::set<Point>& blocked,
                  Point& out_next) const {
        Point n = cur + delta;
        if (!in_bounds(n)) return false;
        if (blocked.count(n)) return false;  // un mur arrête le mouvement
        out_next = n;
        return true;
    }

    /** BFS récursif depuis la tête du snake avec gravité : on bouge le snake case par case.
     *  À chaque pas, le chemin (path_snake) doit garder au moins un segment en contact avec un mur.
     *  parent_direction[n] = premier delta (depuis la tête de départ) sur le chemin optimal vers n. */
    void bfs_with_gravity(const Snake& snake, const std::set<Point>& blocked,
                         const std::set<Point>& energy, const std::set<Point>& others_body,
                         std::map<Point, int>& dist, std::map<Point, Point>& parent,
                         std::map<Point, Point>* parent_direction = nullptr) const {
        dist.clear();
        parent.clear();
        if (parent_direction) parent_direction->clear();
        if (snake.empty()) return;
        static const std::vector<Point> deltas = {{1,0},{0,1},{-1,0},{0,-1}};
        //static const std::vector<Point> deltas = {{-1,0}};

        std::function<void(Snake, int, Point)> rec;
        rec = [&](Snake path_snake, int d, Point first_delta_on_path) {
            Point cur = path_snake.head();
            auto it = dist.find(cur);
            if (it != dist.end() && it->second <= d) return;
            if(d > 10) return;
            dist[cur] = d;
            if (energy.count(cur)) {
            /*    std::cout << "energy found at: " << cur.x << ", " << cur.y << " d: " << d << std::endl;
                std::cout << "first_delta_on_path: " << first_delta_on_path.x << ", " << first_delta_on_path.y << std::endl;
                if (parent_direction) {
                    std::cout << "parent_direction: " << parent_direction->at(cur).x << ", " << parent_direction->at(cur).y << std::endl;
                }else{
                    std::cout << "parent_direction: null" << std::endl;
                }*/
                return;
            }
            if (!has_platform_contact(path_snake, energy, others_body)){
                return;
            }
            Point n;
            for (const Point& delta : deltas) {
                if (!can_step(cur, delta, blocked, n)) continue;
                Snake next_snake;
                next_snake.body = path_snake.body;
                next_snake.move(delta);
                if (!has_platform_contact(next_snake, energy, others_body)) {
                    while (!has_platform_contact(next_snake, energy, others_body)) {
                        next_snake.snake_down();
                    }
                }
                Point landing = next_snake.head();
                int nd = d + 1;
                Point first_for_path = (d == 0) ? delta : first_delta_on_path;
                auto nit = dist.find(landing);
                if (nit == dist.end() || nit->second > nd) {
                    parent[landing] = cur;
                    if (parent_direction) (*parent_direction)[landing] = first_for_path;
                    rec(next_snake, nd, first_for_path);
                }
            }
        };
        Snake start_path;
        start_path.body = snake.body;
        rec(start_path, 0, Point(0, 0));
    }

    /** Trouve l’énergie atteignable la plus proche en tenant compte de la gravité
     *  (sans traverser murs ni autres snakes, uniquement sur cases supportées).
     *  Retourne le vecteur unitaire de la direction à prendre (premier slide), ou (-1,-1) si aucune. */
    Point first_step_toward_nearest_reachable_energy(const Snake& my_snake,
            const std::set<Point>& others_body, const std::set<Point>& energy) const {
        if (my_snake.empty()) return Point(-1, -1);
        std::set<Point> my_body_set = my_snake.body_set();
        Point head = my_snake.head();
        Point my_tail = my_snake.tail();
        std::set<Point> blocked = blocked_cells(my_body_set, my_tail, others_body);
        std::map<Point, int> dist;
        std::map<Point, Point> parent;
        std::map<Point, Point> parent_direction;
        bfs_with_gravity(my_snake, blocked, energy, others_body, dist, parent, &parent_direction);

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
     *  Chaque snakebot vise l’énergie atteignable la plus proche (sans murs ni autres snakes).
     *  Puis on évite que deux alliés visent la même case sur le premier pas (id plus petit prioritaire). */
    void recalculate_possible_actions(const std::set<Point>& energy) {
        clear_actions();

        struct Plan {
            int id;
            Point head;
            Point tail;
            std::set<Point> my_body_set;
            std::set<Point> others_body;
            std::string best_dir;
            bool found;
        };
        std::vector<Plan> plans;

        for (const Snakebot& bot : my_snakebots) {
            if (bot.empty()) continue;

            Snake snake = bot.as_snake();
            Point head = snake.head();
            Point tail = snake.tail();
            std::string cur_dir = last_direction.count(bot.id) ? last_direction[bot.id] : RIGHT;
            std::set<Point> my_body_set = snake.body_set();

            std::set<Point> others_body;
            for (const Snakebot& other : my_snakebots) {
                if (other.id == bot.id) continue;
                for (const Point& p : other.body) others_body.insert(p);
            }
            for (const Snakebot& other : opp_snakebots) {
                for (const Point& p : other.body) others_body.insert(p);
            }

            Point step_dir = first_step_toward_nearest_reachable_energy(
                snake, others_body, energy);

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

            /* Même case qu’au tour précédent : ne pas renvoyer la même direction (évite boucle). */
            if (found && last_head_position.count(bot.id) && head == last_head_position[bot.id]
                && best_dir == cur_dir) {
                std::vector<std::string> dirs = {UP, DOWN, LEFT, RIGHT};
                for (const std::string& d : dirs) {
                    if (d == best_dir) continue;
                    Point delta = dir_to_point(d);
                    Point next_head = head + delta;
                    if (!in_bounds(next_head)) continue;
                    if (is_platform(next_head.x, next_head.y)) continue;
                    if (my_body_set.count(next_head) && next_head != tail) continue;
                    if (others_body.count(next_head)) continue;
                    best_dir = d;
                    break;
                }
            }

            Plan p;
            p.id = bot.id;
            p.head = head;
            p.tail = tail;
            p.my_body_set = std::move(my_body_set);
            p.others_body = std::move(others_body);
            p.best_dir = best_dir;
            p.found = found;
            plans.push_back(std::move(p));
        }

        std::sort(plans.begin(), plans.end(), [](const Plan& a, const Plan& b) {
            return a.id < b.id;
        });

        auto valid_first_head = [&](const Plan& pl, const Point& nh) -> bool {
            if (!in_bounds(nh)) return false;
            if (is_platform(nh.x, nh.y)) return false;
            if (pl.my_body_set.count(nh) && nh != pl.tail) return false;
            if (pl.others_body.count(nh)) return false;
            return true;
        };

        std::set<Point> claimed_next;
        static const std::vector<std::string> kAllDirs = {UP, DOWN, LEFT, RIGHT};
        for (Plan& p : plans) {
            if (!p.found) continue;
            Point nh = p.head + dir_to_point(p.best_dir);
            if (valid_first_head(p, nh) && !claimed_next.count(nh)) {
                claimed_next.insert(nh);
                continue;
            }
            std::vector<std::string> try_order;
            try_order.push_back(p.best_dir);
            for (const std::string& d : kAllDirs) {
                if (d != p.best_dir) try_order.push_back(d);
            }
            bool placed = false;
            for (const std::string& d : try_order) {
                Point cand = p.head + dir_to_point(d);
                if (!valid_first_head(p, cand)) continue;
                if (claimed_next.count(cand)) continue;
                p.best_dir = d;
                claimed_next.insert(cand);
                placed = true;
                break;
            }
            if (!placed) {
                for (const std::string& d : try_order) {
                    Point cand = p.head + dir_to_point(d);
                    if (!valid_first_head(p, cand)) continue;
                    p.best_dir = d;
                    if (!claimed_next.count(cand)) claimed_next.insert(cand);
                    break;
                }
            }
        }

        for (Plan& p : plans) {
            last_head_position[p.id] = p.head;
            last_direction[p.id] = p.best_dir;
            if (p.found) {
                actions.push_back(std::to_string(p.id) + " " + p.best_dir);
            }
        }

        if (actions.empty()) {
            actions.push_back("WAIT");
        }
    }

    /** Dernière direction envoyée par snakebot id (pour éviter les demi-tours). */
    std::map<int, std::string> last_direction;
    /** Tête au tour précédent (pour détecter l’immobilité et varier la direction). */
    std::map<int, Point> last_head_position;
};




#endif
