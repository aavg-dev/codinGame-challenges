#include <iostream>
#include <math.h>
#include <string>
#include <vector>
#include <algorithm>
#include <cmath>
#include <list>
#include <memory>


using namespace std;

enum Action {BARRACKS_KNIGHT,
             BARRACKS_ARCHER,
             MOVE_NEXT_SITE,
             PROTECT,
             WAIT
            };

enum OwnerTypes {
    FRIENDLY = 0,
    ENEMY = 1
};

class Building;
void updateBarrackList( const shared_ptr<Building> barrack );








/*************************/


class Building 
{
public:
    Building(){
        mSiteId = -1;
        mX = 0;
        mY = 0;
        mRadius = 0;
        int mOwner = -1;
    }

    Building(int side_id, int x, int y, int radius ){
        mSiteId = side_id;
        mX = x;
        mY = y;
        mRadius = radius;
        mOwner = -1;

    }
    Building(const Building &b){
        mSiteId = b.mSiteId;
        mX = b.mX;
        mY = b.mY;
        mRadius = b.mRadius;
        mOwner = b.mOwner;
    }

   
    double getDistanceToBarrack(int x , int y){
        return (sqrt(pow(x - mX, 2) + pow(y - mY, 2)));
    }

    enum BuildingType{NONE,BARRACK,TOWER,MINE};

    //private would be better, but makes things more difficult
    int mSiteId = -1;
    int mX = 0;
    int mY = 0;
    int mRadius = 0;
    int mOwner = -1;
    BuildingType mBuildingType = NONE;


};

class Barrack : public Building 
{
    public:
    enum BarrackType {NONE,BARRACK_KNIGHT, BARRACK_ARCHER,BARRACK_GIANT};

    Barrack(Building b, BarrackType type, bool canTrain): Building(b) {
        mBuildingType = BuildingType::BARRACK;
        mBarrackType = type;
        mCanTrain = canTrain;
    }

    BarrackType mBarrackType = BarrackType::NONE;
    bool mCanTrain = false;
};




static string to_string(const Building &b)
{
    return "SiteId "+ to_string(b.mSiteId) +
     "; mX" + to_string(b.mX) + "; mY" + to_string(b.mY) +
      "; mRadius" + to_string(b.mRadius) + "; mOwner" + to_string(b.mOwner);
}

static string to_string(shared_ptr<Building> &b)
{
    if(b != nullptr){
        return "SiteId "+ to_string(b->mSiteId) +
        "; mX" + to_string(b->mX) + "; mY" + to_string(b->mY) +
        "; mRadius" + to_string(b->mRadius) + "; mOwner" + to_string(b->mOwner);
    }
    return "";
}

//Barrack related globals and functions
static vector<shared_ptr<Building>> buildingList = {};
static vector<shared_ptr<Building>> builtBuildingsList = {};

bool getBuildingById(const int site_id, shared_ptr<Building>& building){
    //cerr << "Iterating over barrack ids. Looking for barrack "<< site_id <<endl;
    //cerr << "Looking for barrack with id "<< site_id  << endl;
    for(auto b: buildingList){
        //cerr << "b->mSiteId "<< b->mSiteId <<endl;
        //cerr << "site_id "<< site_id <<endl;

        if (b->mSiteId == site_id){
            building = b;
            //cerr << "Barrack " << b->mSiteId << " found "<< endl;
            //cerr << to_string(barrack) << endl;
            return true;
        }
    }
    //cerr << "Cannot obtain barrack with id"<< site_id <<endl;
    building = nullptr;
    return false;
}


class Unit {

public:
    enum Type {QUEEN = -1, KNIGHT = 0, ARCHER = 1};

    Unit(int x, int y, int owner, Type type, int health):
     mX(x), mY(y), mOwner(owner), mType(type), mHealth(health)
    {
    }

