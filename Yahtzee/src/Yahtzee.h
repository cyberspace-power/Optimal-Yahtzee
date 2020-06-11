//================-============================================================
// Name        : Yahtzee.h
// Author      : Gabriel Simmons, Austin Sullivan
// Version     : 0.1
// Copyright   : Your copyright notice
// Description : Header file for Yahtzee.h
//============================================================================

#ifndef YAHTZEE_H
#define YAHTZEE_H

#include <vector>
#include <iostream>
#include <string>
#include <unordered_map>

typedef struct state {
	unsigned int up_bonus;
	bool y_bonus;
	unsigned short sc_status; //scorecard status --> what sections remain
	char roll_num;
	std::vector<int> dice;
	bool is_new_turn; // Aids getDiceStateId --> if true: return dice state 0 = no roll yet
} state;

typedef struct scorecard {
	// Sections
	int ones = -1;
	int twos = -1;
	int threes = -1;
	int fours = -1;
	int fives = -1;
	int sixes = -1;
	int three_of_kind = -1;
	int four_of_kind = -1;
	int full_house = -1;
	int small_straight = -1;
	int large_straight = -1;
	int yahtzee = -1;
	int chance = -1;
	// Bonuses
	int upper_bonus = 0;
	int yahtzee_bonus = 0;
} scorecard;

class Yahtzee {
  public:
	// Constructors:
	Yahtzee(state start_state);
	Yahtzee();

	void setDiceMap(int freq_left, int min, int curr_index, int (&curr_combo)[10], int &combo_count);
	int getDiceKey(int (&dice_multisets)[10]); // Helper for getDiceStateId()
	unsigned char getDiceStateId();
	// State Id Functions:
	long getStateId();
	void setStateId();
	char setUpperBonusStateId();

	// Roll Functions:
	void roll(int kept_dice_state);
	int selectDice(std::string& input);

	// Scoring Funtions:
	int takeSection(int section);

  private:
	int up_total;
	long curr_state_id;
	state st;
	int score;
	std::unordered_map<int, int> dice_state_map;

};



#endif /* YAHTZEE_H_ */
