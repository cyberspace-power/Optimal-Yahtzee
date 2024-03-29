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
#include <cmath>

#include "Yahtzee.h"
#include "Database.h"

// Begins a game from specified state
Yahtzee::Yahtzee(state start_state) {
	// set up database
	std::string filename = "test.db";
	db.setFilename(filename);
	initializeTableDiceConfig();
	initializeTableDiceProbability();

	st = start_state;
	if(st.y_bonus_state && (((st.sc_status>>11) & 1) != 1)) {
		// Makes sure user didn't accidentally set y_bonus to available if yahtzee has not been taken
		st.y_bonus_state = false;
		std::cout << "Had to reset y_bonus to false\n";
	}
	updateStateId();
	// print state in bits to diagnose any errors
	std::bitset<64> b(curr_state_id);
	std::cout << "Current State ID = " << b << std::endl;
	srand(time(0)); // Set seed for RNG

	// Set up dice state mapping:
}

// Begins a game from the beginning. Do not jump to specific state.
Yahtzee::Yahtzee() {
	// set up database
	std::string filename = "test.db";
	db.setFilename(filename);
	initializeTableDiceConfig();
	initializeTableDiceProbability();

	curr_state_id = 1 << 8; // Initial state at beginning of new game
	std::vector<int> d(5,1);
	st.dice = d;
	st.roll_num = 1;
	st.sc_status = 0;
	st.up_total = 0;
	st.y_bonus_state = 0;
	st.is_new_turn = true;
	srand(time(0)); // Set seed for RNG
}

/*
 * Helper function for getDiceStateId
 * Simply calculates and returns key for the unordered mapping
 */
int Yahtzee::getDiceKey(int (&dice_multisets)[10]) {
	// Since no number in either array will be greater than 6, the max of 10
	// numbers will all fit in 3 bits. 3*10 = 30 bits < 32 bit integer.
	int key = 0;
	for(int i = 0; i < 5 && dice_multisets[i*2] != 0; i++)
		key |= (dice_multisets[i*2] << ((i*6) + 3)) | (dice_multisets[i*2 + 1] << (i*6));
	return key;
}

/*
 * Organize dice state into multiset form such that it can be condensed
 * to 8 bits. Use hashmap or dict(?) lookup to quickly determine it's id.
 * Return possible results of 0-252 as an unsigned char (8 bits).
 */
unsigned char Yahtzee::getDiceStateId() {
	if(st.is_new_turn)
		return 0; // No roll yet this turn. Return empty dice state (Only happens after taking section)
	std::vector<int> dice_copy = st.dice; // Create copy of dice because dice must be sorted
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
	int key = getDiceKey(dice_multisets);
	diceConfig dc_data;
	dc_data.dice_key = key;
	db.selectDiceConfig(&dc_data, true);
	return dc_data.dice_id;
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
void Yahtzee::updateStateId() {
	curr_state_id = ((long)st.sc_status << 21) | ((long)setUpperBonusStateId() << 15) | ((long)st.y_bonus_state << 14) |
				((long)st.roll_num << 8) |  ((long)getDiceStateId());
	return;
}

unsigned char Yahtzee::setUpperBonusStateId() {
	// If bonus has already been achieved, count no higher than 63
	return st.up_total >= 63 ? 63 : st.up_total;
}

long Yahtzee::getStateId() {
	return curr_state_id;
}

/*
 * Rolls non-kept dice. The kept_dice parameter is the same as the input
 * parameter from the selectDice function (see below)
 */
void Yahtzee::roll(int kept_dice_state) {
	// If is_new_turn is true, then the takeSection method will have already changed the roll
	// number. However, we must still reset the new_turn bool value to false. Otherwise, we
	// are in the middle of a turn, and must increment the roll number
	if(st.is_new_turn)
		st.is_new_turn = false; // If you are rolling, the turn has begun or is continuing
	else
		st.roll_num++;
	bool is_roll[5] = {true, true, true, true, true};
	for(unsigned int i = 0; i < 5; i++) {
		if(kept_dice_state & (1 << i)) // if ith bit is true, that dice is held
			is_roll[i] = false;
	}
	for(int i = 0; i < 5; i++) {
		if(is_roll[i])
			st.dice.at(i) = (rand() % 6) + 1; // Generate number between 1 and 6
	}

	// For testing purposes
	std::cout << "\nRoll: ";
	for(int i = 0; i < 5; i++) {
		if(is_roll[i])
			std::cout << st.dice[i] << ", ";
		else
			std::cout << st.dice[i] << "*, ";
	}
	std::cout << std::endl;
	updateStateId();
	return;
}

/*
 * This function exists to sanitize input the user inputs for
 * dice they want to keep. Invalid inputs return -1, valid ones return a number between 0-30.
 * This sanitized input (passed by reference to allow changes to the input)
 * is intended to be passed directly to roll.
 */
int Yahtzee::selectDice(std::string& input) {
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
	int kept_dice_state = 0;

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
		kept_dice_state |= (1 << (dice_to_keep[i] - 49)); // bits 1-5 represent dice kept
	}
	return kept_dice_state;
}

