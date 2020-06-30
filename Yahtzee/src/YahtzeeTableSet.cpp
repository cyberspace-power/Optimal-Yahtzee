//================-============================================================
// Name        : YahtzeeTableSet.cpp
// Author      : Gabriel Simmons, Austin Sullivan
// Version     : 0.1
// Copyright   : Your copyright notice
// Description : Functions to Fill in Tables in Database
//============================================================================

#include <iostream>
#include <string>
#include "Yahtzee.h"
#include "YahtzeeTableSet.h"
#include "Database.h"

YahtzeeTableSet::YahtzeeTableSet(std::string filename)
	: db(filename) {}

// A wrapper to initialize dice config table
void YahtzeeTableSet::initializeDiceConfigTable() {
	int curr_combo[10] = {0,0,0,0,0,0,0,0,0,0};
	int combo_count = 1;
	setDiceConfigTables(5, 1, 0, curr_combo, combo_count);
}

// A wrapper to intialize the kept dice table and dice probability table
// The dice probability table is only initialized if the bool value = true
void YahtzeeTableSet::initializeKeptDiceTables(bool initialize_prob_table) {
	int curr_combo[10] = {0,0,0,0,0,0,0,0,0,0};
	int combo_count = 0;
	if(initialize_prob_table)
		setKeptDiceTables(0, 0, 1, 0, curr_combo, combo_count, true);
	else
		setKeptDiceTables(0, 0, 1, 0, curr_combo, combo_count, false);
}

/*
 * Purpose: Set the dice configuration table to allow for constant time lookups
 * Style: Recursive function that iterates through all possible
 *    dice combinations and maps them to the number of mapped combos
 *    to that point (i.e. A number between 1-252)
 */
void YahtzeeTableSet::setDiceConfigTables(int freq_left, int min, int curr_index, int (&curr_combo)[10], int &combo_count) {
	for(int i = min; i <= 6; i++) {
		curr_combo[curr_index] = i;
		for(int j = freq_left; j > 0; j--) {
			curr_combo[curr_index + 1] = j;
			if(j == freq_left) { // No more dice can be added to combo. Map it!
				diceConfig dc;
				int scoring_data = Yahtzee::setScoringMapValue(curr_combo);
				dc.dice_key = Yahtzee::getDiceKey(curr_combo);
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

void YahtzeeTableSet::setKeptDiceTables(int num_of_dice, int freq_left, int min, int curr_index,
		int (&curr_combo)[10], int &combo_count, bool initialize_prob_table)
{
	if(num_of_dice == 0) {
		// Add to table
		combo_count++;
	}
	for(int i = min; i <= 6; i++) {
			curr_combo[curr_index] = i;
			for(int j = freq_left; j > 0; j--) {
				curr_combo[curr_index + 1] = j;
				if(j == freq_left) { // No more dice can be added to combo. Map it!
					// Add to kept dice table
					combo_count++;
				}
				else if(min == 6 && j < freq_left) // No higher number than 6 is possible. Break!
					break;
				else // recurse to next highest number and place in combo array
					setKeptDiceTables(num_of_dice, freq_left-j, i+1, curr_index+2, curr_combo, combo_count, initialize_prob_table);
			}
		}
		// About to return to previous recursion. Reset array values to 0
		curr_combo[curr_index] = 0;
		curr_combo[curr_index + 1] = 0;

		// Initial recursion finished --> increment # of dice if < 4
		// You cannot keep more than 4 dice, therefore stop after 4:
		if(freq_left == num_of_dice && num_of_dice < 4) {
			setKeptDiceTables(num_of_dice+1, freq_left+1, 1, 0, curr_combo, combo_count, initialize_prob_table);
		}
		return;
}

void setDiceProbTable(int freq_left, int min, int curr_index, int (&roll_curr_combo)[10],
			const int kept_curr_combo[10])
{
	return;
}
