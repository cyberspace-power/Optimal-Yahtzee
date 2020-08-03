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
#include <numeric>
#include <algorithm>
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

class Fraction {
  public:
	unsigned int num;
	unsigned int den;

	Fraction(unsigned int num, unsigned int den) : num(num), den(den) {
		reduce();
	}

	// Add two fractions together
	Fraction add(Fraction& other) {
		unsigned int lcm = std::lcm(other.den, den); // Get the least common multiple
		unsigned int this_den_factor = lcm / den; // calculates denom factor to reach LCM
		unsigned int other_den_factor = lcm / other.num; // Ditto as above line

		// Multiply this fraction's num by denom factor to reach LCM
		num *= this_den_factor;
		// Do the same for the other fraction
		other.num *= other_den_factor;
		return Fraction(num + other.num, lcm);
	}

	// Overload + operator
	Fraction operator+(Fraction& other) {
		return add(other);
	}

	void operator+=(Fraction& other) {
		Fraction temp = add(other);
		num = temp.num;
		den = temp.den;
	}

	// Multiply two fractions together
	Fraction multiply(Fraction& other) {
		Fraction fr(num * other.num, den * other.den);
		fr.reduce();
		return fr;
	}

	// Overload * operator
	Fraction operator*(Fraction& other) {
		return multiply(other);
	}

	void operator*=(Fraction& other) {
		Fraction temp = multiply(other);
		num = temp.num;
		den = temp.den;
	}

  private:
	void reduce() {
		unsigned int gcd = std::__gcd(num, den);
		num /= gcd;
		den /= gcd;
		return;
	}

};

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
	void Yahtzee::fillKeptDiceArray(int (&roll_curr_combo)[10], int dice_id, int kept_dice_key);
	Fraction getKeptStateExpValue(int num_of_dice, int freq_left, int min, int curr_index, int (&roll_curr_combo)[10],
			int (&kept_curr_combo)[10], int state);

};

#endif /* YAHTZEE_H */
