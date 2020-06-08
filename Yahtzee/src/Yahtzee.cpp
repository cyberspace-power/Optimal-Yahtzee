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
#include <cstdlib> // rand, atoi
#include <ctime> // time() used to seed RNG
#include <string>
#include <unordered_map>
#include "Yahtzee.h"

// Begins a game from specified state
Yahtzee::Yahtzee(state start_state) {
	score = 0;
	st = start_state;

	// There will be 252 dice states (including null roll). Set map to
	// be ready for 252 entries. Reduces rehashing that can slow speed
	dice_state_map.reserve(252);
	int curr_combo[10] = {0,0,0,0,0,0,0,0,0,0}; // Bug fix: SET ARRAY!!!
	int combo_count = 1;
	setDiceMap(5, 1, 0, curr_combo, combo_count); // Sets the dice state unordered map
	curr_state_id = !st ? (0x1 << 28) : 0; //Start state of new game
	setStateId(st);
	// print state in bits to diagnose any errors
	std::bitset<34> b(curr_state_id);
	std::cout << "Current State ID = " << b << std::endl;
	srand(time(0)); // Set seed for RNG

	// Set up dice state mapping:
}

// Begins a game from the beginning. Do not jump to specific state.
Yahtzee::Yahtzee() {
	up_total = 0;
	score = 0;

	// There will be 252 dice states (including null roll). Set map to
	// be ready for 252 entries. Reduces rehashing that can slow speed
	dice_state_map.reserve(252);
	int curr_combo[10] = {0,0,0,0,0,0,0,0,0,0}; // Bug fix: SET ARRAY!!!
	int combo_count = 0;
	setDiceMap(5, 1, 0, curr_combo, combo_count); // Sets the dice state unordered map
	std::cout << "Dice state map size = " << dice_state_map.size() << "\n";
	curr_state_id = 1 << 28; // Initial state at beginning of new game
	std::vector<int> d(5,1);
	st.dice = d;
	st.roll_num = 1;
	st.sc_status = 0;
	st.up_bonus = 0;
	st.y_bonus = 0;
	srand(time(0)); // Set seed for RNG
}

/*
 * Purpose: Set the dice state map to allow for constant time lookups
 *    The unordered_map from STL is used to track this mapping
 * Style: Recursive function that iterates through all possible
 *    dice combinations and maps them to the number of mapped combos
 *    to that point (i.e. A number between 1-252)
 */
void Yahtzee::setDiceMap(int freq_left, int min, int curr_index, int (&curr_combo)[10], int &combo_count) {
	for(int i = min; i <= 6; i++) {
		curr_combo[curr_index] = i;
		for(int j = freq_left; j > 0; j--) {
			curr_combo[curr_index + 1] = j;
			if(j == freq_left) { // No more dice can be added to combo. Map it!
				if(dice_state_map.find(getDiceKey(curr_combo)) != dice_state_map.end())
					std::cout << "Error -- Element already exists\n";
				dice_state_map[getDiceKey(curr_combo)] = combo_count;

				// Print for testing purposes
				std::cout << combo_count << std::endl;
				for(int i = 0; i <= curr_index; i += 2) {
					std::cout << curr_combo[i] << ": " << curr_combo[i+1] << std::endl;
				}
				combo_count++;
			}
			else if(min == 6 && j < freq_left) // No higher number than 6 is possible. Break!
				break;
			else // recurse to next highest number and place in combo array
				setDiceMap(freq_left-j, i+1, curr_index+2, curr_combo, combo_count);
		}
	}
	// About to return to previous recursion. Reset array values to 0
	curr_combo[curr_index] = 0;
	curr_combo[curr_index + 1] = 0;
	return;
}

/*
 * Takes in current state and sets the current state id
 * Condenses all game states into one general game state
 * Does this by compressing states like the following:
 *  Roll # | Scorecard state | Dice state | Upper Bonus State | Yahtzee Bonus State
 *  6 bits |     13 bits     |   8 bits   |      6 bits       |        1 bit	   = 34 bits
 * These states are strategically organized such that the probability calculations
 * later will be done close to the order of the states in a sorted state table
 */
void Yahtzee::setStateId(state &s) {
	curr_state_id = ((long)s.sc_status << 21) | ((long)s.up_bonus << 15) | ((long)s.y_bonus << 14) |
				((long)s.roll_num << 8) |  ((long)getDiceStateId(s));
	return;
}

/*
 * Helper function for getDiceStateId
 * Simply calculates and returns key for the unordered mapping
 */
int Yahtzee::getDiceKey(int (&dice_multisets)[10]) {
	// Since no number in either array will be greater than 6, the max of 10
	// numbers will all fit in 3 bits. 3*10 = 30 bits < 32 bit integer.
	int key = 0;
	for(int i = 0; i < 5; i++)
		key |= (dice_multisets[i*2] << ((i*6) + 3)) | (dice_multisets[i*2 + 1] << (i*6));
	// For testing purposes
	/*std::bitset<30> bit(key);
	std::cout << "Key = " << key << std::endl;
	std::cout << "Key(binary) = " << bit << std::endl;*/

	return key;
}