/*
 * If the frequency of any of the first three numbers is 3 or more: true
 * Do not need to check after first three dice numbers because if more
 * than three dice numbers exist in the dice state, it cannot be a 3 of a kind.
 */
bool Yahtzee::is3OfKind(int (&curr_combo)[10]) {
	return (curr_combo[1] >= 3 || curr_combo[3] >= 3 || curr_combo[5] >= 3);
}

/*
 * If the frequency of any of the first two numbers is 4 or more: true
 * Do not need to check after first two dice numbers because if more
 * than two dice numbers exist in the dice state, it cannot be a 4 of a kind.
 */
bool Yahtzee::is4OfKind(int (&curr_combo)[10]) {
	return (curr_combo[1] >= 4 || curr_combo[3] >= 4);
}

/*
 * if there are no more than 2 dice numbers AND the frequency of one of the
 * numbers is 2 or 3, then the other number must be the other of 2 or 3 frequency,
 * meaning that the result is a full house
 */
bool Yahtzee::isFullHouse(int (&curr_combo)[10]) {
	return (curr_combo[4] == 0 && (curr_combo[1] == 2 || curr_combo[1] == 3));
}

// if the 1st and 4th OR 2nd and 5th are 3 apart: true
bool Yahtzee::isSmallStraight(int (&curr_combo)[10]) {
	return (curr_combo[0] + 3 == curr_combo[6] || curr_combo[2] + 3 == curr_combo[8]);
}

// if the 1st and 5th dice numbers are 4 apart: true
bool Yahtzee::isLargeStraight(int (&curr_combo)[10]) {
	return (curr_combo[0] + 4 == curr_combo[8]);
}

// if the frequency of the only number is 5: true
bool Yahtzee::isYahtzee(int (&curr_combo)[10]) {
	return (curr_combo[1] == 5);
}

/*
 * Return integer to map to dice state id to allow quick scoring lookups
 * Does this by compressing states like the following:
 * Dice Sum | isYahtzee | isLS | isSS | isFH | is4oK | is3oK |   6s   |   5s   |   4s   |   3s   |   2s   |   1s   |
 *  5 bits  |   1 bit   | 1 bit| 1 bit| 1 bit| 1 bit | 1 bit | 3 bits | 3 bits | 3 bits | 3 bits | 3 bits | 3 bits |
 *  = 29 total bits < 32 bits --> Fits in an integer
 */
int Yahtzee::setScoringMapValue(int (&curr_combo)[10]) {
	int sum = 0;
	int score_data = 0; // Return value
	for(int i = 0; i < 10 && curr_combo[i] != 0; i += 2) {
		sum += (curr_combo[i] * curr_combo[i+1]);
		// Left shift the frequency of dice number to the left 3*dice number
		score_data |= (curr_combo[i+1] << (3*(curr_combo[i] - 1)));
	}

	score_data |= (sum << 24) | ((int)isYahtzee(curr_combo) << 23) | ((int)isLargeStraight(curr_combo) << 22) |
			((int)isSmallStraight(curr_combo) << 21) | ((int)isFullHouse(curr_combo) << 20) |
			((int)is4OfKind(curr_combo) << 19) | ((int)is3OfKind(curr_combo) << 18);

	return score_data;
}

/*
 * Purpose: Helper for takeSection(). Determines whether a section will count for
 * 		normal point due to the yahtzee joker rule. Also used to determine whether
 * 		the 100 point bonus should be applied
 * Returns: True if lower section can be taken for full point total, false otherwise
 */
