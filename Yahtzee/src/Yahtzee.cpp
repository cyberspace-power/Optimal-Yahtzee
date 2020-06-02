//============================================================================
// Name        : Yahtzee.cpp
// Author      : Gabriel Simmons, Austin Sullivan
// Version     : 0.1
// Copyright   : Your copyright notice
// Description : Regular Yahtzee Functions
//============================================================================

#include <iostream>
#include <bitset>
//#include <stdio.h>
#include "Yahtzee.h"

Yahtzee::Yahtzee(state * start_state) {
	st = start_state;
	curr_state_id = !st ? (0x1 << 16) : 0; //Start state of new game
	setStateId(st);
	// print state in bits to diagnose any errors
	std::bitset<64> b(curr_state_id);
	std::cout << "Current State ID = " << b << std::endl;
}

Yahtzee::Yahtzee() : up_total(0), curr_state_id(1 << 16), st(nullptr) {}

Yahtzee::~Yahtzee() {
	//delete s; // Is this even necessary?
}

void Yahtzee::setStateId(state * s) {
	curr_state_id = (s->sc_status << 24) | (s->roll_num << 16) | (getDiceStateId(s) << 8) |
			(s->up_bonus << 1) | (s->y_bonus);
	return;
}

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