/*
 * Organize dice state into multiset form such that it can be condensed
 * to 8 bits. Use hashmap or dict(?) lookup to quickly determine it's id.
 * Return possible results of 0-252 as an unsigned char (8 bits).
 */
unsigned char Yahtzee::getDiceStateId(state &s) {
	if(s == nullptr) // When turn ends, takeSection passes in nullptr
		return 0; // Dice state 0 represents beginning of new turn (no dice roll yet)
	//Add null roll check
	std::vector<int> dice_copy = s.dice; // Create copy of dice because dice must be sorted
	std::sort(dice_copy.begin(), dice_copy.end());
	// First put dice number and frequencies into arrays:
	int dice_multisets[10] = {0,0,0,0,0,0,0,0,0,0};
	int numbers = -1;
	for(int i = 0; i < 5; i++) {
		if(i > 0 && dice_copy[i] == dice_copy[i-1]) {
			dice_multisets[numbers*2 + 1]++;
		}
		else {
			numbers++;
			dice_multisets[numbers*2] = dice_copy[i];
			dice_multisets[numbers*2 + 1]++;
		}
	}
	// For testing purposes
	for(int i = 0; i < 5; i++)
		std::cout << dice_multisets[i*2] << ": " << dice_multisets[i*2 + 1] << std::endl;

	int key = getDiceKey(dice_multisets);
	std::cout << key << "\n";
	return dice_state_map.at(key);
}

char Yahtzee::setUpperBonusStateId() {
	// If bonus has already been achieved, count no higher than 63
	return up_total >= 63 ? 63 : up_total;
}

long Yahtzee::getStateId() {
	return curr_state_id;
}

/*
 * Rolls non-kept dice. The kept_dice parameter is the same as the input
 * parameter from the selectDice function (see below)
 */
void Yahtzee::roll(std::string& kept_dice) {
	bool is_roll[5] = {true, true, true, true, true};
	for(unsigned int i = 0; i < kept_dice.size(); i++)
		is_roll[(int)kept_dice[i] - 49] = false; // convert char to int and subtract 49 for correct integer
	for(int i = 0; i < 5; i++) {
		if(is_roll[i])
			st.dice.at(i) = (rand() % 6) + 1; // Generate number between 1 and 6
	}

	// For testing purposes
	std::cout << "Roll: ";
	for(int i = 0; i < 5; i++) {
		if(is_roll[i])
			std::cout << st.dice[i] << ", ";
		else
			std::cout << st.dice[i] << "*, ";
	}
	std::cout << std::endl;
	getDiceStateId(st);
	return;
}

/*
 * This function exists to sanitize input the user inputs for
 * dice they want to keep. Invalid inputs return -1, valid ones return 0.
 * This sanitized input (passed by reference to allow changes to the input)
 * is intended to be passed directly to roll.
 */
int Yahtzee::selectDice(std::string& input) { 										// Return kept dice state ID number (0-31)
	// If getline is used to collect input, it adds null terminated characters
	// Following if statement gets rid of it
	if(input[input.size() - 1] < 32)
		input = input.substr(0, input.size() - 1);
	// If no input is selected, return normally and all dice will be rolled
	if(input.size() == 0)
		return 0;
	//
	if(input.size() > 4) {
		std::cout << "Your input is too large. You can select no more than 4 dice to keep.\n"
				<< "If you want to keep all the dice, then please select a section to take.\n";
		return -1;
	}
	// First, put all dice intended to keep into sorted vector of 1-4 values
	std::vector<char> dice_to_keep;
	for(unsigned int i = 0; i < input.size(); i++)
		dice_to_keep.push_back(input[i]);
	// Sort in ascending order so the last dices' values are not accidentally eliminated
	// See comment later in this function
	std::sort(dice_to_keep.begin(), dice_to_keep.end());

	// Next, check for invalid inputs (characters other than numbers 1-5 or duplicated characters)
	if(dice_to_keep[0] < '1' || dice_to_keep[dice_to_keep.size() - 1] > '5') {
		//std::cout << (int)dice_to_keep[0] << " " << (int)dice_to_keep[dice_to_keep.size() - 1] << std::endl;
		std::cout << "Invalid input. Please only enter characters between 1 and 5.\n";
		return -1;
	}
	for(unsigned int i = 0; i < dice_to_keep.size(); i++) {
		if(i > 0 && dice_to_keep[i] == dice_to_keep[i-1]) {
			std::cout << "Invalid input. Please do not duplicate values. Was this in error? Try again.\n";
			return -1;
		}
	}
	return 0;
}

int main() {
	//std::cout << "Welcome to the yahtzee program!" << std::endl; // prints Hello World!!!
	std::vector<int> d(5, 1);
	state test;
	test.sc_status = 8190; // Only taken yahtzee
	test.roll_num = 62; // Entering first roll of second turn
	test.dice = d;
	test.up_bonus = 62;
	test.y_bonus = 1; // Yahtzee bonus is available

	Yahtzee y(test);
	std::string temp;
	for(int i = 0; i < 10; i++) {
		std::cout << "Hello!" << std::endl;
		y.roll(temp);
		int selected_dice = -1;
		while(selected_dice == -1) {
			std::cout << "\nSelect which dice to keep: ";
			std::getline(std::cin, temp);
			selected_dice = y.selectDice(temp);
		}
	}
	return 0;
}
