#include "battleLogic.h"

int totalFightsSimulated = 0;

// Function determining if a monster is strictly better than another
bool isBetter(Monster * a, Monster * b, bool considerAbilities) {
    if (a->element == b->element) {
        return (a->damage >= b->damage) && (a->hp >= b->hp);
    } else { // a needs to be better than b even when b has elemental advantage, or a is at disadvantage
        return !considerAbilities && (a->damage >= b->damage * elementalBoost) && (a->hp >= b->hp * elementalBoost);
    }
}

// TODO: Implement MAX AOE Damage to make sure nothing gets revived
// Simulates One fight between 2 Armies and writes results into left's LastFightData
// Yes, I know this method is unelegant as fuck but it's much faster this way I promise. Should be readable still too
void simulateFight(Army & left, Army & right, bool verbose) {
    // left[0] and right[0] are the first to fight
    // Damage Application Order:
    //  1. Base Damage of creature
    //  2. Multiplicators of self       (friends, berserk)
    //  3. Buffs from heroes            (Hunter, Rei, etc.)
    //  4. Elemental Advantage          (f.e. Fire vs. Earth)
    //  5. Protection of enemy Side     (Nimue, Athos, etc.)
    //  6. AOE of friendly Side         (Tiny, Alpha, etc.)
    //  7. Healing of enemy Side        (Auri, Aeris, etc.)
    
    totalFightsSimulated++;
    
    size_t leftLost = 0;
    size_t leftArmySize = left.monsterAmount;
    Monster** leftLineup = left.monsters;
    
    int leftFrontDamageTaken = 0;
    int leftHealing = 0;
    int leftCumAoeDamageTaken = 0;
    float leftBerserkProcs = 0;
    
    size_t rightLost = 0;
    size_t rightArmySize = right.monsterAmount;
    Monster** rightLineup = right.monsters;
    
    int rightFrontDamageTaken = 0;
    int rightHealing = 0;
    int rightCumAoeDamageTaken = 0;
    float rightBerserkProcs = 0;
    
    // If no heroes are in the army the result from the smaller army is still valid
    if (left.lastFightData.valid && !verbose) { 
        // Set pre-computed values to pick up where we left off
        leftLost                = leftArmySize-1; // All monsters of left died last fight only the new one counts
        leftFrontDamageTaken    = left.lastFightData.leftAoeDamage;
        leftCumAoeDamageTaken   = left.lastFightData.leftAoeDamage;
        rightLost               = left.lastFightData.monstersLost;
        rightFrontDamageTaken   = left.lastFightData.damage;
        rightCumAoeDamageTaken  = left.lastFightData.rightAoeDamage;
        rightBerserkProcs       = left.lastFightData.berserk;
    }
    
    // Values for skills  
    int damageLeft, damageRight;
    int damageBuffLeft, damageBuffRight;
    int protectionLeft, protectionRight;
    int aoeDamageLeft, aoeDamageRight;
    int paoeDamageLeft, paoeDamageRight;
    int healingLeft, healingRight;
    int pureMonstersLeft, pureMonstersRight;
    int elementalDifference;
    
    // hero temp Variables
    Monster * currentMonsterLeft;
    Monster * currentMonsterRight;
    HeroSkill * skill;
    SkillType skillType;
    Element skillTarget;
    
    while (true) {
        // Get all hero influences
        damageBuffLeft = 0;
        protectionLeft = 0;
        aoeDamageLeft = 0;
        paoeDamageLeft = 0;
        healingLeft = 0;
		pureMonstersLeft = 0;
		for (size_t i = leftLost; i < leftArmySize; i++)
		{
		  if (leftCumAoeDamageTaken >= leftLineup[i]->hp)
		  { // Check for Backline Deaths
			leftLost += (leftLost == i);
		  }
		  else
		  {
			skill = &leftLineup[i]->skill;
			skillType = skill->type;
			skillTarget = skill->target;
			if (skillType == nothing)
				pureMonstersLeft++; // count for friends ability
			else if (skillType == protect && (skillTarget == all || skillTarget == leftLineup[leftLost]->element))
				protectionLeft = (int)(protectionLeft + skill->amount);
			else if (skillType == buff && (skillTarget == all || skillTarget == leftLineup[leftLost]->element))
				damageBuffLeft = (int)(damageBuffLeft + skill->amount);
			else if (skillType == champion && (skillTarget == all || skillTarget == leftLineup[leftLost]->element))
			{
			  damageBuffLeft = (int)(damageBuffLeft + skill->amount);
			  protectionLeft = (int)(protectionLeft + skill->amount);
			}
			else if (skillType == heal)
				healingLeft = (int)(healingLeft + skill->amount);
			else if (skillType == aoe)
				aoeDamageLeft = (int)(aoeDamageLeft + skill->amount);
			else if (skillType == pAoe && i == leftLost)
				paoeDamageLeft += leftLineup[i]->damage;
		  }
		}

		damageBuffRight = 0;
		protectionRight = 0;
        aoeDamageRight = 0;
        paoeDamageRight = 0;
        healingRight = 0;
        pureMonstersRight = 0;
		for (size_t i = rightLost; i < rightArmySize; i++)
		{
		  if (rightCumAoeDamageTaken >= rightLineup[i]->hp)
		  { // Check for Backline Deaths
			rightLost += (i == rightLost);
		  }
		  else
		  {
			skill = &rightLineup[i]->skill;
			skillType = skill->type;
			skillTarget = skill->target;
			if (skillType == nothing)
			  pureMonstersRight++;  // count for friends ability
			else if (skillType == protect && (skillTarget == all || skillTarget == rightLineup[rightLost]->element))
				protectionRight = (int)(protectionRight + skill->amount);
			else if (skillType == buff && (skillTarget == all || skillTarget == rightLineup[rightLost]->element))
				damageBuffRight = (int)(damageBuffRight + skill->amount);
			else if (skillType == champion && (skillTarget == all || skillTarget == rightLineup[rightLost]->element))
			{
			  damageBuffRight = (int)(damageBuffRight + skill->amount);
			  protectionRight = (int)(protectionRight + skill->amount);
			}
			else if (skillType == heal)
			  healingRight = (int)(healingRight + skill->amount);
			else if (skillType == aoe)
			  aoeDamageRight = (int)(aoeDamageRight + skill->amount);
			else if (skillType == pAoe && i == rightLost)
			  paoeDamageRight += rightLineup[i]->damage;
		  }
		}
        
        // Add last effects of abilities and start resolving the turn
        if (leftLost >= leftArmySize || rightLost >= rightArmySize) {
            break; // At least One army was beaten
        }
        
        // Heal everything that hasnt died
        leftFrontDamageTaken -= leftHealing; // these values are from the last iteration
        leftCumAoeDamageTaken -= leftHealing;
        rightFrontDamageTaken -= rightHealing;
        rightCumAoeDamageTaken -= rightHealing;
        if (leftFrontDamageTaken < 0) {
            leftFrontDamageTaken = 0;
        }
        if (leftCumAoeDamageTaken < 0) {
            leftCumAoeDamageTaken = 0;
        }
        if (rightFrontDamageTaken < 0) {
            rightFrontDamageTaken = 0;
        }
        if (rightCumAoeDamageTaken < 0) {
            rightCumAoeDamageTaken = 0;
        }
        
        // Get Base Damage for this Turn
        currentMonsterLeft = leftLineup[leftLost];
        currentMonsterRight = rightLineup[rightLost];
        damageLeft = currentMonsterLeft->damage;
        damageRight = currentMonsterRight->damage;
        
        // Handle Monsters with skills berserk or friends
        if (currentMonsterLeft->skill.type == berserk) {
					damageLeft = (int)(damageLeft * pow(currentMonsterLeft->skill.amount, leftBerserkProcs));
            leftBerserkProcs++;
        } else {
            leftBerserkProcs = 0;
        }
        if (currentMonsterLeft->skill.type == friends) {
					damageLeft = (int)(damageLeft * pow(currentMonsterLeft->skill.amount, pureMonstersLeft));
        }
        
        if (currentMonsterRight->skill.type == berserk) {
					damageRight = (int)(damageRight * pow(currentMonsterRight->skill.amount, rightBerserkProcs));
            rightBerserkProcs++; 
        } else {
            rightBerserkProcs = 0;
        }
        if (currentMonsterRight->skill.type == friends) {
					damageRight = (int)(damageRight * pow(currentMonsterRight->skill.amount, pureMonstersRight));
        }
        
        // Add Buff Damage
        damageLeft += damageBuffLeft;
        damageRight += damageBuffRight;
        
        // Handle Elemental advantage
        elementalDifference = (currentMonsterLeft->element - currentMonsterRight->element);
        if (elementalDifference == -1 || elementalDifference == 3) {
					damageLeft = (int)(damageLeft * elementalBoost);
        } else if (elementalDifference == 1 || elementalDifference == -3) {
					damageRight = (int)(damageRight * elementalBoost);
        }
        
        // Handle Protection
        if (damageLeft > protectionRight) {
            damageLeft -= protectionRight;
        } else {
            damageLeft = 0;
        }
        
        if (damageRight > protectionLeft) {
            damageRight -= protectionLeft;
        } else {
            damageRight = 0; 
        }
        
        // Write values into permanent Variables for the next iteration
        rightFrontDamageTaken += damageLeft + aoeDamageLeft;
        rightCumAoeDamageTaken += aoeDamageLeft + paoeDamageLeft;
        rightHealing = healingRight;
        leftFrontDamageTaken += damageRight + aoeDamageRight;
        leftCumAoeDamageTaken += aoeDamageRight + paoeDamageRight;
        leftHealing = healingLeft;
        
        // Check if the first Monster died (Neccessary cause of the AOE-Piercing-Heal-Interaction)
        if (currentMonsterLeft->hp <= leftFrontDamageTaken) {
            leftLost++;
            leftBerserkProcs = 0;
            leftFrontDamageTaken = leftCumAoeDamageTaken;
        }
        if (currentMonsterRight->hp <= rightFrontDamageTaken) {
            rightLost++;
            rightBerserkProcs = 0;
            rightFrontDamageTaken = rightCumAoeDamageTaken;
        }
        
        // Output detailed fight Data for debugging
        if (verbose) {
            cout << setw(3) << leftLost << " " << setw(3) << leftFrontDamageTaken << " " << setw(3) << rightLost << " " << setw(3) << rightFrontDamageTaken << endl;
        }
    }
    
    // write all the results into a FightResult
    left.lastFightData.dominated = false;
    left.lastFightData.leftAoeDamage = (int16_t)(leftCumAoeDamageTaken);
    left.lastFightData.rightAoeDamage = (int16_t)(rightCumAoeDamageTaken);
    
    if (leftLost >= leftArmySize) { //draws count as right wins. 
        left.lastFightData.rightWon = true;
        left.lastFightData.monstersLost = (int8_t)(rightLost); 
        left.lastFightData.damage = (int16_t)(rightFrontDamageTaken);
        left.lastFightData.berserk = (int8_t)(rightBerserkProcs);
    } else {
        left.lastFightData.rightWon = false;
        left.lastFightData.monstersLost = (int8_t)(leftLost);
        left.lastFightData.damage = (int16_t)(leftFrontDamageTaken);
        left.lastFightData.berserk = (int8_t)(leftBerserkProcs);
    }
}