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
/***********************************************/
/***************     TILES     ***************/
/*********************************************/
class Tile {
public:
    Tile(int x, int y, int scrap_amount,int owner, int units,int recycler,int can_build,int can_spawn, int in_range_of_recycler){
        mPos.x = x;
        mPos.y = y;
        mScrap_amount = scrap_amount;
        mOwner = owner;
        mUnits = units;
        mRecycler = recycler;
        mCan_build = can_build;
        mCan_spawn = can_spawn;
        mIn_range_of_recycler = in_range_of_recycler;
    }
    Position mPos = {};
    int mScrap_amount = -1;
    int mOwner = -2; // 1 = me, 0 = foe, -1 = neutral
    int mUnits = -1;
    int mRecycler = -1;
    int mCan_build = -1;
    int mCan_spawn = -1;
    int mIn_range_of_recycler = -1;
};
enum AreaDelimiterType {
    FRIENDLY_AREA_TOP,
    FRIENDLY_AREA_BOTTOM,
    FRIENDLY_AREA_MIDDLE,
    NEUTRAL_AREA_TOP,
    NEUTRAL_AREA_BOTTOM,
    NEUTRAL_AREA_MIDDLE,
    ENEMY_AREA_TOP,
    ENEMY_AREA_BOTTOM,
    ENEMY_AREA_MIDDLE
};
enum GlobalStrategy {
    EXPAND_TERRITORY,
    DEFENSIVE
};
enum MapSize {
    SMALL,
    MEDIUM,
    BIG,
    SUPER_BIG
};
/***************     GLOBALS     ***************/
vector<Tile> g_tileList;
vector<Tile> g_enemyTileList;
vector<Tile> g_friendlyTileList;
map<AreaDelimiterType, Position> g_areaDelimiterMap; // Do not clear unless we want to update it during game
class RobotEntity;
vector<RobotEntity> g_friendlyRobots;
vector<RobotEntity> g_enemyRobots;
static int g_turns = 1;
static int g_width = 99;
static int g_height = 99;
static MapSize g_map_size = SMALL;
static GlobalStrategy g_game_strategy =  EXPAND_TERRITORY;
//Refresh them after every loop
void clearGlobals(){
    g_tileList.clear();
    g_enemyTileList.clear();
    g_friendlyTileList.clear();
    g_friendlyRobots.clear();
    g_enemyRobots.clear();
}
/***************     Method declarations     ***************/
shared_ptr<Tile> getTileByPos(Position pos);
int getTileRecyclerRating(Tile tile);
std::shared_ptr<Position> getCloserEnemyTile(bool mustHaveEnemies);
std::shared_ptr<Position> getCloserFriendlyTile(bool must_be_spawnable, bool must_be_buildable);
std::vector<std::shared_ptr<Position>> getFriendlyTilesCloserToEnemy(bool must_be_spawnable, bool must_be_buildable, bool must_have_enemies);
shared_ptr<Position> getBestBuildPosition(vector<Tile> possibleSpots);
/***********************************************/
/***************     STRATEGY   ***************/
/*********************************************/
enum OwnerTypes{
    NEUTRAL = -1,
    ENEMY = 0,
    FRIENDLY = 1
};               
enum RobotBehaviourStrategy {
    SECURE_BEST_RECYCLER_TILES,
    CONQUER_FRIENDLY_AREA_TOP,
    CONQUER_FRIENDLY_AREA_BOTTOM,
    CONQUER_NEUTRAL_AREA_TOP,
    CONQUER_NEUTRAL_AREA_BOTTOM,
    CONQUER_ENEMY_AREA_TOP,
    CONQUER_ENEMY_AREA_BOTTOM,
    CONQUER_ANY_MAP_AREA,
    AGGRO_CONQUER_TILES,
    AGGRESIVE
};
enum StrategicPositionType{
    ENEMY_FARTHEST_TOP,
    ENEMY_FARTHEST_BOTTOM,
    ENEMY_FARTHEST_MIDDLE,
    ENEMY_CLOSER_TOP,
    ENEMY_CLOSER_BOTTOM,
    ENEMY_CLOSER_MIDDLE,
    NEUTRAL_FARTHEST_TOP,
    NEUTRAL_FARTHEST_BOTTOM,
    RECYCLER_OPTIMAL_CLOSER
};
struct StrategicPosition:  Position {
    StrategicPosition():x(0), y(0){
    }
    StrategicPosition(int x_coord, int y_coord){
        x = x_coord;
        y = y_coord;
    }
    int x = 0;
    int y = 0;
    StrategicPositionType type;
};
static double FRIENDLY_ROBOTS_PROPORTION = 0.3;
static double CONQUER_NEUTRAL_AREA_ROBOTS_PROPORTION = 0.5;
static double CONQUER_ENEMY_AREA_ROBOTS_PROPORTION = 0.2;
static double RECYCLER_SPOT_FINDERS_ROBOTS_PROPORTION = 0.0;
static double AGGRESIVE_ROBOT_PROPORTION =  0.0;
class RobotEntity {
public:
    RobotEntity(){
    }
    RobotEntity(int x, int y, int owner, int quantity,RobotBehaviourStrategy strat = CONQUER_ANY_MAP_AREA){
        mPos.x = x;
        mPos.y = y;
        mOwner = owner;
        mQuantity = quantity;
        mStrategy = strat;
    }
    RobotEntity(const RobotEntity& r){
        mPos.x = r.mPos.x;
        mPos.y = r.mPos.y;
        mOwner = r.mOwner;
        mQuantity = r.mQuantity;
        mStrategy = r.mStrategy;
    }
   
