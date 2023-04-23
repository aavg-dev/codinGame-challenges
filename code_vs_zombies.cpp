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
#include <set>

using namespace std;

#define INT_MAX 2147483647
/**
 * Save humans, destroy zombies!
 **/

struct Position {
    Position():x(0), y(0){
    }
    Position(int x_coord, int y_coord){
        x = x_coord;
        y = y_coord;
    }
    Position(const Position& p){
        x = p.x;
        y = p.y;
    }
    friend bool operator!=(const Position& lhs, const Position& rhs) { return !(lhs == rhs);}
    friend bool operator==(const Position& lhs, const Position& rhs) { return (lhs.x == rhs.x && lhs.y == rhs.y);}
    friend Position operator+(const Position& lhs, const Position& rhs) { return Position (lhs.x + rhs.x, lhs.y + rhs.y); }
    int x = 0;
    int y = 0;
    int distanceTo(const Position& pos) const {
        return sqrt(pow((pos.x - x),2.0) + pow((pos.y - y),2.0));
    }
};
string to_string(const Position& p){
    return (to_string(p.x) + " " + (to_string(p.y)));
}
string to_string(std::shared_ptr<Position> p){
    return (to_string(p->x) + " " + (to_string(p->y)));
}

class Player {

    public:
    Player(int x, int y){
        mPos.x = x;
        mPos.y = y; 
    }
    Position mPos = {};
    int mSpeed = 1000;
    int mRange = 1000;
};

class Human {
public:
    Human( int id, int x, int y){
        mId = id;
        mPos.x = x;
        mPos.y = y;
    }
    Human(const Human& h){
        mId = h.mId;
        mPos = h.mPos;
    }
    int mId = 0;
    Position mPos = {};

};
string to_string(const Human& h){
    return ("Id: " + to_string(h.mId) + " Pos:" + to_string(h.mPos.x) + "," + (to_string(h.mPos.y)));
}

class Zombie {
public:
    Zombie(int id,int x, int y, int nextX, int nextY){
        mId = id;
        mPos.x = x;
        mPos.y = y;
        mNextPos.x = nextX;
        mNextPos.y = nextY;
    }
    Zombie(const Zombie& z){
        mId = z.mId;
        mPos = z.mPos;
        mNextPos = z.mNextPos;
    }
    int mId = 0;
    Position mPos = {};
    Position mNextPos = {};
    int mSpeed = 400;
    int mRange = 400;

};

string to_string(const Zombie& z){
    return ("Id: " + to_string(z.mId) + " Pos:" + to_string(z.mPos.x) + "," + (to_string(z.mPos.y)));
}

class Threat {
public:
    Threat( int distance,Zombie z, Human h): mDistance(distance), mZombie(z), mHuman(h){ }
    friend bool operator<(const Threat& lhs, const Threat& rhs) {return lhs.mDistance<rhs.mDistance;}

    int mDistance;
    Zombie mZombie;
    Human mHuman;

};

/***************     GLOBALS     ***************/

static Player g_Player (0,0); 
static vector<Human> g_HumanList;
static vector<Zombie> g_ZombieList;


//Refresh them after every loop
void clearGlobals(){
    g_HumanList.clear();
    g_ZombieList.clear();
}

/*********************************************/

std::shared_ptr<Zombie> getCloserZombieToPos(Position pos);
std::multiset<Threat> getOrderedThreatsList();
std::shared_ptr<Human> getCloserHumanToMe();


//A list of threats which is defined by a zombie, a human and the distance between them
std::multiset<Threat> getOrderedThreatsList(){

    int closest_zombie_distance = INT_MAX;
    std::multiset<Threat> ret;
    for (auto human : g_HumanList){
        std::shared_ptr<Zombie> z = getCloserZombieToPos(human.mPos);
        if(z != nullptr){
            ret.insert(Threat( z->mNextPos.distanceTo(human.mPos),*z, human));
        }          
    }
    return ret;
}


std::shared_ptr<Zombie> getCloserZombieToPos(Position pos){

    int distance = INT_MAX;
    std::shared_ptr<Zombie> ret = nullptr;
    for (auto zombie : g_ZombieList){
        int new_distance = zombie.mNextPos.distanceTo(pos);
        if(new_distance < distance){
            distance = new_distance;
            ret = std::make_shared<Zombie>(zombie);
        }
    }
    return ret;
}

