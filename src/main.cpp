#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <set>
#include <map>
#include <sstream>
#include <cmath>
#include "Game.hpp"

using namespace std;


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

    Game g;
    cin >> g.my_id; cin.ignore();
    cin >> g.width; cin.ignore();
    cin >> g.height; cin.ignore();
    if (debug) {
        cerr << "my_id=" << g.my_id << " width=" << g.width << " height=" << g.height << endl;
    }
    g.grid.resize(g.height);
    for (int i = 0; i < g.height; i++) {
        getline(cin, g.grid[i]);
        if (debug) cerr << "row " << i << ": " << g.grid[i] << endl;
    }
    int snakebots_per_player;
    cin >> snakebots_per_player; cin.ignore();
    if (debug) cerr << "snakebots_per_player=" << snakebots_per_player << endl;

    vector<int> my_snakebot_ids(snakebots_per_player);
    vector<int> opp_snakebot_ids(snakebots_per_player);
    for (int i = 0; i < snakebots_per_player; i++) {
        cin >> my_snakebot_ids[i]; cin.ignore();
        g.last_direction[my_snakebot_ids[i]] = UP;
        if (debug) cerr << "my_snakebot_id=" << my_snakebot_ids[i] << endl;
    }
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

        g.clear_snakebots();
        for (int i = 0; i < snakebot_count; i++) {
            int snakebot_id;
            string body_str;
            cin >> snakebot_id >> body_str; cin.ignore();
            vector<Point> body = parse_body(body_str);
            string last_dir = g.last_direction.count(snakebot_id) ? g.last_direction[snakebot_id] : UP;
            Snakebot bot(snakebot_id, body, last_dir);
            if (find(my_snakebot_ids.begin(), my_snakebot_ids.end(), snakebot_id) != my_snakebot_ids.end()) {
                g.my_snakebots.push_back(bot);
            } else {
                g.opp_snakebots.push_back(bot);
            }
            if (debug) cerr << "  snakebot_id=" << snakebot_id << " body=" << body_str << endl;
        }

        g.recalculate_possible_actions(energy);
        for (size_t i = 0; i < g.actions.size(); i++) {
            cout << g.actions[i] << ";";
        }
        cout << endl;
    }
}