    //Return id of the closer barrack
    int getCloserBuilding(bool consider_friendly_barrack = false){
        int closer_distance = 1500;
        int closer_barrack_id = -1;
        for (auto b: buildingList){

            if(!consider_friendly_barrack && b->mOwner == (int)OwnerTypes::FRIENDLY){
                cerr << "Closer barrack is a friendly one, dont consider" << endl;
                continue;
            }
            int d = b->getDistanceToBarrack(this->mX, this->mY);
            //cerr << "DistanceToBarrack " << d <<endl;
            if(d < closer_distance){
                //cerr << "closer_distance is  " << d <<endl;
                closer_distance = d;
                closer_barrack_id = b->mSiteId;
            
            }
        }
        cerr << "closer barrack is  " << closer_barrack_id <<endl;

        return closer_barrack_id;
    }

    int mX;
    int mY;
    int mOwner; //0 Friendly , 1 Enemy 
    Type mType; // -1 = QUEEN, 0 = KNIGHT, 1 = ARCHER
    int mHealth;
};

class Queen: public Unit {
public:

    Queen(int x, int y, int owner, int health):Unit(x,y,owner, Unit::Type::QUEEN,health){

    }

 string getMoveNextLocationStr(){
    int site_id = getCloserBuilding();
    shared_ptr<Building> b;
    string ret;
    if( getBuildingById(site_id, b)) {
        cerr << "Found barrack with id "<< site_id  << " to move to" <<  endl;

        ret = "MOVE " + to_string(b->mX) + " " + to_string(b->mY);
    } else {
        cerr << "CANNOT OBTAIN BARRACK to move to" << endl;
        ret = "WAIT";
    }
    cerr << "getMoveNextLocationStr ["<< ret << "]" << endl;
    return ret;
 }

};


/**
 * Auto-generated code below aims at helping you parse
 * the standard input according to the problem statement.
 **/

//Globals
Queen queen(0,0,0,100);
Queen enemyQueen(0,0,1,100);
int myGold = 0;
int x_home = 0;
int y_home = 0;


 string getBuildBarrackStr(int id, Barrack::BarrackType type ){
    string ret = string("BUILD " + to_string(id) + " ");
    if (type == Barrack::BarrackType::BARRACK_KNIGHT){
        ret += ("BARRACKS-KNIGHT" );
    } else {
        ret += ("BARRACKS-ARCHER" );
    }
    cerr << "getBuildBarrackStr [" << ret << "]" << endl;
    return ret;
 }

  Unit::Type getPredominantType(const list<Unit>& friendlyUnits){
    int archers = 0;
    int knights = 0;
    for(auto unit : friendlyUnits){
        if (unit.mType == Unit::Type::ARCHER) archers++;
        if (unit.mType == Unit::Type::KNIGHT) knights++;
    }
    //Prioritize knights in case we are equal
    //PRIORITIZE KNIGHTS OVERALL unless they triple the archers 
    return (archers * 3 >= knights ? Unit::Type::ARCHER :  Unit::Type::KNIGHT); 
  }


  string getTrainUnitsStr(const list<Unit> &friendlyUnits){
    int gold = myGold;
    int archerCost = 100;
    int knightCost = 80;
    //bool enoughGold;
    //gold > 100 ? enoughGold = true : enoughGold = false;

    Unit::Type predominantUnit = getPredominantType(friendlyUnits);
    cerr << "Predominant unit is " << predominantUnit << endl; //ARCHER 1, KNIGHT 0

    string ret = string("TRAIN" );
    for(auto building: builtBuildingsList){
        //Build only in barracks for the non-predominant unit
        if(building->mBuildingType == Building::BuildingType::BARRACK){
            std::shared_ptr<Barrack> barrack = static_pointer_cast<Barrack> (building);
            if((barrack->mBarrackType == Barrack::BarrackType::BARRACK_ARCHER && predominantUnit == Unit::Type::KNIGHT) || (barrack->mBarrackType == Barrack::BarrackType::BARRACK_KNIGHT && predominantUnit == Unit::Type::ARCHER)) {
                cerr << "Training on barrack of type" << barrack->mBarrackType << endl; //BARRACK ARCHER 3, BARRACK KNIGHT 2
                
                if(barrack->mCanTrain && gold >= 100 ){
                    ret += " " + to_string(barrack->mSiteId) ;
                    if(barrack->mBarrackType == Barrack::BarrackType::BARRACK_ARCHER){
                        gold -= archerCost;
                    } else if (barrack->mBarrackType == Barrack::BarrackType::BARRACK_KNIGHT){
                        gold -= knightCost;
                    }
                }
            }

            if (gold <= 100){
                break;
            }
        }

    }
    cerr << "getTrainUnitsStr [" << ret << "]" << endl;
    return ret;
 }