bool Yahtzee::isJoker(const diceConfig &dc, int section) {
	// Joker rule doesn't apply unless roll is yahtzee and yahtzee has been taken for 0 or 50 points
	if(!isSectionTaken(12) || !dc.is_yahtzee)
		return false;
	// Determine what number the yahtzee is in
	int yahtzee_dice_num = 0;
	if(dc.num_1s == 5) yahtzee_dice_num = 1;
	else if(dc.num_2s == 5) yahtzee_dice_num = 2;
	else if(dc.num_3s == 5) yahtzee_dice_num = 3;
	else if(dc.num_4s == 5) yahtzee_dice_num = 4;
	else if(dc.num_5s == 5) yahtzee_dice_num = 5;
	else if(dc.num_6s == 5) yahtzee_dice_num = 6;

	// You are not allowed to take lower section with the joker rule unless the corresponding upper
	// section total of the yahtzee dice value has already been taken. This checks for that
	switch(yahtzee_dice_num)
	{
	case 1: // Yahtzee on ones
		return (!isSectionTaken(1) && section != 1) ? false : true;
	case 2: // Yahtzee on twos
		return (!isSectionTaken(2) && section != 2) ? false : true;
	case 3: // Yahtzee on threes
		return (!isSectionTaken(3) && section != 3) ? false : true;
	case 4: // Yahtzee on fours
		return (!isSectionTaken(4) && section != 4) ? false : true;
	case 5: // Yahtzee on fives
		return (!isSectionTaken(5) && section != 5) ? false : true;
	case 6: // Yahtzee on sixes
		return (!isSectionTaken(6) && section != 6) ? false : true;
	default:
		std::cout << "Error -- what was yahtzee on?\n";
		return false;
	}
}

bool Yahtzee::isSectionTaken(int section) {
	/*std::bitset<16> b(curr_state_id>>21);
	std::cout << "Scorecard State = " << b << std::endl;*/
	return ((curr_state_id>>(21 + (section-1))) & 1);
}

/*
 * Parameter: Section is a number 1-13 representing scorecard section
 * This function will check if section has already been taken and if not,
 * will "take" that section for the user, granting the appropriate number
 * of points. The return value will return the number of points scored,
 * or -1 upon failure (section has already been taken or input number is bad)
 */
int Yahtzee::takeSection(int section) {
	if(section < 1 || section > 13) {
		std::cout << "Please enter a valid number 1-13 corresponding to a section you have not yet taken.\n";
		return -1;
	}
	else if(isSectionTaken(section)) {
		std::cout << "You have already taken this section. Try again\n";
		return -1;
	}
	else
		st.sc_status |= 1<<(section-1);

	diceConfig dc;
	dc.dice_id = (unsigned short) getDiceStateId();
	db.selectDiceConfig(&dc, false);
	//int scoring_info = dice_scoring_map[curr_state_id & 0xff]; // First get scoring info mapped by dice state ID

	bool is_bonus_achieved = (setUpperBonusStateId() == 63) ? true : false;
	int score = 0;

	switch(section)
	{
	  case 1: // Ones
		score = section * dc.num_1s;
		st.up_total += score;
		break;
	  case 2: // Twos
	    score = section * dc.num_2s;
		st.up_total += score;
		break;
	  case 3: // Threes
		score = section * dc.num_3s;
		st.up_total += score;
		break;
	  case 4: // Fours
	    score = section * dc.num_4s;
		st.up_total += score;
		break;
	  case 5: // Fives
		score = section * dc.num_5s;
		st.up_total += score;
		break;
	  case 6: // Sixes
		score = section * dc.num_6s;
		st.up_total += score;
		break;
	  case 7: // Three of a Kind
		score = dc.is_3_of_a_kind ? dc.sum : 0;
		break;
	  case 8: // Four of a Kind
		score = dc.is_4_of_a_kind ? dc.sum : 0;
		break;
	  case 9: // Full House
		if(dc.is_full_house || isJoker(dc, section)) {
			score = 25;
		}
		else
			score = 0;
		break;
	  case 10: // Small Straight
		if(dc.is_small_straight || isJoker(dc, section)) {
			score = 30;
		}
		else
			score = 0;
		break;
	  case 11: // Large Straight
		if(dc.is_large_straight || isJoker(dc, section)) {
			score = 40;
		}
		else
			score = 0;
		break;
	  case 12: // Yahtzee
		if(dc.is_yahtzee) {
			score = 50;
			st.y_bonus_state = true;
		}
		else
			score = 0;
		break;
	  case 13: // Chance
		score = dc.sum; // Sum of dice
		break;
	}
	// Add upper section bonus
	if(st.up_total >= 63 && !is_bonus_achieved) { //&& sc.upper_bonus == 0) {
		score += 35;
	}
	// Add yahtzee joker bonuses
	if(st.y_bonus_state && isJoker(dc, section)) {
		score += 100;
	}

	// Handle current state stuff
	// Since we start a new turn, move the roll number to the beginning of
	// the next turn. The following equation ensures that will always be a
	// number where x % 3 = 1.
	st.roll_num = (st.roll_num + 3) - ((st.roll_num-1) % 3);
	st.is_new_turn = true;
	updateStateId();

	return score;
}

