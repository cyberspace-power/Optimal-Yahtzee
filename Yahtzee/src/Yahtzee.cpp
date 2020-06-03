//============================================================================
// Name        : Yahtzee.cpp
// Author      : Gabriel Simmons, Austin Sullivan
// Version     : 0.1
// Copyright   : Your copyright notice
// Description : Regular Yahtzee Functions
//============================================================================

#include <iostream>
#include <bitset> // print out bits
#include <algorithm> // sort
#include <cstdlib> // rand
#include <ctime> // time() used to seed RNG
#include <string>
//#include <bits/stdc++.h>
#include "Yahtzee.h"

// Begins a game from specified state
Yahtzee::Yahtzee(state * start_state) {
	st = start_state;
	curr_state_id = !st ? (0x1 << 16) : 0; //Start state of new game
	setStateId(st);
	// print state in bits to diagnose any errors
	std::bitset<64> b(curr_state_id);
	std::cout << "Current State ID = " << b << std::endl;
	srand(time(0)); // Set seed for RNG
}

// Begins a game from the beginning. Do not jump to specific state.
Yahtzee::Yahtzee() : up_total(0), curr_state_id(1 << 16), st(nullptr) {
	srand(time(0)); // Set seed for RNG
}

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

void Yahtzee::roll(int num_to_roll) {
	if(num_to_roll > 5 || num_to_roll < 1) {
		std::cout << "You cannot roll this many dice! (" << num_to_roll << ")\n";
		return;
	}
	for(int i = 0; i < num_to_roll; i++)
		st->dice.at(i) = (rand() % 6) + 1; // Generate number between 1 and 6
	std::sort(st->dice.begin(), st->dice.end());

	// For testing purposes
	for(int i = 0; i < num_to_roll; i++)
		std::cout << "Dice " << i << " = " << st->dice[i] << "; ";
	std::cout << std::endl;
	return;
}


int Yahtzee::selectDice(std::string input) {
	if(input.size() == 0) {
		std::cout << "Please input which dice to keep\n";
		return -1;
	}
	int num_kept = 0;
	// First, put all dice intended to keep into sorted vector of 1-4 values
	std::vector<char> dice_to_keep;
	for(unsigned int i = 0; i < input.size(); i++)
		dice_to_keep.push_back(input[i]);
	// Sort in ascending order so the last dices' values are not accidentally eliminated
	// See comment later in this function
	std::sort(dice_to_keep.begin(), dice_to_keep.end(), std::greater<char>());

	// Next, check for invalid inputs (characters other than numbers 1-5 or duplicated characters)
	if(dice_to_keep[0] < '1' || dice_to_keep[dice_to_keep.size() - 1] > '5') {
		std::cout << "Invalid input. Please only enter characters between 1 and 5.\n";
		return -1;
	}
	for(unsigned int i = 0; i < dice_to_keep.size(); i++) {
		if(i > 0 && dice_to_keep[i] == dice_to_keep[i-1]) {
			std::cout << "Invalid input. Please do not duplicate values. Was this in error? Try again.\n";
			return -1;
		}
	}

	// Finally, if input is valid then save the kept dice at the back of the dice list
	// Kept dice are saved at the back so the roll function does not overwrite them.
	for(unsigned int i = 0; i < dice_to_keep.size(); i++) {
		// curr - 49 will yield a number 1-5
		st->dice[5-num_kept] = st->dice[dice_to_keep[i] - 49]; // 49 is ascii value '1'
		num_kept++;
	}
	return num_kept;
}

int main() {
	//std::cout << "Welcome to the yahtzee program!" << std::endl; // prints Hello World!!!
	std::vector<int> d(5, 1);
	state * test = new state;
	test->sc_status = 2; // Only taken yahtzee
	test->roll_num = 4; // Entering first roll of second turn
	test->dice = d;
	test->up_bonus = 0;
	test->y_bonus = 1; // Yahtzee bonus is available

	Yahtzee y(test);
	for(int i = 0; i < 10; i++)
		y.roll(5);
	delete test;
	return 0;
}
