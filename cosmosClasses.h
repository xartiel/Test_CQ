#ifndef COSMOS_CLASSES_HEADER
#define COSMOS_CLASSES_HEADER

#include <string>
#include <vector>
#include <iostream>
#include <iomanip>
#include <cstdint>

using namespace std;

// Define types of HeroSkills and Elements
enum SkillType {nothing, buff, protect, aoe, pAoe, heal, berserk, friends};
enum Element {
    earth   = 0,
    air     = 1, 
    water   = 2, 
    fire    = 3, 
    all, 
    self
}; // also used for hero skill targets

// Defines Skills of Heros
struct HeroSkill {
    SkillType type;
    Element target;
    Element sourceElement;
    double amount;
};
static HeroSkill none = HeroSkill({nothing, air, air, 1}); // base skill used for normal monsters

// Defines a Monster or Hero
class Monster {
    public :
        int hp;
        int damage;
        int cost;
        bool isHero;
        string name;
        Element element;
        HeroSkill skill;
        
        Monster(int hp, int damage, int cost, string name, Element element, HeroSkill skill = none);
        Monster();
};

// Function for sorting Monsters by cost (ascending)
bool isCheaper(Monster * a, Monster * b);

// Defines the results of a fight between two armies; monstersLost and damage desribe the condition of the winning side
class FightResult {
    public :
        int16_t damage;             // how much damage dealt to the current leading mob of the winning side
        int16_t leftAoeDamage;      // how much aoe damage left took
        int16_t rightAoeDamage;     // how much aoe damage right took
        int8_t berserk;            // berserk multiplier, if there is a berserker in the front
        int8_t monstersLost;    // how many mobs lost on the winning side (the other side lost all)
        bool valid;             // If the result is valid
        bool rightWon;          // false -> left win, true -> right win.
        bool dominated;         // If the result is worse than another
            
        FightResult();
        
    bool operator <= (FightResult & toCompare);
    bool operator >= (FightResult & toCompare);
};

// Defines a single lineup of monsters
class Army {
    public:
        Monster* monsters[6];
        FightResult lastFightData;
        int32_t followerCost;
        int8_t monsterAmount;
        
        void add(Monster * m);
        void print();
        Army(vector<Monster *> monsters = {});
};

// Function for sorting Armies by followers (ascending)
bool hasFewerFollowers(const Army & a, const Army & b);

#endif