// A wrapper to initialize dice config table
void Yahtzee::initializeTableDiceConfig() {
	std::string str = "DiceConfig";
    if(db.getRowCount(str) == 0) { // Table has not yet been created
		int curr_combo[10] = {0,0,0,0,0,0,0,0,0,0};
		int combo_count = 1;
		setDiceConfigTables(5, 1, 0, curr_combo, combo_count);
		diceConfig dc_data;
		dc_data.dice_key = 0;
		db.selectDiceConfig(&dc_data, true);
    }
    else
    	std::cout << "DiceConfig data already exists. Skipping dice config table initialization.\n";

}

// A wrapper to initialize dice probability table
void Yahtzee::initializeTableDiceProbability() {
	std::string str = "DiceProbability";
	int count = db.getRowCount(str);
	if(count == 0) { // Table has not yet been created
		int curr_combo[10] = {0,0,0,0,0,0,0,0,0,0};
		// Calls wrapper that specifies kept dice state before recursing through possible rolls
		setKeptDice(0, 0, 1, 0, curr_combo);
		diceProbability dc_prob;
		dc_prob.kept_dice = 0;
		dc_prob.next_dice = 1;
		db.selectDiceProbability(&dc_prob);
	}
	else
		std::cout << "DiceProbability data already exists. Skipping dice probability table initialization.\n";
}

// A wrapper to initialize dice probability table
void Yahtzee::initializeTableOutput() {
	std::string str = "Output";
	if(db.getRowCount(str) == 0) { // Table has not yet been created
		// TODO :)

	}
	else
		std::cout << "Output data already exists. Skipping output table initialization.\n";
}

/*
 * Purpose: Set the dice configuration table to allow for constant time lookups
 * Style: Recursive function that iterates through all possible
 *    dice combinations and maps them to the number of mapped combos
 *    to that point (i.e. A number between 1-252)
 */
void Yahtzee::setDiceConfigTables(int freq_left, int min, int curr_index, int (&curr_combo)[10], int &combo_count) {
	for(int i = min; i <= 6; i++) {
		curr_combo[curr_index] = i;
		for(int j = freq_left; j > 0; j--) {
			curr_combo[curr_index + 1] = j;
			if(j == freq_left) { // No more dice can be added to combo. Map it!
				diceConfig dc;
				int scoring_data = setScoringMapValue(curr_combo);
				dc.dice_key = getDiceKey(curr_combo);
				dc.dice_id = combo_count;
				dc.sum = scoring_data>>24;
				dc.is_yahtzee = ((scoring_data>>23) & 1);
				dc.is_large_straight = ((scoring_data>>22) & 1);
				dc.is_small_straight = ((scoring_data>>21) & 1);
				dc.is_full_house = ((scoring_data>>20) & 1);
				dc.is_4_of_a_kind = ((scoring_data>>19) & 1);
				dc.is_3_of_a_kind = ((scoring_data>>18) & 1);
				dc.num_1s = scoring_data & 0x7;
				dc.num_2s = ((scoring_data>>3) & 0x7);
				dc.num_3s = ((scoring_data>>6) & 0x7);
				dc.num_4s = ((scoring_data>>9) & 0x7);
				dc.num_5s = ((scoring_data>>12) & 0x7);
				dc.num_6s = ((scoring_data>>15) & 0x7);
				db.insertDiceConfig(&dc, 0);
				combo_count++; // Increment combo count
			}
			else if(min == 6 && j < freq_left) // No higher number than 6 is possible. Break!
				break;
			else // recurse to next highest number and place in combo array
				setDiceConfigTables(freq_left-j, i+1, curr_index+2, curr_combo, combo_count);
		}
	}
	// About to return to previous recursion. Reset array values to 0
	curr_combo[curr_index] = 0;
	curr_combo[curr_index + 1] = 0;
	return;
}

/*
 * Purpose: Sets the dice roll probability table and helps output table set values:
 * Kept State Id | Potential Dice State Id | Prob Numerator | Prob Denominator
 * Style: Recursively goes through all possible rolls given the kept dice state. This means
 * 		that impossible dice rolls will not be added to the table.
 */
