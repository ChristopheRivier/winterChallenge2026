#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <map>
#include <sstream>
#include <cmath>

using namespace std;

const string UP = "UP";
const string DOWN = "DOWN";
const string LEFT = "LEFT";
const string RIGHT = "RIGHT";

int my_id;
int width, height;
vector<string> grid;
vector<int> my_snakebot_ids;
vector<int> opp_snakebot_ids;
map<int, string> last_direction; // last direction sent per snakebot id

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

bool in_bounds(const Point& p) {
    return p.x >= 0 && p.x < width && p.y >= 0 && p.y < height;
}

bool is_platform(int x, int y) {
    if (x < 0 || x >= width || y < 0 || y >= height) return false;
    char c = grid[y][x];
    return c == '#' || c == '1' || c == 'X';
}

vector<Point> parse_body(const string& body) {
    vector<Point> segs;
    string s = body;
    for (char& c : s) if (c == ';' || c == ':') c = ' ';
    istringstream iss(s);
    string token;
    while (iss >> token) {
        size_t comma = token.find(',');
        if (comma != string::npos) {
            int x = stoi(token.substr(0, comma));
            int y = stoi(token.substr(comma + 1));
            segs.push_back(Point(x, y));
        }
    }
    return segs;
}

Point dir_to_point(const string& d) {
    if (d == UP) return Point(0, -1);
    if (d == DOWN) return Point(0, 1);
    if (d == LEFT) return Point(-1, 0);
    if (d == RIGHT) return Point(1, 0);
    return Point(0, -1);
}

string point_to_dir(const Point& p) {
    if (p.x == 0 && p.y == -1) return UP;
    if (p.x == 0 && p.y == 1) return DOWN;
    if (p.x == -1 && p.y == 0) return LEFT;
    if (p.x == 1 && p.y == 0) return RIGHT;
    return UP;
}

int manhattan(const Point& a, const Point& b) {
    return abs(a.x - b.x) + abs(a.y - b.y);
}

int main(int argc, char* argv[])
{
    bool debug = true;
    int max_turns = -1;  // -1 = illimité (jeu normal)
    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "--max-turns" && i + 1 < argc) {
            max_turns = stoi(argv[++i]);
            break;
        }
    }

    cin >> my_id; cin.ignore();
    cin >> width; cin.ignore();
    cin >> height; cin.ignore();
    if (debug) {
        cerr << "my_id=" << my_id << " width=" << width << " height=" << height << endl;
    }
    grid.resize(height);
    for (int i = 0; i < height; i++) {
        getline(cin, grid[i]);
        if (debug) cerr << "row " << i << ": " << grid[i] << endl;
    }
    int snakebots_per_player;
    cin >> snakebots_per_player; cin.ignore();
    if (debug) cerr << "snakebots_per_player=" << snakebots_per_player << endl;
    my_snakebot_ids.resize(snakebots_per_player);
    for (int i = 0; i < snakebots_per_player; i++) {
        cin >> my_snakebot_ids[i]; cin.ignore();
        last_direction[my_snakebot_ids[i]] = UP;
        if (debug) cerr << "my_snakebot_id=" << my_snakebot_ids[i] << endl;
    }
    opp_snakebot_ids.resize(snakebots_per_player);
    for (int i = 0; i < snakebots_per_player; i++) {
        cin >> opp_snakebot_ids[i]; cin.ignore();
        if (debug) cerr << "opp_snakebot_id=" << opp_snakebot_ids[i] << endl;
    }

    int turn = 0;
    while (1) {
        if (max_turns >= 0 && turn >= max_turns) break;
        if (cin.eof()) break;
        int power_source_count;
        cin >> power_source_count;
        if (!cin) break;
        cin.ignore();
        turn++;
        set<Point> energy;
        if (debug) cerr << "power_source_count=" << power_source_count << endl;
        for (int i = 0; i < power_source_count; i++) {
            int x, y;
            cin >> x >> y; cin.ignore();
            energy.insert(Point(x, y));
            if (debug) cerr << "  energy " << x << " " << y << endl;
        }
        int snakebot_count;
        cin >> snakebot_count; cin.ignore();
        if (debug) cerr << "snakebot_count=" << snakebot_count << endl;
        map<int, vector<Point>> snakebots;
        set<Point> all_body_cells;
        for (int i = 0; i < snakebot_count; i++) {
            int snakebot_id;
            string body;
            cin >> snakebot_id >> body; cin.ignore();
            vector<Point> segs = parse_body(body);
            snakebots[snakebot_id] = segs;
            for (const auto& p : segs) all_body_cells.insert(p);
            if (debug) cerr << "  snakebot_id=" << snakebot_id << " body=" << body << endl;
        }

        vector<string> actions;
        for (int sid : my_snakebot_ids) {
            if (snakebots.find(sid) == snakebots.end()) continue;
            const vector<Point>& body = snakebots[sid];
            if (body.empty()) continue;
            Point head = body[0];
            string cur_dir = last_direction[sid];
            set<Point> my_body_set(body.begin(), body.end());
            set<Point> others_body;
            for (const auto& kv : snakebots) {
                if (kv.first == sid) continue;
                for (const auto& p : kv.second) others_body.insert(p);
            }

            string best_dir = cur_dir;
            int best_score = -2000000000;
            vector<string> dirs = {UP, DOWN, LEFT, RIGHT};
            for (const string& d : dirs) {
                Point delta = dir_to_point(d);
                Point next_head = head + delta;
                if (!in_bounds(next_head)) continue;
                if (is_platform(next_head.x, next_head.y)) continue;
                if (my_body_set.count(next_head) && next_head != body.back()) continue;
                if (others_body.count(next_head)) continue;

                int score = 0;
                if (energy.count(next_head)) score += 10000;
                int nearest = 1e9;
                for (const Point& e : energy) {
                    nearest = min(nearest, manhattan(next_head, e));
                }
                score -= nearest * 2;
                if (next_head.y >= height - 2) score -= 100;
                if (score > best_score) {
                    best_score = score;
                    best_dir = d;
                }
            }
            last_direction[sid] = best_dir;
            if (best_score > -2000000000)
                actions.push_back(to_string(sid) + " " + best_dir);
        }
        if (actions.empty()) actions.push_back("WAIT");
        for (size_t i = 0; i < actions.size(); i++) {
            cout << actions[i] << ";";
        }
        cout << endl;
    }
}