void updateBuildingData(shared_ptr<Building>& building, const int & owner, const int & structure_type, const int & param_1 , const int & param_2){

    if(building == nullptr){
        cerr << "Cannot update a null building "<<  endl;
        return;
    }

    building->mOwner = owner;
    structure_type == -1 ? building->mBuildingType = Building::NONE : building->mBuildingType = Building::BARRACK;
    switch (structure_type)
    {
    case -1:
        cerr << "No structure at location "<< building->mSiteId << endl;
        building->mBuildingType = Building::BuildingType::NONE;
        break;
    case 0:
        cerr << "Goldmine at location "<< building->mSiteId << endl;
        building->mBuildingType = Building::BuildingType::MINE;

    break;
    case 1:
        cerr << "Tower at location "<< building->mSiteId << endl;
    break;
    case 2:
    {
        cerr << "Barracks at location "<< building->mSiteId << endl;
        Barrack::BarrackType type;
        if(type == 0) type = Barrack::BarrackType::BARRACK_KNIGHT;
        if(type == 1) type = Barrack::BarrackType::BARRACK_ARCHER;
        if(type == 2) type = Barrack::BarrackType::BARRACK_GIANT;
        bool canTrain = param_1 == 0 ? true : false;
        building = std::make_shared<Barrack>(Barrack(*building.get(),type,canTrain));
    }
    break;
    default:
        break;
    }
    updateBarrackList(building);
    shared_ptr<Building> c;
    if (getBuildingById(building->mSiteId,c)){
        if(c != nullptr){
            cerr << "Updated barrack\n" << to_string(c) << endl;
        }  
    }

}


void updateBarrackList( const shared_ptr<Building> barrack ){
    //Considering barrackList[0] corresponds to barrack.mSiteId=0

    buildingList[barrack->mSiteId] = barrack;
    //cerr << "After " << to_string(barrackList[barrack.mSiteId])<< endl;

    for(int i =0; i<builtBuildingsList.size(); i++){
        //cerr << "Updating built barracks" << endl;

        if(barrack->mSiteId == builtBuildingsList[i]->mSiteId){
            builtBuildingsList[i] = barrack;
        }
    }
}