void Yahtzee::setDiceProbTable(int num_of_dice, int freq_left, int min, int curr_index, int (&roll_curr_combo)[10],
		int (&kept_curr_combo)[10])
{
	for(int i = min; i <= 6; i++) {
		roll_curr_combo[curr_index] = i;
		for(int j = freq_left; j > 0; j--) {
			roll_curr_combo[curr_index + 1] = j;
			if(j == freq_left) { // No more dice can be added to combo. Map it!
				diceProbability dp;
				dp.kept_dice = getDiceKey(kept_curr_combo);
				dp.next_dice = combineAndGetDiceId(roll_curr_combo, kept_curr_combo);
				dp.prob_num = getNumerator(num_of_dice, roll_curr_combo);
				dp.prob_den = (int)std::pow(6, num_of_dice);
				db.insertDiceProbability(&dp, 0);
			}
			else if(min == 6 && j < freq_left) // No higher number than 6 is possible. Break!
				break;
			else // recurse to next highest number and place in combo array
				setDiceProbTable(num_of_dice, freq_left-j, i+1, curr_index+2, roll_curr_combo, kept_curr_combo);
		}
	}
	// About to return to previous recursion. Reset array values to 0
	roll_curr_combo[curr_index] = 0;
	roll_curr_combo[curr_index + 1] = 0;
	return;
}

/*
 * Purpose: Helper function for setDiceProbTable. Needed so we can recurse through possible kept dice states
 */
void Yahtzee::setKeptDice(int num_of_dice, int freq_left, int min, int curr_index, int (&curr_combo)[10]) {
	// If keeping no dice, special case: roll all dice!
	if(num_of_dice == 0) {
		int arr[10] = {0,0,0,0,0,0,0,0,0,0};
		setDiceProbTable(5, 5, 1, 0, arr, curr_combo);
	}
	// Otherwise, recurse through potential kept dice states starting with keeping 1 dice
	for(int i = min; i <= 6; i++) {
			curr_combo[curr_index] = i;
			for(int j = freq_left; j > 0; j--) {
				curr_combo[curr_index + 1] = j;
				if(j == freq_left) { // No more dice can be added to combo. Call setDiceProbTable!
					int arr[10] = {0,0,0,0,0,0,0,0,0,0};
					setDiceProbTable(5-num_of_dice, 5-num_of_dice, 1, 0, arr, curr_combo);
				}
				else if(min == 6 && j < freq_left) // No higher number than 6 is possible. Break!
					break;
				else // recurse to next highest number and place in combo array
					setKeptDice(num_of_dice, freq_left-j, i+1, curr_index+2, curr_combo);
			}
		}
		// About to return to previous recursion. Reset array values to 0
		curr_combo[curr_index] = 0;
		curr_combo[curr_index + 1] = 0;

		// Initial recursion finished --> increment # of dice if < 4
		// You cannot keep more than 4 dice, therefore stop after 4:
		if(freq_left == num_of_dice && num_of_dice < 4) {
			setKeptDice(num_of_dice+1, freq_left+1, 1, 0, curr_combo);
		}
		return;
}

/*
 * Purpose: Helper for setDiceProbTable to determine the numerator for the probability of dice roll
 * Style: loop through all the numbers of the rolled dice, multiplying
 * 		together Combination(remaining dice to consider, frequency of curr dice num)
 * 		This yields the number of ways to achieve given roll --> which is the
 * 		numerator of the probability calculations
 */
int Yahtzee::getNumerator(int num_of_dice, const int (&roll_curr_combo)[10]) {
	int numer = 1;
	int curr_freq = 0;
	for(int i = 1; i < 9 && roll_curr_combo[i] > 0; i+=2) {
		// multiply numerator by C(# dice remaining, freq of curr num)
		int comb_arg1 = num_of_dice - curr_freq;
		int comb_arg2 = roll_curr_combo[i];
		// C(n,k) = fact(n) / (fact(k) * (fact(n-k)))
		numer *= (factorial(comb_arg1) / (factorial(comb_arg2) * factorial(comb_arg1 - comb_arg2)));
		curr_freq += roll_curr_combo[i]; // Increment number of dice already considered
	}

	return numer;
}

// Compute the factorial of x
int Yahtzee::factorial(int x) {
	if(x < 0) {
		std::cout << "Error: Cannot take the factorial of a negative number!\n";
		return -1;
	}
	return (x == 1 || x == 0) ? 1 : x * factorial(x-1);
}

