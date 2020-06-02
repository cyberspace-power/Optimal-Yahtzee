//============================================================================
// Name        : Yahtzee.cpp
// Author      : Gabriel Simmons, Austin Sullivan
// Version     : 0.1
// Copyright   : Your copyright notice
// Description : Regular Yahtzee Functions
//============================================================================

#include <iostream>
#include <bitset>
#include "Yahtzee.h"

// Begins a game from specified state
Yahtzee::Yahtzee(state * start_state) {
	st = start_state;
	curr_state_id = !st ? (0x1 << 16) : 0; //Start state of new game
	setStateId(st);
	// print state in bits to diagnose any errors
	std::bitset<64> b(curr_state_id);
	std::cout << "Current State ID = " << b << std::endl;
}

// Begins a game from the beginning. Do not jump to specific state.
Yahtzee::Yahtzee() : up_total(0), curr_state_id(1 << 16), st(nullptr) {}

Yahtzee::~Yahtzee() {
	//delete s; // Is this even necessary?
}

/*
 * Takes in current state and sets the current state id
 * Condenses all game states into one general game state
 * Does this by compressing states like the following:
 * Scorecard state | Roll # | Dice state | Upper Bonus State | Yahtzee Bonus State
 *     16 bits     | 8 bits |   8 bits   |      7 bits       |        1 bit
 * Note that the two bonus states are combined into the last byte.
 * These states are strategically organized such that the probability calculations
 * later will be done close to the order of the states in a sorted state table
 */
void Yahtzee::setStateId(state * s) {
	curr_state_id = (s->sc_status << 24) | (s->roll_num << 16) | (getDiceStateId(s) << 8) |
			(s->up_bonus << 1) | (s->y_bonus);
	return;
}

/*
 * Organize dice state into multiset form such that it can be condensed
 * to 8 bits. Use hashmap or dict(?) lookup to quickly determine it's id.
 * Return possible results of 0-252 as a char (8 bits).
 */
char Yahtzee::getDiceStateId(state * s) {
	return 0;
}

char Yahtzee::setUpperBonusStateId() {
	// If bonus has already been achieved, count no higher than 63
	return up_total >= 63 ? 63 : up_total;
}

long Yahtzee::getStateId() {
	return curr_state_id;
}

int main() {
	//std::cout << "Welcome to the yahtzee program!" << std::endl; // prints Hello World!!!
	std::vector<char> d(5, 1);
	state * test = new state;
	test->sc_status = 2; // Only taken yahtzee
	test->roll_num = 4; // Entering first roll of second turn
	test->dice = d;
	test->up_bonus = 0;
	test->y_bonus = 1; // Yahtzee bonus is available

	Yahtzee y(test);
	delete test;
	return 0;
}