    Position mPos = {};
    Position mLastPos = {};
    int mOwner = 2; //2 is a invalid one
    int mQuantity = -1;
    RobotBehaviourStrategy mStrategy = CONQUER_ANY_MAP_AREA;
    friend bool operator==(const RobotEntity& lhs, const RobotEntity& rhs) { return (lhs.mPos == rhs.mPos);}
   
    std::vector<std::shared_ptr<Position>> getCloserNeutralTiles(bool mustBeNavigable = true) const{
        int distance = 99;
        std::vector<std::shared_ptr<Position>> neutral_pos_list;
   
        for (auto tile : g_tileList){
            std::shared_ptr<Position> pos;
            if(tile.mOwner != OwnerTypes::NEUTRAL){
                continue;
            }
            //Do not count it if there is no scrap, we cant navigate there
            if(mustBeNavigable && tile.mScrap_amount == 0){
                continue;
            }
            int new_distance= mPos.distanceTo(tile.mPos);
            if(new_distance < distance){
                distance = new_distance;
                neutral_pos_list.clear();//Otherwise we may include positions far away which have been detected before
                neutral_pos_list.push_back(std::make_shared<Position>(tile.mPos));
            } else if (new_distance == distance){
                neutral_pos_list.push_back(std::make_shared<Position>(tile.mPos));
            }
        }
        return neutral_pos_list;
    }
    std::shared_ptr<Position> getCloserEnemyTile(const bool mustHaveEnemies) const {
        int distance = 99;
        std::shared_ptr<Position> pos;
        for (auto tile : g_enemyTileList){
            if(mustHaveEnemies && tile.mUnits != 0){
                continue;
            }
            int new_distance = mPos.distanceTo(tile.mPos);
            if(new_distance < distance){
                distance = new_distance;
                pos = std::make_shared<Position>(tile.mPos);
            }
        }
        return pos;
    }
    std::vector<std::shared_ptr<Position>> getNextStrategyObjectives() const {
        //todo: always move even though implies changing strategy, late game looks bad
        std::vector<std::shared_ptr<Position>> objective_list = {};
        cerr << "getNextStrategyObjectives mStrategy " << mStrategy << endl;
        bool must_have_enemies = false;
        std::shared_ptr<Position> closer_enemy_tile = getCloserEnemyTile(must_have_enemies);
        must_have_enemies = true;
        std::shared_ptr<Position> closer_enemy_tile_with_robots = getCloserEnemyTile(must_have_enemies);
        std::shared_ptr<Position> closer_neutral_tile = nullptr;
        vector<shared_ptr<Position>> neutral_tiles = getCloserNeutralTiles(true);
        if(neutral_tiles.size() > 0){ 
            closer_neutral_tile = neutral_tiles[0];
        }       
        if(closer_enemy_tile == nullptr && closer_neutral_tile == nullptr)
        {
            return objective_list;
        }
        int distance_to_enemy = 99;
        if(closer_enemy_tile != nullptr){
            cerr << "closer_enemy_tile to " << to_string(mPos) << " is " << to_string(closer_enemy_tile) << endl;
            distance_to_enemy =  closer_enemy_tile->distanceTo(mPos);
        }
        int distance_to_neutral = 99;
        if(closer_neutral_tile != nullptr){
            cerr << "closer_neutral_tile to " << to_string(mPos) << " is " << to_string(closer_neutral_tile) << endl;
            distance_to_neutral =  closer_neutral_tile->distanceTo(mPos);
        }
        switch (mStrategy)
        {
            case RobotBehaviourStrategy::SECURE_BEST_RECYCLER_TILES:
            {
                return std::vector<std::shared_ptr<Position>> {getBestBuildPosition(g_tileList)};
            break;
            }
            case RobotBehaviourStrategy::CONQUER_FRIENDLY_AREA_TOP:
            {  
                if(g_turns <= 5 && this->mPos != g_areaDelimiterMap[AreaDelimiterType::FRIENDLY_AREA_TOP]){
                    objective_list.push_back( std::make_shared<Position>(Position(g_areaDelimiterMap[AreaDelimiterType::FRIENDLY_AREA_TOP])));
                } else {
                    //Go to enemy tile if closer
                    if(distance_to_enemy < distance_to_neutral){
                        if(closer_enemy_tile != nullptr){
                            objective_list.push_back(closer_enemy_tile);
                        }
                    }
                    return getCloserNeutralTiles(true);
                }
            break;
            }
            case RobotBehaviourStrategy::CONQUER_FRIENDLY_AREA_BOTTOM:
            {
                if(g_turns <= 5 && this->mPos != g_areaDelimiterMap[AreaDelimiterType::FRIENDLY_AREA_BOTTOM]){
                    objective_list.push_back(std::make_shared<Position>(Position(g_areaDelimiterMap[AreaDelimiterType::FRIENDLY_AREA_BOTTOM])));
                } else {
                    //Go to enemy tile if closer
                    if(distance_to_enemy < distance_to_neutral){
                        if(closer_enemy_tile != nullptr){
                            objective_list.push_back(closer_enemy_tile);
                        }
                    }
                    objective_list = getCloserNeutralTiles(true);
                    
                }
            break;
            }
            case RobotBehaviourStrategy::CONQUER_NEUTRAL_AREA_TOP:
           
                if(g_turns <= 5 && this->mPos != g_areaDelimiterMap[AreaDelimiterType::NEUTRAL_AREA_TOP]){
                    objective_list.push_back( std::make_shared<Position>(Position(g_areaDelimiterMap[AreaDelimiterType::NEUTRAL_AREA_TOP])));
                } else {
                    //Go to enemy tile if closer
                    if(distance_to_enemy < distance_to_neutral){
                        if(closer_enemy_tile != nullptr){
                            objective_list.push_back(closer_enemy_tile);
                        }
                    }
                    return getCloserNeutralTiles(true);
                }
            break;
           
            case RobotBehaviourStrategy::CONQUER_NEUTRAL_AREA_BOTTOM:
            {
                if(g_turns <= 5 && this->mPos != g_areaDelimiterMap[AreaDelimiterType::NEUTRAL_AREA_BOTTOM]){
                    objective_list.push_back(std::make_shared<Position>(Position(g_areaDelimiterMap[AreaDelimiterType::NEUTRAL_AREA_BOTTOM])));
                } else {
                    //Go to enemy tile if closer
                    if(distance_to_enemy < distance_to_neutral){
                        if(closer_enemy_tile != nullptr){
                            objective_list.push_back(closer_enemy_tile);
                        }
                    }
                    objective_list = getCloserNeutralTiles(true);
                    
                }
            break;
           
            }
            case RobotBehaviourStrategy::CONQUER_ENEMY_AREA_TOP:
            {
                cerr << "CONQUER_ENEMY_AREA_TOP "  << endl;
                if(g_turns <= g_width/2){
                    cerr << "g_turns <= g_width/2 "  << endl;
                    objective_list.push_back(std::make_shared<Position>(g_areaDelimiterMap[AreaDelimiterType::ENEMY_AREA_TOP]));
                } else {
                    if(closer_enemy_tile != nullptr){
                        objective_list.push_back(closer_enemy_tile);
                        cerr << "closer_enemy_tile should be fine "  << endl;
                    } else {
                        cerr << "closer_enemy_tile nullptr "  << endl;
                    }
                }
            break;
            }
            case RobotBehaviourStrategy::CONQUER_ENEMY_AREA_BOTTOM:
            {
                if(g_turns <= g_width/2){
                    objective_list.push_back(std::make_shared<Position>(Position(g_areaDelimiterMap[AreaDelimiterType::ENEMY_AREA_BOTTOM])));
                } else {
                    if(closer_enemy_tile != nullptr){
                        objective_list.push_back(closer_enemy_tile);
                    }
                }
            break;
            }
            case RobotBehaviourStrategy::CONQUER_ANY_MAP_AREA:
            {
                //Go to enemy tile if closer or equal
                if(distance_to_enemy < distance_to_neutral){
                    if(closer_enemy_tile != nullptr){
                        objective_list.push_back(closer_enemy_tile);
                    }
                }
                return getCloserNeutralTiles(true);
            break;   
            }
            case RobotBehaviourStrategy::AGGRO_CONQUER_TILES:
            {
                if(closer_enemy_tile != nullptr){
                    objective_list.push_back(closer_enemy_tile);
                }
            break;
            }
            case RobotBehaviourStrategy::AGGRESIVE:
            {
                cerr << "AGGRESIVE "  << endl;
                if(closer_enemy_tile_with_robots != nullptr){
                    cerr << "objective_list.push_back(pos); " << to_string(closer_enemy_tile_with_robots)  << endl;
                    objective_list.push_back(closer_enemy_tile_with_robots);
                }
                cerr << "done " << endl;
                break;
            }
            default:
            {
                if(closer_enemy_tile != nullptr){
                    objective_list.push_back(closer_enemy_tile);
                }
            break;           
            }
        }
        cerr << "returning objective_list "  << endl;
        return objective_list;
    }
};
/** Methods / Logic needed to assign the different behaviours at a given moment **/
// void getCloserRobotToPos(vector<RobotEntity>& robotList, Position pos, std::shared_ptr <RobotEntity>& closerRobot){
int getCloserRobotToPosIndex(vector<RobotEntity>& robotList, const Position& pos){
    int distance = 99;
    //std::shared_ptr<RobotEntity>
    int i = 0;
    int closerRobotIndex = 99;
    for (auto it = begin (robotList); it != end (robotList); ++it, i++) {
        int new_distance = it->mPos.distanceTo(pos);
        if(new_distance < distance){
            distance = new_distance;
            closerRobotIndex = i;
        }
    }
    return closerRobotIndex;
}
void getCloserRobotToPos(const vector<RobotEntity>& robotList,const Position& pos, RobotEntity& closerRobot){
    int distance = 99;
    //std::shared_ptr<RobotEntity>
    for(auto robot: robotList){
        int new_distance = robot.mPos.distanceTo(pos);
        if(new_distance < distance){
            distance = new_distance;
            closerRobot = robot;
        }
    }
   
}
//TODO Use lambda
void removeRobotFromList(vector<RobotEntity>& list,const RobotEntity& r ) {
    list.erase(std::remove(list.begin(), list.end(), r), list.end());
}
void assignRobotsStrategy(){
   
    for(int i=0; i < g_friendlyRobots.size() * FRIENDLY_ROBOTS_PROPORTION/2; i++){
        //Conquer friendly areas
        std::shared_ptr<Tile> t_bot = getTileByPos(g_areaDelimiterMap[FRIENDLY_AREA_BOTTOM]);
        if (t_bot != nullptr ){
            //TODO: Do something if already under our control
            int index = getCloserRobotToPosIndex(g_friendlyRobots,t_bot->mPos);
            if(g_friendlyRobots[index].mStrategy == CONQUER_ANY_MAP_AREA && g_friendlyRobots[index].mPos != g_areaDelimiterMap[FRIENDLY_AREA_BOTTOM]){
                g_friendlyRobots[index].mStrategy = RobotBehaviourStrategy::CONQUER_FRIENDLY_AREA_BOTTOM;
                //cerr << "CONQUER_FRIENDLY_AREA_BOTTOM "<< i << endl;
            }
        }
        std::shared_ptr<Tile> t_top = getTileByPos(g_areaDelimiterMap[FRIENDLY_AREA_TOP]);
        if (t_top != nullptr ){
            //TODO: Do something if already under our control
            int index = getCloserRobotToPosIndex(g_friendlyRobots,t_top->mPos);
            if(g_friendlyRobots[index].mStrategy == CONQUER_ANY_MAP_AREA && g_friendlyRobots[index].mPos != g_areaDelimiterMap[FRIENDLY_AREA_TOP]){
                g_friendlyRobots[index].mStrategy = RobotBehaviourStrategy::CONQUER_FRIENDLY_AREA_TOP;
                //cerr << "CONQUER_FRIENDLY_AREA_TOP "<< i << endl;
            }
        }
    }
    for(int i=0; i < g_friendlyRobots.size() * CONQUER_ENEMY_AREA_ROBOTS_PROPORTION/2; i++){
        //Conquer enemy area
        std::shared_ptr<Tile> t_enemy_top = getTileByPos(g_areaDelimiterMap[ENEMY_AREA_TOP]);
        if (t_enemy_top != nullptr ){
            //TODO: Do something if already under our control
            int index = getCloserRobotToPosIndex(g_friendlyRobots,t_enemy_top->mPos);
            if(g_friendlyRobots[index].mStrategy == CONQUER_ANY_MAP_AREA){
                g_friendlyRobots[index].mStrategy = RobotBehaviourStrategy::CONQUER_ENEMY_AREA_TOP;
                //cerr << "CONQUER_ENEMY_AREA_TOP "<< i << endl;
                //cerr << "Assigning conquer aggresive at "<< to_string(g_friendlyRobots[index].mPos) << endl;
                //removeRobotFromList(friendlyRobots,*r);
            }
        }
        //Conquer enemy area
        std::shared_ptr<Tile> t_enemy_bot = getTileByPos(g_areaDelimiterMap[ENEMY_AREA_BOTTOM]);
        if (t_enemy_bot != nullptr ){
            //TODO: Do something if already under our control
            int index = getCloserRobotToPosIndex(g_friendlyRobots,t_enemy_bot->mPos);
            if(g_friendlyRobots[index].mStrategy == CONQUER_ANY_MAP_AREA){
                g_friendlyRobots[index].mStrategy = RobotBehaviourStrategy::CONQUER_ENEMY_AREA_BOTTOM;
                //cerr << "CONQUER_ENEMY_AREA_TOP "<< i << endl;
                //cerr << "Assigning conquer aggresive at "<< to_string(g_friendlyRobots[index].mPos) << endl;
                //removeRobotFromList(friendlyRobots,*r);
            }
        }
    }
    for(int i=0; i < g_friendlyRobots.size() * RECYCLER_SPOT_FINDERS_ROBOTS_PROPORTION; i++){
        //Get best recycler spot
        std::shared_ptr<Tile> t_rec = getTileByPos(*getBestBuildPosition(g_tileList));
        if (t_rec != nullptr ){
            //TODO: Do something if already under our control
            std::shared_ptr <RobotEntity> r;
            int index = getCloserRobotToPosIndex(g_friendlyRobots,t_rec->mPos);
            if(g_friendlyRobots[index].mStrategy == CONQUER_ANY_MAP_AREA){
                //cerr << "Assigning recycler spot robot finder at "<< to_string(g_friendlyRobots[index].mPos) << endl;
                g_friendlyRobots[index].mStrategy = RobotBehaviourStrategy::SECURE_BEST_RECYCLER_TILES;
                //removeRobotFromList(friendlyRobots,*r);
            }
        }
    }
   
    std::vector<std::shared_ptr<Position>> aggro_positions = getFriendlyTilesCloserToEnemy(true,false,false);
    for(int i=0; i < g_friendlyRobots.size() * AGGRESIVE_ROBOT_PROPORTION; i++){
        for(auto aggro_pos : aggro_positions){
            if(aggro_pos != nullptr){
                int index = getCloserRobotToPosIndex(g_friendlyRobots,*aggro_pos);
                if(g_friendlyRobots[index].mStrategy == CONQUER_ANY_MAP_AREA){
                    //cerr << "Assigning AGGRESIVE robot at "<< to_string(g_friendlyRobots[index].mPos) << endl;
                    g_friendlyRobots[index].mStrategy = RobotBehaviourStrategy::AGGRESIVE;
                    continue;
                }
            }
        }
    }
    //Take the rest and assign them
    /*
    for(int i=0; i < friendlyRobots.size(); i++){
        cerr << "Assigning conquer defensive  robot finder , iteration "<< i << endl;
        //Get best recycler spot
        friendlyRobots[i].mStrategy = RobotBehaviourStrategy::CONQUER_DEFENSIVE;       
    }
    */
    if(g_turns > 10){
        AGGRESIVE_ROBOT_PROPORTION = 0.5;
        CONQUER_ENEMY_AREA_ROBOTS_PROPORTION = 0.3;
        CONQUER_ENEMY_AREA_ROBOTS_PROPORTION = 0.3;
        FRIENDLY_ROBOTS_PROPORTION = 0.0;
        RECYCLER_SPOT_FINDERS_ROBOTS_PROPORTION = 0.0;
    } else if (g_turns > 30 ) {
        AGGRESIVE_ROBOT_PROPORTION = 0.5;
        CONQUER_ENEMY_AREA_ROBOTS_PROPORTION = 0.5;
        FRIENDLY_ROBOTS_PROPORTION = 0.0;
        RECYCLER_SPOT_FINDERS_ROBOTS_PROPORTION = 0.0;
    }
    //for(auto friendly_robot : g_friendlyRobots){
    //
    //     cerr << "friendly_robot.mStrategy " << friendly_robot.mStrategy << endl;
    //}
    //       
    //Get best recycler spot
   
}
std::shared_ptr<Position> getCloserEnemyTile(const std::shared_ptr<Position>& position,const bool mustHaveEnemies){
    int distance = 99;
    std::shared_ptr<Position> ret;
    for (auto tile : g_enemyTileList){
        //Dont consider tiles with a recycler, we cannot reach them and will be destroyed
        if(tile.mRecycler == 1){
            continue;
        }
        if(mustHaveEnemies && tile.mUnits != 0){
            continue;
        }
        int new_distance = position->distanceTo(tile.mPos);
        if(new_distance < distance){
            distance = new_distance;
            ret = std::make_shared<Position>(tile.mPos);
        }
    }
    return ret;
}
std::shared_ptr<Position> getCloserFriendlyTile(const std::shared_ptr<Position>& position,const bool must_be_spawnable,const bool must_be_buildable){
    int distance = 99;
    std::shared_ptr<Position> ret;
    for(auto tile : g_friendlyTileList){
        if(tile.mCan_spawn == false && must_be_spawnable == true){
            continue;
        }
        if(tile.mCan_build == false && must_be_buildable){
            continue;
        }
        int new_distance = position->distanceTo(tile.mPos);
        if(new_distance < distance){
            distance = new_distance;
            ret = std::make_shared<Position>(tile.mPos);
        }
    }
    return ret;
}
        //todo: always move even though implies changing strategy, late game looks bad
       