std::shared_ptr<Zombie> getCloserZombieToMe(){
    return getCloserZombieToPos(g_Player.mPos);
}

std::shared_ptr<Human> getCloserHumanToMe(){

    int distance = INT_MAX;
    std::shared_ptr<Human> ret = nullptr;
    for (auto human : g_HumanList){
        int new_distance = human.mPos.distanceTo(g_Player.mPos);
        if(new_distance < distance){
            distance = new_distance;
            ret = std::make_shared<Human>(human);
        }
    }
    return ret;
}

//Calculate how much turns:
//1. the zombie needs to get to the human
//2. the player needs to get to the zombie next pos !!
bool canReachDestinationInTime(Position human_pos, Zombie zombie, Position player_pos){
    cerr << "human_pos " << to_string(human_pos) << ", zombie_pos " << to_string(zombie.mPos) << ", player_pos " << to_string(player_pos) << endl;

    //TODO: Should it be float ? Do we need that precise comparison?
    int turns_to_kill = zombie.mPos.distanceTo(human_pos) / 800; 
    int turns_to_save = player_pos.distanceTo(zombie.mNextPos) / 2000;
    cerr << "Turns needed to kill human :"<< turns_to_kill << endl;
    cerr << "Turns needed to save human :"<< turns_to_save << endl;

    if(turns_to_kill+1 >=  turns_to_save){ //+1 as the zombie kills at the end of the turn, we kill before
        return true;
    }
    return false;
}

std::shared_ptr<Position> getReachableThreadDestination(){

    std::multiset threats = getOrderedThreatsList();
    if(threats.size() != 0){
        for ( auto threats_it = threats.begin(); threats_it != threats.end(); threats_it++ ){
            cerr << "dangerous_zombie at "<< to_string(threats_it->mZombie.mPos) << endl;

            if (canReachDestinationInTime(threats_it->mHuman.mPos, threats_it->mZombie,g_Player.mPos)){
                cerr << "Can reach zombie [" << to_string(threats_it->mZombie) << "] in time, let's go "<< endl;
                return make_shared<Position>(threats_it->mZombie.mNextPos);
                //cout << to_string(dangerous_zombie->mNextPos) << endl;
            } else {
                cerr << "Discarding zombie " << to_string(threats_it->mZombie) << ", we would never save the human"<< endl;

            }
        }
    }
    return nullptr;
}


int main()
{

    // game loop
    while (1) {
        int x;
        int y;
        cin >> x >> y; cin.ignore();
        clearGlobals();
        g_Player.mPos = Position(x,y);
        int human_count;
        cin >> human_count; cin.ignore();
        for (int i = 0; i < human_count; i++) {
            int human_id;
            int human_x;
            int human_y;
            cin >> human_id >> human_x >> human_y; cin.ignore();
            Human h(human_id, human_x, human_y);
            g_HumanList.push_back(h);
        }
        int zombie_count;
        cin >> zombie_count; cin.ignore();
        for (int i = 0; i < zombie_count; i++) {
            int zombie_id;
            int zombie_x;
            int zombie_y;
            int zombie_xnext;
            int zombie_ynext;
            cin >> zombie_id >> zombie_x >> zombie_y >> zombie_xnext >> zombie_ynext; cin.ignore();
            Zombie z(zombie_id, zombie_x, zombie_y,zombie_xnext,zombie_ynext );
            g_ZombieList.push_back(z);

        }
        cerr << "g_ZombieList size  "<< g_ZombieList.size() << endl;
        cerr << "g_HumanList size  "<< g_HumanList.size() << endl;

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;
       
        //TODO: Create a threshold to kill nearby enemies if needed

        std::shared_ptr<Position> next_destination = getReachableThreadDestination();
        if(next_destination != nullptr){
            cout << to_string(next_destination) << endl;
        } else {
            cerr << "Could not find a human to save "<<  endl;
            std::shared_ptr<Zombie> z = getCloserZombieToMe();;
            if(z != nullptr){
                cout << to_string(z->mNextPos) << endl;
            } else {
                std::shared_ptr<Human> h = getCloserHumanToMe();
                if(h != nullptr){
                    cout << to_string(h->mPos) << endl;
                } else {
                    cerr << "Something is bad!" << endl;
                    cout << "0 0" << endl; // Your destination coordinates
                }
            }
        }


    }
}