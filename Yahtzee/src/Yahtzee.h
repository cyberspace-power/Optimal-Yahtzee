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

#include "Database.h"

typedef struct state {
	std::vector<int> dice;
	bool is_new_turn; // Aids getDiceStateId --> if true: return dice state 0 = no roll yet

	// Rest of variables make up state ID number
	unsigned char up_total;
	bool y_bonus_state;
	unsigned short sc_status; //scorecard status --> what sections remain
	char roll_num;
} state;

class Yahtzee {
  public:
	// Constructors:
	Yahtzee(state start_state);
	Yahtzee();

	// Dice Id functions:
	unsigned char getDiceStateId();
	static int getDiceKey(int (&dice_multisets)[10]); // Helper for getDiceStateId()

	// State Id Functions:
	void updateStateId();
	long getStateId();

	// Roll Functions:
	void roll(int kept_dice_state);
	int selectDice(std::string& input);

	// Scoring Funtions:
	int takeSection(int section);
	static int setScoringMapValue(int (&curr_combo)[10]);

	// fill in the whole table
	void initializeTableDiceConfig();
	void initializeTableDiceProbability();
	void initializeTableOutput();  // TODO here's the fun(nest) part

	Database db;
  private:
	long curr_state_id;
	state st;

	// State Id Functions:
	unsigned char setUpperBonusStateId();

	// Scoring Funtions:
	static bool is3OfKind(int (&curr_combo)[10]);
	static bool is4OfKind(int (&curr_combo)[10]);
	static bool isFullHouse(int (&curr_combo)[10]);
	static bool isSmallStraight(int (&curr_combo)[10]);
	static bool isLargeStraight(int (&curr_combo)[10]);
	static bool isYahtzee(int (&curr_combo)[10]);
	bool isJoker(const diceConfig &dc, int section); // Helper for takeSection()
	bool isSectionTaken(int section); // Helper for takeSection()

	// Table setters
	void setDiceConfigTables(int freq_left, int min, int curr_index, int (&curr_combo)[10], int &combo_count);
	void setKeptDice(int num_of_dice, int freq_left, int min, int curr_index, int (&curr_combo)[10]);
	void setDiceProbTable(int num_of_dice, int freq_left, int min, int curr_index, int (&roll_curr_combo)[10],
			int (&kept_curr_combo)[10]);
	// TODO setOutputTable()

	// Helpers for table setters:
	int getNumerator(int num_of_dice, const int (&roll_curr_combo)[10]); // helps DiceProb
	static int factorial(int x); // helps DiceProb
	int combineAndGetDiceId(const int (&roll_curr_combo)[10], const int (&kept_curr_combo)[10]); // helps DiceProb and Output
	void reduce(int &num, int &den);
};

#endif /* YAHTZEE_H */
