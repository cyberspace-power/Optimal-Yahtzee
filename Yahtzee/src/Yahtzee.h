//============================================================================
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
} state;

typedef struct state2 {
	unsigned int up_bonus;
	bool y_bonus;
	unsigned short sc_status; //scorecard status --> what sections remain
	char roll_num;
} state2;

class Yahtzee {
  public:
	Yahtzee(state start_state);
	Yahtzee();
	void setDiceMap(int freq_left, int min, int curr_index, int (&curr_combo)[10], int &combo_count);
	long getStateId();
	void setStateId(state &s);
	int getDiceKey(int (&dice_multisets)[10]); // Helper for getDiceStateId()
	unsigned char getDiceStateId(state &s);
	char setUpperBonusStateId();
	void roll(std::string& kept_dice);
	int selectDice(std::string& input);
	int takeSection();

  private:
	int up_total;
	long curr_state_id;
	state st;
	int score;
	std::unordered_map<int, int> dice_state_map;

};



#endif /* YAHTZEE_H_ */
