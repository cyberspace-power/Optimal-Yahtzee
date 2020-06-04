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

class Yahtzee {
  public:
	Yahtzee(state * start_state);
	Yahtzee();
	~Yahtzee();
	long getStateId();
	void setStateId(state * s);
	unsigned char getDiceStateId(state * s);
	char setUpperBonusStateId();
	void roll(std::string& kept_dice);
	int selectDice(std::string& input);
	void takeSection();

  private:
	int up_total;
	long curr_state_id;
	state * st;
	int score;
	std::unordered_map<int, int> dice_state_map;

};



#endif /* YAHTZEE_H_ */
