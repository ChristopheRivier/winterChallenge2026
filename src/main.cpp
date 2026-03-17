#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

int main()
{
    int my_id;
    cin >> my_id; cin.ignore();
    int width;
    cin >> width; cin.ignore();
    int height;
    cin >> height; cin.ignore();
    for (int i = 0; i < height; i++) {
        string row;
        getline(cin, row);
    }
    int snakebots_per_player;
    cin >> snakebots_per_player; cin.ignore();
    for (int i = 0; i < snakebots_per_player; i++) {
        int my_snakebot_id;
        cin >> my_snakebot_id; cin.ignore();
    }
    for (int i = 0; i < snakebots_per_player; i++) {
        int opp_snakebot_id;
        cin >> opp_snakebot_id; cin.ignore();
    }

    // game loop
    while (1) {
        int power_source_count;
        cin >> power_source_count; cin.ignore();
        for (int i = 0; i < power_source_count; i++) {
            int x;
            int y;
            cin >> x >> y; cin.ignore();
        }
        int snakebot_count;
        cin >> snakebot_count; cin.ignore();
        for (int i = 0; i < snakebot_count; i++) {
            int snakebot_id;
            string body;
            cin >> snakebot_id >> body; cin.ignore();
        }

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;

        cout << "WAIT" << endl;
    }
}