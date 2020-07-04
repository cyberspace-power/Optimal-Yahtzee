// //================-============================================================
// // Name        : YahtzeeTableSet.cpp
// // Author      : Gabriel Simmons, Austin Sullivan
// // Version     : 0.1
// // Copyright   : Your copyright notice
// // Description : Functions to Fill in Tables in Database
// //============================================================================

// #include "YahtzeeTableSet.h"

// #include <iostream>
// #include <cmath>
// #include <string>
// #include <sqlite3.h> 

// YahtzeeTableSet::YahtzeeTableSet(sqlite3 *db) {
// 	this->db = db;
// }

// /*
//  * Purpose: Set the dice configuration table to allow for constant time lookups
//  * Style: Recursive function that iterates through all possible
//  *    dice combinations and maps them to the number of mapped combos
//  *    to that point (i.e. A number between 1-252)
//  */
// void YahtzeeTableSet::setDiceConfigTables(int freq_left, int min, int curr_index, int (&curr_combo)[10], int &combo_count) {
// 	for(int i = min; i <= 6; i++) {
// 		curr_combo[curr_index] = i;
// 		for(int j = freq_left; j > 0; j--) {
// 			curr_combo[curr_index + 1] = j;
// 			if(j == freq_left) { // No more dice can be added to combo. Map it!
// 				diceConfig dc;
// 				int scoring_data = Yahtzee::setScoringMapValue(curr_combo);
// 				dc.dice_key = Yahtzee::getDiceKey(curr_combo);
// 				dc.dice_id = combo_count;
// 				dc.sum = scoring_data>>24;
// 				dc.is_yahtzee = ((scoring_data>>23) & 1);
// 				dc.is_large_straight = ((scoring_data>>22) & 1);
// 				dc.is_small_straight = ((scoring_data>>21) & 1);
// 				dc.is_full_house = ((scoring_data>>20) & 1);
// 				dc.is_4_of_a_kind = ((scoring_data>>19) & 1);
// 				dc.is_3_of_a_kind = ((scoring_data>>18) & 1);
// 				dc.num_1s = scoring_data & 0x7;
// 				dc.num_2s = ((scoring_data>>3) & 0x7);
// 				dc.num_3s = ((scoring_data>>6) & 0x7);
// 				dc.num_4s = ((scoring_data>>9) & 0x7);
// 				dc.num_5s = ((scoring_data>>12) & 0x7);
// 				dc.num_6s = ((scoring_data>>15) & 0x7);
// 				db.insertDiceConfig(&dc, 0);

// 				combo_count++; // Increment combo count
// 			}
// 			else if(min == 6 && j < freq_left) // No higher number than 6 is possible. Break!
// 				break;
// 			else // recurse to next highest number and place in combo array
// 				setDiceConfigTables(freq_left-j, i+1, curr_index+2, curr_combo, combo_count);
// 		}
// 	}
// 	// About to return to previous recursion. Reset array values to 0
// 	curr_combo[curr_index] = 0;
// 	curr_combo[curr_index + 1] = 0;
// 	return;
// }

// /* Purpose: Sets the kept dice table by mapping kept dice states to ID numbers 0-209
//  * Does not map states in which 5 dice are kept (assumed impossible)
//  * Style: Same as other table setting functions for different dice states
//  */

// void YahtzeeTableSet::setKeptDiceTables(int num_of_dice, int freq_left, int min, int curr_index,
// 		int (&curr_combo)[10], int &combo_count, bool initialize_prob_table)
// {
// 	if(num_of_dice == 0) {
// 		// Add to table
// 		if(initialize_prob_table) {
// 			int partial_roll[10] = {0,0,0,0,0,0,0,0,0,0};
// 			setDiceProbTable(5, 5, 1, 0, combo_count, partial_roll, curr_combo);
// 		}
// 		combo_count++;
// 	}
// 	for(int i = min; i <= 6; i++) {
// 		curr_combo[curr_index] = i;
// 		for(int j = freq_left; j > 0; j--) {
// 			curr_combo[curr_index + 1] = j;
// 			if(j == freq_left) { // No more dice can be added to combo. Map it!
// 				// Add to kept dice table
// 				if(initialize_prob_table) { // Set dice probability table if this flag is true
// 					int partial_roll[10] = {0,0,0,0,0,0,0,0,0,0};
// 					setDiceProbTable(5 - num_of_dice, 5 - num_of_dice, 1, 0, combo_count, partial_roll, curr_combo);
// 				}
// 				combo_count++;
// 			}
// 			else if(min == 6 && j < freq_left) // No higher number than 6 is possible. Break!
// 				break;
// 			else // recurse to next highest number and place in combo array
// 				setKeptDiceTables(num_of_dice, freq_left-j, i+1, curr_index+2, curr_combo, combo_count, initialize_prob_table);
// 		}
// 	}
// 	// About to return to previous recursion. Reset array values to 0
// 	curr_combo[curr_index] = 0;
// 	curr_combo[curr_index + 1] = 0;