std::vector<std::shared_ptr<Position>> getFriendlyTilesCloserToEnemy(bool must_be_spawnable, bool must_be_buildable, bool must_have_enemies){
   
    std::vector<std::shared_ptr<Position>> friendlyTiles = {};
    //Get closer enemy tile to our areas
    std::shared_ptr<Position> e_pos_middle = getCloserEnemyTile(std::make_shared<Position>(g_areaDelimiterMap[AreaDelimiterType::NEUTRAL_AREA_MIDDLE]), must_have_enemies);
    std::shared_ptr<Position> e_pos_top = getCloserEnemyTile(std::make_shared<Position>(g_areaDelimiterMap[AreaDelimiterType::NEUTRAL_AREA_TOP]), must_have_enemies);
    std::shared_ptr<Position> e_pos_bottom = getCloserEnemyTile(std::make_shared<Position>(g_areaDelimiterMap[AreaDelimiterType::NEUTRAL_AREA_BOTTOM]), must_have_enemies);
    //Get closer friendly tiles to the enemy tiles we just obtained
    if(e_pos_middle != nullptr){
        std::shared_ptr<Position> f_pos_middle = getCloserFriendlyTile(e_pos_middle, must_be_spawnable, must_be_buildable);
        if (f_pos_middle != nullptr){
            friendlyTiles.emplace_back(f_pos_middle);
        }
    }
    if(e_pos_top != nullptr){
        std::shared_ptr<Position> f_pos_top = getCloserFriendlyTile(e_pos_top, must_be_spawnable, must_be_buildable);
        if (f_pos_top != nullptr){
            friendlyTiles.emplace_back(f_pos_top);
        }
    }
    if(e_pos_bottom != nullptr){
        std::shared_ptr<Position> f_pos_bottom = getCloserFriendlyTile(e_pos_bottom, must_be_spawnable, must_be_buildable);
            if (f_pos_bottom != nullptr){
                friendlyTiles.emplace_back(f_pos_bottom);
            }
    }
    return friendlyTiles;
}
/**************          RECYCLER METHODS        *********************/
//Get a list containing the rating for the different spots
list<tuple<int,Tile>> getTilesRecyclerRating(vector<Tile> possibleSpots){
    int max_tile_rating = 0;
    list<tuple<int,Tile>> rating_to_pos_tuple;
    for(auto tile : possibleSpots){
        int rating = getTileRecyclerRating(tile);
        rating_to_pos_tuple.push_back(std::make_tuple(rating,tile));
    }
    return rating_to_pos_tuple;
}
//Rating is a sum of the scrap amount we can find in the surrounding tiles
int getTileRecyclerRating(Tile tile){
    int total_rating = tile.mScrap_amount;
    //Get scrap amount from surrounding tiles
    shared_ptr<Tile> t_right, t_left,t_top,t_bottom;
    t_right = getTileByPos(tile.mPos + Position(1,0));
    t_left = getTileByPos(tile.mPos + Position(-1,0));
    t_top = getTileByPos(tile.mPos + Position(0,1));
    t_bottom = getTileByPos(tile.mPos + Position(0,-1));
    //Get total rating
    if(t_right!= nullptr){
        total_rating += t_right->mScrap_amount;
    }
    if(t_left!= nullptr){
        total_rating += t_left->mScrap_amount;
    }
    if(t_top!= nullptr){
        total_rating += t_top->mScrap_amount;
    }
    if(t_bottom!= nullptr){
        total_rating += t_bottom->mScrap_amount;
    }
    //TODO: Get one further from enemy
    return total_rating;
}
std::shared_ptr<Position> getCloserRecyclerPos(const Position& pos, const OwnerTypes owner) {
    int distance = 99;
    std::shared_ptr<Position> closer_recycler_pos = nullptr;
    auto tileList = (owner == OwnerTypes::ENEMY ? g_enemyTileList : g_friendlyTileList);
    for (auto tile : tileList){
        if(tile.mRecycler == 0){
            continue;
        }
        int new_distance = pos.distanceTo(tile.mPos);
        if(new_distance < distance){
            distance = new_distance;
            closer_recycler_pos = std::make_shared<Position>(tile.mPos);
        }
    }
    return closer_recycler_pos;
}
shared_ptr<Position> getBestBuildPosition(vector<Tile> possibleSpots){
    list<tuple<int,Tile>> rated_tile_list = getTilesRecyclerRating(possibleSpots);
    int max_rating = 0;
    shared_ptr<Position> best_pos = nullptr;
    for(auto rated_tuple: rated_tile_list){
        int rating = std::get<0>(rated_tuple);
       
        bool friendly_recycler_is_far = false;
        bool enemy_recycler_is_close = false;
        std::shared_ptr<Position> friendly_closer_recycler = getCloserRecyclerPos(std::get<1>(rated_tuple).mPos, OwnerTypes::FRIENDLY);
        std::shared_ptr<Position> enemy_closer_recycler = getCloserRecyclerPos(std::get<1>(rated_tuple).mPos, OwnerTypes::ENEMY);
        if(friendly_closer_recycler != nullptr){
            friendly_recycler_is_far = friendly_closer_recycler->distanceTo(std::get<1>(rated_tuple).mPos) > 1;
            //cerr << "Friendly recycler position is far enough" << endl;
        } else {
            friendly_recycler_is_far = true; //No recyclers obtained , so friendly recyclers are "far"
            //cerr << "friendly_closer_recycler nullptr" << endl;
        }
        if(enemy_closer_recycler != nullptr){
            enemy_recycler_is_close = enemy_closer_recycler->distanceTo(std::get<1>(rated_tuple).mPos) == 1;
            //cerr << "Enemey recycler is close far enough" << endl;
        } else {
            enemy_recycler_is_close = false;
            //cerr << "enemy_closer_recycler nullptr"  << endl;
        }       
        if ((max_rating < rating && std::get<1>(rated_tuple).mCan_build) && (friendly_recycler_is_far || enemy_recycler_is_close)) {
            max_rating = rating;
            best_pos = std::make_shared<Position>(std::get<1>(rated_tuple).mPos);
        }
    }
    return best_pos;
}
/***********************************************/
shared_ptr<Tile> getTileByPos(Position pos){
    for(auto tile: g_tileList){
        if(tile.mPos == pos){
            return make_shared<Tile>(tile);
        }
    }
    //In case we would query a Tile out of the map
    return nullptr;
}
void setPlayerMapReferences(int width, int height){
    g_width = width;
    g_height = height;
    int size = width * height;
    if(size > 200){
        g_map_size = SUPER_BIG;
    } else if (size > 150){
        g_map_size = BIG;
    } else if (size > 100){
        g_map_size = MEDIUM;
    } else {
        g_map_size = SMALL;
    }
    int friendly_dist_to_origin = g_friendlyTileList[0].mPos.distanceTo(Position(0,0));
    int enemy_dist_to_origin = g_enemyTileList[0].mPos.distanceTo(Position(0,0));
    if(friendly_dist_to_origin < enemy_dist_to_origin){
        g_areaDelimiterMap[AreaDelimiterType::FRIENDLY_AREA_TOP] = Position(0,0);
        g_areaDelimiterMap[AreaDelimiterType::FRIENDLY_AREA_BOTTOM] = Position(0,height-1);
        g_areaDelimiterMap[AreaDelimiterType::FRIENDLY_AREA_MIDDLE] = Position(0,(height-1)/2);
        g_areaDelimiterMap[AreaDelimiterType::ENEMY_AREA_TOP] = Position(width-1,0);
        g_areaDelimiterMap[AreaDelimiterType::ENEMY_AREA_BOTTOM] = Position(width-1,height-1);
        g_areaDelimiterMap[AreaDelimiterType::ENEMY_AREA_MIDDLE] = Position((width-1)/2,(height-1)/2);
    } else {
        g_areaDelimiterMap[AreaDelimiterType::ENEMY_AREA_TOP] = Position(0,0);
        g_areaDelimiterMap[AreaDelimiterType::ENEMY_AREA_BOTTOM] = Position(0,height-1);
        g_areaDelimiterMap[AreaDelimiterType::ENEMY_AREA_MIDDLE] = Position(0,(height-1)/2);
       
        g_areaDelimiterMap[AreaDelimiterType::FRIENDLY_AREA_TOP] = Position(width-1,0);
        g_areaDelimiterMap[AreaDelimiterType::FRIENDLY_AREA_BOTTOM] = Position(width-1,height-1);
        g_areaDelimiterMap[AreaDelimiterType::ENEMY_AREA_MIDDLE] = Position((width-1)/2,(height-1)/2);

    }
    g_areaDelimiterMap[AreaDelimiterType::NEUTRAL_AREA_TOP] = Position((width-1)/2,0);
    g_areaDelimiterMap[AreaDelimiterType::NEUTRAL_AREA_MIDDLE] = Position((width-1)/2,height-1);
    g_areaDelimiterMap[AreaDelimiterType::NEUTRAL_AREA_MIDDLE] = Position((width-1)/2,(height-1)/2);
    cerr << "Friendly area  : TOP " << to_string(g_areaDelimiterMap[AreaDelimiterType::FRIENDLY_AREA_TOP]) << " BOTTOM " <<  to_string(g_areaDelimiterMap[AreaDelimiterType::FRIENDLY_AREA_BOTTOM]) << endl;
    cerr << "Enemy area  : TOP " << to_string(g_areaDelimiterMap[AreaDelimiterType::ENEMY_AREA_TOP]) << " BOTTOM " <<  to_string(g_areaDelimiterMap[AreaDelimiterType::ENEMY_AREA_BOTTOM]) << endl;
}
void appendRobotMovementStr(std::string& movement_str, const RobotEntity& robot){
    cerr << "appendRobotMovementStr" << endl;
    //Verify quantity of robots we want to move --> divide it to expand easier
    std::vector<std::shared_ptr<Position>> new_destination_list = robot.getNextStrategyObjectives();
    cerr << "checking next strategy objectives" << endl;
    if(new_destination_list.size() > 0 ){
        if((new_destination_list.size() == 1 || robot.mQuantity == 1) && new_destination_list[0] != nullptr){
            cerr << "Debug moving robot at " << to_string(robot.mPos) << endl;
            //int quantity_to_move = robot.mQuantity > 1 ? robot.mQuantity/2 :  robot.mQuantity;
            movement_str += "MOVE " + to_string(robot.mQuantity) + " " + to_string(robot.mPos) + " " + to_string(new_destination_list[0]) + ";";
            cerr << "partial move_str :"<< movement_str << endl;
            //cerr << "Robot at " << to_string(robot.mPos) << " moving towards " << to_string(new_destination_list[0]) << endl;
            //cerr << "\tRobot mStrategy " << robot.mStrategy << endl;
        } else {
            cerr << "quantity_per_position " << endl;
            int quantity_per_position = robot.mQuantity / new_destination_list.size();
            if(quantity_per_position == 0) {
                quantity_per_position = 1;
            }
            int remaining_robots = robot.mQuantity;
            for(auto pos : new_destination_list){
                movement_str += "MOVE " + to_string(quantity_per_position) + " " + to_string(robot.mPos) + " " + to_string(pos) + ";";
                //cerr << "partial move_str :"<< move_str << endl;
                cerr << "Robot (qty: " << quantity_per_position <<") at " << to_string(robot.mPos) << " moving towards " << to_string(pos) << endl;
                cerr << "\tRobot mStrategy " << robot.mStrategy << endl;
                remaining_robots -= quantity_per_position;
                if(remaining_robots <= 0){
                    break;
                }
            }
        }
    } else {
        cerr << "CANNOT OBTAIN NEW DESTINATION" << endl;
    }
}
std::string getBuildStr(int& currentMatter){
    //TODO: Stop building if we already have the majority of the map conquered
    //We have enough control, we dont need more buildings
    if((g_game_strategy == GlobalStrategy::DEFENSIVE && (g_enemyTileList.size() < (g_friendlyTileList.size() - 5)) )){
        //cerr << "We are not building anymore, turn 30 and 5+ tiles than enemy " << endl;
        return "";
    } else if (g_enemyTileList.size() < (g_friendlyTileList.size() - 10)) {
        //cerr << "We are not building anymore, we have 10+ tiles than the enemy" << endl;
        return "";
    } else if((g_turns == 0 || g_turns % 3 == 0) && currentMatter >= 10 ){
        //cerr << "Building, we dont have enough control yet" << endl;
        shared_ptr<Position> buildPos = getBestBuildPosition(g_friendlyTileList);
        if(buildPos != nullptr) {
            currentMatter -= 10;
            return std::string( "BUILD " + to_string(buildPos) + ";");
        } else {
            return "";
        }
    }
    return "";
}
std::string getSpawnStr(int& currentMatter){
    bool must_have_enemies;
    int spawn_quantity = 1;
    if(g_game_strategy == EXPAND_TERRITORY){
        must_have_enemies = false;
    } else if (g_game_strategy == DEFENSIVE){
        spawn_quantity = 3;
        must_have_enemies = true;
    }
    int possible_spawns = currentMatter / 10;
    //Must check if it is spawnable
    cerr << "currentMatter " << currentMatter << endl;
    cerr << "possible_spawns " << possible_spawns << endl;
    std::vector<std::shared_ptr<Position>> spawn_positions = getFriendlyTilesCloserToEnemy(true, false,must_have_enemies);
    if(spawn_positions.size() == 0){
        return "";
    }
    std::string spawn_str;
    int current_spawns = 0;
    while(currentMatter >= (spawn_quantity * 10) && possible_spawns >= current_spawns){
        cerr << "spawning stuff! " << endl;
        for(auto spawn_pos : spawn_positions){
            if(spawn_pos != nullptr){
                spawn_str += "SPAWN " + to_string(spawn_quantity) + " " + to_string(spawn_pos) + ";";
                //RobotEntity r (spawn_pos->x, spawn_pos->y,FRIENDLY,spawn_quantity);
                //appendRobotMovementStr(moveAfterSpawnStr, r);
                //Update current matter we spent and spawns we made
                currentMatter -= (spawn_quantity * 10);
                current_spawns += spawn_quantity;
                cerr << "current_spawns " << current_spawns << endl;
                if(possible_spawns >= current_spawns){
                    break;
                }
            }
        }
    }
    return spawn_str;
}
void updateGameStrategy(){
   
    switch(g_map_size)
    {
        case SMALL:
        {
            g_game_strategy = g_turns >= 5 ? GlobalStrategy::EXPAND_TERRITORY : GlobalStrategy::DEFENSIVE; 
            break;
        }
        case MEDIUM:
        {
            g_game_strategy = g_turns >= 10 ? GlobalStrategy::EXPAND_TERRITORY : GlobalStrategy::DEFENSIVE; 
            break;
        }
        case BIG:
        {
            g_game_strategy = g_turns >= 15 ? GlobalStrategy::EXPAND_TERRITORY : GlobalStrategy::DEFENSIVE; 
            break;
        }
        case SUPER_BIG:
        {
            g_game_strategy = g_turns >= 20 ? GlobalStrategy::EXPAND_TERRITORY : GlobalStrategy::DEFENSIVE; 
            break;
        }
        default:
        break;
    }
   
}
int main()
{
    int width;
    int height;
    cin >> width >> height; cin.ignore();
    // game loop
    while (1) {
        int my_matter;
        int opp_matter;
        cin >> my_matter >> opp_matter; cin.ignore();
        clearGlobals();//Do that after iteration so that they are refreshed
        for (int i = 0; i < height; i++) {
            for (int j = 0; j < width; j++) {
                int scrap_amount;
                int owner; // 1 = me, 0 = foe, -1 = neutral
                int units;
                int recycler;
                int can_build;
                int can_spawn;
                int in_range_of_recycler;
                cin >> scrap_amount >> owner >> units >> recycler >> can_build >> can_spawn >> in_range_of_recycler; cin.ignore();
                Tile t (j,i,scrap_amount,owner,units,recycler,can_build,can_spawn, in_range_of_recycler);
                g_tileList.emplace_back(t);
                if(owner == 1){
                    g_friendlyTileList.emplace_back(t);
                    if(units > 0){
                        RobotEntity r (j,i,owner,units);
                        g_friendlyRobots.emplace_back(r);
                    }
                } else if (owner == 0){
                    g_enemyTileList.emplace_back(t);
                    if(units > 0){
                        RobotEntity r (j,i,owner,units);
                        g_enemyRobots.emplace_back(r);
                    }
                }
            }
        }
        //cerr << "g_tileList.size() " << g_tileList.size() << endl;
        //cerr << "g_friendlyTileList.size() " << g_friendlyTileList.size() << endl;
        //cerr << "g_enemyTileList.size() " << g_enemyTileList.size() << endl;
        //cerr << "g_friendlyRobots.size() " << g_friendlyRobots.size() << endl;
        //cerr << "g_enemyRobots.size() " << g_enemyRobots.size() << endl;
        static bool first_time = true;
        //Get enemy area and friendly area and their delimiters
        if(first_time){
            setPlayerMapReferences(width,height);
            first_time = false;
        }
        ////////////////////////////////////////////////////////////////////
        /*
        using std::chrono::high_resolution_clock;
        using std::chrono::milliseconds;
        using std::chrono::duration_cast;
        using std::chrono::duration;
       
        auto t1 = high_resolution_clock::now();
        auto t4 = high_resolution_clock::now();
        auto ms_int_2 = duration_cast<milliseconds>(t4 - t3);
        cerr << "Until getSpawnStr took " <<  ms_int_2.count() << " milliseconds" << endl;
        */
        /////////////////////////////////////////////////////////////////////
        updateGameStrategy();
        std::string build_str = "";
        std::string move_str = "";
        std::string spawn_str = "";
        //cerr << "g_friendlyRobots.size() "<< g_friendlyRobots.size() << endl;
        assignRobotsStrategy();
        cerr << "Debug before appendRobotMovementStr " << endl;
        //Get move string
        for(auto friendly_robot : g_friendlyRobots){
            appendRobotMovementStr(move_str, friendly_robot);
        }
        cerr << "Debug getBuildStr " << endl;
        build_str = getBuildStr(my_matter);
        //Spawn tanks closer to enemy tile
 
        cerr << "Debug getSpawnStr " << endl;
        spawn_str = getSpawnStr(my_matter);

        cerr << "move_str: " << move_str << endl;
        cerr << "build_str: " << build_str << endl;
        cerr << "spawn_str: " << spawn_str << endl;
        g_turns++;
        std::string final_string;
        if(move_str.size() == 0 && build_str.size() == 0 && spawn_str.size() == 0){
            final_string = "WAIT;";
        } else {
            final_string  = move_str + build_str + spawn_str; //+ move_str_after_spawn;
        }
        if(final_string == ""){
           final_string = "WAIT;";
        }
        cerr << "final_string: " << final_string << endl;
        cout << final_string << endl;
  
 
    }
} 