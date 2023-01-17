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
    int mId = 0;
    Position mPos = {};

};

class Zombie {
public:
    Zombie(int x, int y, int id,int nextX, int nextY){
        mId = id;
        mPos.x = x;
        mPos.y = y;
        mNextPos.x = nextX;
        mNextPos.y = nextY;
    }
    int mId = 0;
    Position mPos = {};
    Position mNextPos = {};
    int mSpeed = 400;
    int mRange = 400;

};

class Threat {
public:
    Threat( int distance,Zombie z, Human h){
        mDistance = distance;
        mZombie=z;
        mHuman = h;
    }
    int mDistance;
    Zombie mZombie;
    Human mHuman;

}

/***************     GLOBALS     ***************/

Player g_Player (0,0); 
vector<Human> g_HumanList;
vector<Zombie> g_ZombieList;


//Refresh them after every loop
void clearGlobals(){
    g_HumanList.clear();
    g_ZombieList.clear();
}

/*********************************************/



//A list of threats which is defined by a zombie, a human and the distance between them
std::list<Threat> getOrderedThreatsList(){

    int closest_zombie_distance = INT_MAX;
    //std::shared_ptr<Zombie> threatening = nullptr;
    //std::shared_ptr<Human> threatened = nullptr;


    for (auto human : g_HumanList){
        for (auto z : g_ZombieList){
            int new_distance = z.mNextPos.distanceTo(human.mPos);
            cerr << "zombie " << z.mId << " is at " << new_distance << " distance units from human " << human.mId << endl;
            if(new_distance < closest_zombie_distance){
                closest_zombie_distance = new_distance;
                threatening = std::make_shared<Zombie>(z);
                threatened = std::make_shared<Human>(human);
            }
        }
    }
    return std::make_pair(threatening, threatened);
}




std::shared_ptr<Zombie> getCloserZombieToMe(){

    int distance = INT_MAX;
    std::shared_ptr<Zombie> ret = nullptr;
    for (auto zombie : g_ZombieList){
        int new_distance = zombie.mNextPos.distanceTo(g_Player.mPos);
        if(new_distance < distance){
            distance = new_distance;
            ret = std::make_shared<Zombie>(zombie);
        }
    }
    return ret;
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

bool canReachDestinationInTime(Position human_pos, Zombie attacker, Player player){

    int turns_to_kill = attacker.mPos.distanceTo(human_pos) / 800; 
    int turns_to_save = player.mPos.distanceTo(human_pos) / 2000;
    if(turns_to_kill >  turns_to_save){
        return true;
    }
    return false;
}



int main()
{

    // game loop
    while (1) {
        int x;
        int y;
        g_Player = Player(x,y);
        clearGlobals();
        cin >> x >> y; cin.ignore();
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
        //std::shared_ptr<Human> in_danger_human = getMostThreateningZombie();
        std::pair<shared_ptr<Zombie>, shared_ptr<Human>> threat_pair = getOrderedThreatsList();
        std::shared_ptr<Zombie> dangerous_zombie = std::get<0>(threat_pair);
        std::shared_ptr<Human> threatened_human = std::get<1>(threat_pair);


        if(dangerous_zombie != nullptr){
            cerr << "dangerous_zombie at "<< to_string(dangerous_zombie->mPos) << endl;
            if(g_Player.mPos == dangerous_zombie->mPos){
                std::shared_ptr<Zombie> z = getCloserZombieToMe();
                if(z != nullptr){
                    cout << to_string(z->mNextPos) << endl;
                }
            } else {
                cout << to_string(dangerous_zombie->mNextPos) << endl;
            }

        } else {
            cerr << "Cannot find a threatened human "<<  endl;

            std::shared_ptr<Zombie> z = getCloserZombieToMe();
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