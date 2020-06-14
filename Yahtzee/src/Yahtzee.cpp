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
	st = start_state;
	if(st.y_bonus_state && (((st.sc_status>>11) & 1) != 1)) {
		// Makes sure user didn't accidentally set y_bonus to available if yahtzee has not been taken
		st.y_bonus_state = false;
		std::cout << "Had to reset y_bonus to false\n";
	}
	// There will be 252 dice states (including null roll). Set map to
	// be ready for 252 entries. Reduces rehashing that can slow speed
	dice_state_map.reserve(252);
	dice_scoring_map.reserve(252);
	int curr_combo[10] = {0,0,0,0,0,0,0,0,0,0};
	int combo_count = 1;
	setDiceMaps(5, 1, 0, curr_combo, combo_count); // Sets the dice state unordered map
	updateStateId();
	// print state in bits to diagnose any errors
	std::bitset<64> b(curr_state_id);
	std::cout << "Current State ID = " << b << std::endl;
	srand(time(0)); // Set seed for RNG

	// Set up dice state mapping:
}

// Begins a game from the beginning. Do not jump to specific state.
Yahtzee::Yahtzee() {
	// There will be 252 dice states (including null roll). Set map to
	// be ready for 252 entries. Reduces rehashing that can slow speed
	dice_state_map.reserve(252);
	dice_scoring_map.reserve(252);
	int curr_combo[10] = {0,0,0,0,0,0,0,0,0,0}; // Bug fix: SET ARRAY!!!
	int combo_count = 0;
	setDiceMaps(5, 1, 0, curr_combo, combo_count); // Sets the dice state unordered map
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
 * Purpose: Set the dice state map and dice scoring map to allow for constant time lookups
 *    The unordered_map from STL is used to track this mapping
 * Style: Recursive function that iterates through all possible
 *    dice combinations and maps them to the number of mapped combos
 *    to that point (i.e. A number between 1-252)
 */
void Yahtzee::setDiceMaps(int freq_left, int min, int curr_index, int (&curr_combo)[10], int &combo_count) {
	for(int i = min; i <= 6; i++) {
		curr_combo[curr_index] = i;
		for(int j = freq_left; j > 0; j--) {
			curr_combo[curr_index + 1] = j;
			if(j == freq_left) { // No more dice can be added to combo. Map it!
				if(dice_state_map.find(getDiceKey(curr_combo)) != dice_state_map.end())
					std::cout << "Error -- Element already exists\n";
				dice_state_map[getDiceKey(curr_combo)] = combo_count;
				dice_scoring_map[combo_count] = setScoringMapValue(curr_combo);

				// Print for testing purposes
				/*std::cout << combo_count << ": " << std::endl;
				for(int i = 0; i <= curr_index; i += 2) {
					std::cout << curr_combo[i] << ": " << curr_combo[i+1] << ", ";
				}
				std::cout << std::endl;
				int temp = dice_scoring_map[combo_count];
				printf("Sum = %d\n3oK: %d; 4oK: %d; FH: %d; SS: %d; LS: %d; Yah: %d\n1s: %d; 2s: %d; 3s: %d; 4s: %d; 5s: %d; 6s: %d\n\n",
						temp>>24, (int)is3OfKind(curr_combo), (int)is4OfKind(curr_combo), (int)isFullHouse(curr_combo),
						(int)isSmallStraight(curr_combo), (int)isLargeStraight(curr_combo), (int)isYahtzee(curr_combo),
						temp & 0x7, (temp>>3) & 0x7, (temp>>6) & 0x7, (temp>>9) & 0x7, (temp>>12) & 0x7, (temp>>15) & 0x7);*/
				combo_count++;
			}
			else if(min == 6 && j < freq_left) // No higher number than 6 is possible. Break!
				break;
			else // recurse to next highest number and place in combo array
				setDiceMaps(freq_left-j, i+1, curr_index+2, curr_combo, combo_count);
		}
	}
	// About to return to previous recursion. Reset array values to 0
	curr_combo[curr_index] = 0;
	curr_combo[curr_index + 1] = 0;
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
	// For testing purposes
	/*for(int i = 0; i < 5; i++)
		std::cout << dice_multisets[i*2] << ": " << dice_multisets[i*2 + 1] << std::endl;

	std::cout << key << "\n";*/
	return dice_state_map.at(key);
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
	std::cout << "Roll: ";
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
bool Yahtzee::isJoker(int scoring_info, int section) {
	// Joker rule doesn't apply unless roll is yahtzee and yahtzee has been taken for 0 or 50 points
	if(!isSectionTaken(12) || (scoring_info>>23 & 1) != 1)
		return false;
	// Determine what number the yahtzee is in
	int yahtzee_dice_num = 0;
	for(int i = 0; i <= 6; i++) {
		if(scoring_info>>(3*i) & 0x7) {
			yahtzee_dice_num = i+1;
			break;
		}
	}

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
	int scoring_info = dice_scoring_map[curr_state_id & 0xff]; // First get scoring info mapped by dice state ID
	int score = 0;
	if(section <= 6) { // If upper section (Minimizes code in cases 1-6 of switch statement)
		score = section * ((scoring_info >> ((section-1) * 3)) & 0x7);
		sc.upper_score += score;
		st.up_total += score; // Increase state's upper total (may be different than scorecard's if game was started from middle state
		sc.total_score += score;
	}

	switch(section)
	{
	case 1: // Ones
		sc.ones = score;
		break;
	case 2: // Twos
		sc.twos = score;
		break;
	case 3: // Threes
		sc.threes = score;
		break;
	case 4: // Fours
		sc.fours = score;
		break;
	case 5: // Fives
		sc.fives = score;
		break;
	case 6: // Sixes
		sc.sixes = score;
		break;
	case 7: // Three of a Kind
		score = ((scoring_info>>18) & 1) ? (scoring_info>>24) : 0;
		sc.three_of_kind = score;
		sc.lower_score += score;
		sc.total_score += score;
		break;
	case 8: // Four of a Kind
		score = ((scoring_info>>19) & 1) ? (scoring_info>>24) : 0;
		sc.four_of_kind = score;
		sc.lower_score += score;
		sc.total_score += score;
		break;
	case 9: // Full House
		if(((scoring_info>>20) & 1) || isJoker(scoring_info, section)) {
			sc.full_house = 25;
			sc.lower_score += 25;
			sc.total_score += 25;
		}
		else
			sc.full_house = 0;
		break;
	case 10: // Small Straight
		if(((scoring_info>>21) & 1) || isJoker(scoring_info, section)) {
			sc.small_straight = 30;
			sc.lower_score += 30;
			sc.total_score += 30;
		}
		else
			sc.small_straight = 0;
		break;
	case 11: // Large Straight
		if(((scoring_info>>22) & 1) || isJoker(scoring_info, section)) {
			sc.large_straight = 40;
			sc.lower_score += 40;
			sc.total_score += 40;
		}
		else
			sc.large_straight = 0;
		break;
	case 12: // Yahtzee
		if((scoring_info>>23) & 1) {
			sc.yahtzee = 50;
			sc.lower_score += 50;
			sc.total_score += 50;
			st.y_bonus_state = true;
		} else
			sc.yahtzee = 0;

		break;
	case 13: // Chance
		score = scoring_info>>24; // Sum of dice
		sc.chance = score;
		sc.lower_score += score;
		sc.total_score += score;
		break;
	}
	// Add upper section bonus
	if(st.up_total >= 63 && sc.upper_bonus == 0) {
		sc.upper_bonus = 35;
		sc.total_score += 35;
	}
	// Add yahtzee joker bonuses
	if(st.y_bonus_state && isJoker(scoring_info, section)) {
		sc.yahtzee_bonus += 100;
		sc.total_score += 100;
	}

	// Handle current state stuff
	// Since we start a new turn, move the roll number to the beginning of
	// the next turn. The following equation ensures that will always be a
	// number where x % 3 = 1.
	st.roll_num = (st.roll_num + 3) - ((st.roll_num-1) % 3);
	st.is_new_turn = true;
	updateStateId();

	std::bitset<64> b(curr_state_id);
	std::cout << "Current State ID = " << b << std::endl;
	return 0;
}

scorecard Yahtzee::getScorecard() {
	return sc;
}


int main() {
	int dies[5] = {1,1,1,1,1};
	std::vector<int> d(dies, dies + sizeof(dies) / sizeof(int));
	state test;
	test.sc_status = 8190; // Only taken yahtzee
	test.roll_num = 39; // Entering first roll of second turn
	test.dice = d;
	test.up_total = 57;
	test.y_bonus_state = false; // Yahtzee bonus is available
	test.is_new_turn = false;

	Yahtzee y(test);
	y.takeSection(1);
	std::cout << y.getScorecard().total_score << std::endl;
	/*for(int i = 1; i <= 13; i++) {
		if(y.isSectionTaken(i))
			std::cout << "Section " << i << " is taken!\n";
		else
			std::cout << "Section " << i << " is not taken!\n";
	}*/
	/*std::string temp;
	int selected_dice = 0;
	for(int i = 0; i < 10; i++) {
		y.roll(selected_dice);
		do {
			std::cout << "\nSelect which dice to keep: ";
			std::getline(std::cin, temp);
			selected_dice = y.selectDice(temp);
		} while(selected_dice == -1);
		std::cout << "Kept Id #: " << selected_dice << std::endl;
	}
	int x[10] = {2, 1, 3, 1, 4, 1, 5, 1, 6, 1}; // one 1, one 5, three 6s
	std::cout << "Scoring Map Value = " << y.setScoringMapValue(x) << std::endl;*/

	return 0;
}
