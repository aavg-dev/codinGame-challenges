#include <cmath>
#include <iostream>
#include <ostream>
#include <string>
#include <tuple>
#include <vector>
#include <algorithm>
#include <list>
#include <memory>
#include <map>
#include <math.h>
#include <chrono>


using namespace std;
/**
* Auto-generated code below aims at helping you parse
* the standard input according to the problem statement.
**/
//TODO : Verify destination is navigable
//TODO: Verify recycler tyles --> need to improve
struct Position {
    Position() :x(0), y(0) {
    }
    Position(int x_coord, int y_coord) {
        x = x_coord;
        y = y_coord;
    }
    Position(const Position& p) {
        x = p.x;
        y = p.y;
    }

    friend bool operator!=(const Position& lhs, const Position& rhs) { return !(lhs == rhs); }
    friend bool operator==(const Position& lhs, const Position& rhs) { return (lhs.x == rhs.x && lhs.y == rhs.y); }
    friend Position operator+(const Position& lhs, const Position& rhs) { return Position(lhs.x + rhs.x, lhs.y + rhs.y); }
    Position& operator+=(const Position& rhs) {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }


    int x = 0;
    int y = 0;
    int distanceTo(const Position& pos) const {
        return sqrt(pow((pos.x - x), 2.0) + pow((pos.y - y), 2.0));
    }
};
string to_string(const Position& p) {
    return (to_string(p.x) + " " + (to_string(p.y)));
}
string to_string(std::shared_ptr<Position> p) {
    return (to_string(p->x) + " " + (to_string(p->y)));
}


/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

int main()
{
    int w; // width of the building.
    int h; // height of the building.
    cin >> w >> h; cin.ignore();
    int n; // maximum number of turns before game over.
    cin >> n; cin.ignore();
    int x0;
    int y0;
    cin >> x0 >> y0; cin.ignore();

    // game loop
    Position lowest_pos = Position(0, 0);
    Position highest_pos = Position(w - 1, h - 1);
    Position batmans_pos = Position(x0,y0); //To calculate the initial pos we will go to 
    //cout << to_string(batmans_pos) << endl;

    cerr << "width of the building is " << w << endl;
    cerr << "height of the building is " << h << endl;


    while (1) {
        string bomb_dir; // the direction of the bombs from batman's current location (U, UR, R, DR, D, DL, L or UL)
        cin >> bomb_dir; cin.ignore();
        cerr << "bomb_dir is " << bomb_dir << endl;

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;
        Position movement_indicator = Position(0, 0);
        
        //Map
        //DOWN +1 Y , UP --> -1 Y ,  
        //RIGHT --> +1 X, LEFT -1 X
        if (bomb_dir == "U") {
            highest_pos.y = batmans_pos.y -1; //UP
        } else if (bomb_dir == "UR") {
            highest_pos.y= batmans_pos.y -1; //UP
            lowest_pos.x = batmans_pos.x +1; //RIGHT
        } else if (bomb_dir == "R") {
            lowest_pos.x = batmans_pos.x +1; //RIGHT
        } else if (bomb_dir == "DR") {
            lowest_pos.y = batmans_pos.y +1; //DOWN
            lowest_pos.x = batmans_pos.x + 1; //RIGHT
        } else if (bomb_dir == "D") {
            lowest_pos.y = batmans_pos.y + 1;  //DOWN
        } else if (bomb_dir == "DL") {
            lowest_pos.y = batmans_pos.y + 1; //DOWN
            highest_pos.x = batmans_pos.x - 1; //LEFT
        } else if (bomb_dir == "L") {
            highest_pos.x = batmans_pos.x - 1; //LEFT
        } else if (bomb_dir == "UL") {
            highest_pos.y = batmans_pos.y - 1; //UP
            highest_pos.x = batmans_pos.x -1; //LEFT
        } else  {
            cerr << "Invalid direction for bomb dir, check" << endl;
        }
        cerr << "Lowest post " << to_string(lowest_pos) << endl;
        cerr << "Highest post " << to_string(highest_pos) << endl;


        cerr << "Batman initial pos is " << to_string(batmans_pos) << endl;
        batmans_pos = Position((lowest_pos.x + highest_pos.x)/2 , (lowest_pos.y + highest_pos.y) / 2);
        cerr << "Batman final pos is " << to_string(batmans_pos) << endl;

        // the location of the next window Batman should jump to.
        cout << to_string(batmans_pos) << endl;
    }
}