// 	// Initial recursion finished --> increment # of dice if < 4
// 	// You cannot keep more than 4 dice, therefore stop after 4:
// 	if(freq_left == num_of_dice && num_of_dice < 4) {
// 		setKeptDiceTables(num_of_dice+1, freq_left+1, 1, 0, curr_combo, combo_count, initialize_prob_table);
// 	}
// 	return;
// }

// /*
//  * Purpose: Sets the dice roll probability table:
//  * Kept State Id | Potential Dice State Id | Prob Numerator | Prob Denominator
//  * Style: Recursively goes through all possible rolls given the kept dice state. This means
//  * 		that impossible dice rolls will not be added to the table.
//  */
// void YahtzeeTableSet::setDiceProbTable(int num_of_dice, int freq_left, int min, int curr_index, int combo_count, int (&roll_curr_combo)[10],
// 		const int (&kept_curr_combo)[10])
// {
// 	for(int i = min; i <= 6; i++) {
// 		roll_curr_combo[curr_index] = i;
// 		for(int j = freq_left; j > 0; j--) {
// 			roll_curr_combo[curr_index + 1] = j;
// 			if(j == freq_left) { // No more dice can be added to combo. Map it!
// 				diceProbability dp;
// 				dp.kept_dice = combo_count;
// 				dp.next_dice = combineAndGetDiceId(roll_curr_combo, kept_curr_combo);
// 				dp.prob_num = getNumerator(num_of_dice, roll_curr_combo);
// 				dp.prob_den = (int)std::pow(6, num_of_dice);
// 				db.insertDiceProbability(&dp, 0);
// 			}
// 			else if(min == 6 && j < freq_left) // No higher number than 6 is possible. Break!
// 				break;
// 			else // recurse to next highest number and place in combo array
// 				setDiceProbTable(num_of_dice, freq_left-j, i+1, curr_index+2, combo_count, roll_curr_combo, kept_curr_combo);
// 		}
// 	}
// 	// About to return to previous recursion. Reset array values to 0
// 	roll_curr_combo[curr_index] = 0;
// 	roll_curr_combo[curr_index + 1] = 0;
// 	return;
// }

// /*
//  * Purpose: Helper for setDiceProbTable to determine the numerator for the probability of dice roll
//  * Style: loop through all the numbers of the rolled dice, multiplying
//  * 		together Combination(remaining dice to consider, frequency of curr dice num)
//  * 		This yields the number of ways to achieve given roll --> which is the
//  * 		numerator of the probability calculations
//  */
// int YahtzeeTableSet::getNumerator(int num_of_dice, const int (&roll_curr_combo)[10]) {
// 	int numer = 1;
// 	int curr_freq = 0;
// 	for(int i = 1; i < 9 && roll_curr_combo[i] > 0; i+=2) {
// 		// multiply numerator by C(# dice remaining, freq of curr num)
// 		int comb_arg1 = num_of_dice - curr_freq;
// 		int comb_arg2 = roll_curr_combo[i];
// 		// C(n,k) = fact(n) / (fact(k) * (fact(n-k)))
// 		numer *= (factorial(comb_arg1) / (factorial(comb_arg2) * factorial(comb_arg1 - comb_arg2)));
// 		curr_freq += roll_curr_combo[i]; // Increment number of dice already considered
// 	}

// 	return numer;
// }

// // Compute the factorial of x
// int YahtzeeTableSet::factorial(int x) {
// 	if(x < 0) {
// 		std::cout << "Error: Cannot take the factorial of a negative number!\n";
// 		return -1;
// 	}
// 	return (x == 1 || x == 0) ? 1 : x * factorial(x-1);
// }

// int combineAndGetDiceId(const int (&roll_curr_combo)[10], const int (&kept_curr_combo)[10]) {
// 	// Combine kept and rolled arrays to make one roll
// 	int freqs[6] = {0,0,0,0,0,0};
// 	for(int i = 0; i <= 8; i += 2) {
// 		freqs[roll_curr_combo[i]-1] += roll_curr_combo[i+1];
// 		freqs[kept_curr_combo[i]-1] += kept_curr_combo[i+1];
// 	}
// 	// Constuct roll array
// 	int roll[10] = {0,0,0,0,0,0,0,0,0,0};
// 	int curr_index = 0;
// 	for(int i = 0; i < 6; i++) {
// 		if(freqs[i] > 0) { // If this number has been rolled
// 			roll[curr_index] = i+1;
// 			roll[curr_index+1] = freqs[i];
// 			curr_index += 2;
// 		}
// 	}

// 	// Finally, grab dice Id from dice id table and return:
// 	int dice_key = Yahtzee::getDiceKey(roll);
// 	// dice_id = table[dice_key]; --> Basically, figure out how to grab this from table
// 	// return dice_id;
// 	return 0;
// }

// int main() {
// 	YahtzeeTableSet y("hello.db");
// 	int arr[10] = {1, 2, 2, 2, 3, 1, 0, 0, 0, 0};
// 	int dice = 5;
// 	std::cout << y.getNumerator(dice, arr) << std::endl;
// }