int main()
{
    int num_sites;
    cin >> num_sites; cin.ignore();
    for (int i = 0; i < num_sites; i++) {
        int site_id;
        int x;
        int y;
        int radius;
        cin >> site_id >> x >> y >> radius; cin.ignore();
        //cerr << "Adding barrack " << site_id << " to list" <<endl;
        buildingList.push_back(make_shared<Building>(site_id,x,y,radius));
    }
    //cerr << "barrackList.size() " << barrackList.size() << endl; 

    // game loop
    while (1) {
        int gold;
        int touched_site; // -1 if none
        cin >> gold >> touched_site; cin.ignore();
        myGold = gold;
        for (int i = 0; i < num_sites; i++) {
            int site_id;
            int ignore_1; // used in future leagues
            int ignore_2; // used in future leagues
            int structure_type; // -1 = No structure, 0: Goldmine , 1: Tower, 2: Barracks 
            int owner; // -1 = No structure, 0 = Friendly, 1 = Enemy
            /* param_1
            When no structure: -1
            When goldmine: the income rate ranging from 1 to 5 (or -1 if enemy)
            When tower: the remaining HP
            When barracks, the number of turns before a new set of creeps can be trained (if 0, then training may be started this turn)
             */
            int param_1; 
            /*
            param2
            When no structure: -1
            When goldmine: -1
            When tower: the attack radius measured from its center
            When barracks: the creep type: 0 for KNIGHT, 1 for ARCHER, 2 for GIANT
            */
            int param_2;
            cin >> site_id >> ignore_1 >> ignore_2 >> structure_type >> owner >> param_1 >> param_2; cin.ignore();
            shared_ptr<Building> b = nullptr;
            //Update barracks info
            if (getBuildingById(site_id,b)){
                //cerr << "Updating barrack id " << site_id << endl;
                updateBuildingData(b,owner,structure_type, param_1, param_2);

            }
        }
        list<Unit> friendlyUnitList;

        int num_units;
        cin >> num_units; cin.ignore();
        for (int i = 0; i < num_units; i++) {
            int x;
            int y;
            int owner;
            int unit_type; // -1 = QUEEN, 0 = KNIGHT, 1 = ARCHER
            int health;
            cin >> x >> y >> owner >> unit_type >> health; cin.ignore();

            //Just care about our own queen for the moment
            static bool first_time = true; 
            if(owner == OwnerTypes::FRIENDLY && unit_type == (int) Unit::Type::QUEEN ){
                if(first_time){
                    x_home = x;
                    y_home = y;
                    first_time = false; //Store home to be able to retreat
                }
                queen.mX = x;
                queen.mY = y;
                queen.mHealth = health;
            } else if (owner == OwnerTypes::FRIENDLY){
                Unit u(x,y,owner,(Unit::Type) unit_type, health);
                friendlyUnitList.push_back(u);
            }
        }

        // Write an action using cout. DON'T FORGET THE "<< endl"
        // To debug: cerr << "Debug messages..." << endl;


        // First line: A valid queen action
        // Second line: A set of training instructions
        string action_1_queen;
        string action_2_train;

        //If we have a lot of sites
        if(builtBuildingsList.size() > num_sites/3){
            //retreat!
            action_1_queen = "MOVE " + to_string(x_home) + " " + to_string(y_home);
        } else if (touched_site != -1){
            cerr << "touched_site " << touched_site << endl;
            shared_ptr<Building> building = nullptr;
            if(getBuildingById(touched_site, building)){
                cerr << "Found barrack " << building->mSiteId << "in list" <<  endl;
                cerr << to_string(building) << endl;
                if (building->mBuildingType == Building::NONE && building->mOwner != OwnerTypes::FRIENDLY){
                    //Build !
                    cerr << "Building a new barrack on site id" << building->mSiteId << endl;

                    //For the moment build 1 archer and 1 knight alternating
                    static bool archer_flag = false;
                    if(archer_flag){
                        action_1_queen = getBuildBarrackStr(touched_site, Barrack::BarrackType::BARRACK_ARCHER);
                        //building->mBuildingType = Barrack::BarrackType::BARRACK_ARCHER; TODO: REVIEW
                        archer_flag = false;

                    } else {
                        action_1_queen = getBuildBarrackStr(touched_site, Barrack::BarrackType::BARRACK_KNIGHT);
                        //building->mBuildingType = Barrack::BarrackType::BARRACK_KNIGHT;
                        archer_flag = true;
                    }
                    
                    building->mOwner = int(OwnerTypes::FRIENDLY);
                    //cerr << "Before " << to_string(barrackList[barrack.mSiteId])<< endl;
                    updateBarrackList(building);
                    //cerr << "After " << to_string(barrackList[barrack.mSiteId])<< endl;

                    builtBuildingsList.push_back(building);
                } else {
                    cerr << "Moving to next location, this one has been already built" <<  endl;

                    action_1_queen = queen.getMoveNextLocationStr();
                }
            }
        } else {
            
            cerr << "Moving to location to build" <<  endl;
            action_1_queen = queen.getMoveNextLocationStr();
        }
        action_2_train = getTrainUnitsStr(friendlyUnitList); 
        cout << action_1_queen << endl;
        cout << action_2_train << endl;

    }
}