int Yahtzee::combineAndGetDiceId(const int (&roll_curr_combo)[10], const int (&kept_curr_combo)[10]) {
	// Combine kept and rolled arrays to make one roll
	int freqs[6] = {0,0,0,0,0,0};
	for(int i = 0; i <= 8; i += 2) {
		freqs[roll_curr_combo[i]-1] += roll_curr_combo[i+1];
		freqs[kept_curr_combo[i]-1] += kept_curr_combo[i+1];
	}
	// Constuct roll array
	int roll[10] = {0,0,0,0,0,0,0,0,0,0};
	int curr_index = 0;
	for(int i = 0; i < 6; i++) {
		if(freqs[i] > 0) { // If this number has been rolled
			roll[curr_index] = i+1;
			roll[curr_index+1] = freqs[i];
			curr_index += 2;
		}
	}

	// Finally, grab dice Id from dice id table and return:
	int dice_key = getDiceKey(roll);
	diceConfig dc_data;
	dc_data.dice_key = dice_key;
	db.selectDiceConfig(&dc_data, true);
	return dc_data.dice_id;
}

/*
 * Fills in kept_curr_combo for the calling function
 */
void Yahtzee::fillKeptDiceArray(int (&kept_dice_arr)[10], int dice_id, int kept_dice_key) {
	diceConfig dc; // get dice config from dice_id
	dc.dice_id = dice_id;
	dc = db.selectDiceConfig(&dc, false); // Get the dice configuration for given dice_id
	int dice_roll_freqs[6] = {dc.num_1s, dc.num_2s, dc.num_3s, dc.num_4s, dc.num_5s, dc.num_6s};
	int dice_roll[5] = {0,0,0,0,0}; // Using the dice freqs, create array of roll
	int curr = 0;
	for(int i = 0; i < 6; i++) {
		for(int j = 0; j < dice_roll_freqs[i]; j++) {
			dice_roll[curr] = i;
			curr++;
		}
	}
	// Figure out which dice are being kept, and if they are NOT, decrement
	// from the corresponding dice_roll_freqs value.
	// Then, dice_roll_freqs will represent the frequency array of the kept dice
	for(int i = 0; i < 5; i++) {
		if(!((kept_dice_key > i) & 1))
			dice_roll_freqs[dice_roll[i]]--;
	}

	// Now, finally we will up the kept dice array based upon the frequencies
	int count = 0;
	for(int i = 0; i < 6; i++) {
			if(dice_roll_freqs[i] != 0) {
			kept_dice_arr[count*2] = i;
			kept_dice_arr[count*2 + 1] = dice_roll_freqs[i];
			count++;
		}
	}
	return;
}

/*
 * Get the expected value from keeping specific dice
 * TEST ME!
 */
Fraction Yahtzee::getKeptStateExpValue(int num_of_dice, int freq_left, int min, int curr_index, int (&roll_curr_combo)[10],
		int (&kept_curr_combo)[10], int next_state) {
	Fraction sum(0,1);
	for(int i = min; i <= 6; i++) {
			roll_curr_combo[curr_index] = i;
			for(int j = freq_left; j > 0; j--) {
				roll_curr_combo[curr_index + 1] = j;
				if(j == freq_left) { // No more dice can be added to combo. Map it!
					diceProbability dp;
					dp.kept_dice = getDiceKey(kept_curr_combo);
					dp.next_dice = combineAndGetDiceId(roll_curr_combo, kept_curr_combo);
					db.selectDiceProbability(&dp);
					output o;
					o.state = (next_state & ~0xff) | dp.next_dice; // Clear last 8 bits (prev dice_id) and replace with new dice id
					db.selectOutput(&o);
					// Here is the punchline -- the expected value of this potential state is calculated
					// By multiplying the expected value of the next state times the probability of that state happening
					Fraction EV(dp.prob_num * o.ev_num, dp.prob_den * o.ev_den);
					sum += EV;
				}
				else if(min == 6 && j < freq_left) // No higher number than 6 is possible. Break!
					break;
				else // recurse to next highest number and place in combo array
					sum += getKeptStateExpValue(num_of_dice, freq_left-j, i+1, curr_index+2, roll_curr_combo, kept_curr_combo, next_state);
			}
		}
		// About to return to previous recursion. Reset array values to 0
		roll_curr_combo[curr_index] = 0;
		roll_curr_combo[curr_index + 1] = 0;
		return sum;
}




/*int main() {
	Yahtzee y;
}*/
