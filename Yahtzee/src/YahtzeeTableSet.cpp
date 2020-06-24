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
	: d(filename) {}

// A wrapper to initialize the set
void YahtzeeTableSet::initializeDiceConfigTable() {
	int curr_combo[10] = {0,0,0,0,0,0,0,0,0,0};
	int combo_count = 0;
	setDiceTables(5, 1, 0, curr_combo, combo_count);
}

/*
 * Purpose: Set the dice configuration table to allow for constant time lookups
 * Style: Recursive function that iterates through all possible
 *    dice combinations and maps them to the number of mapped combos
 *    to that point (i.e. A number between 1-252)
 */
void YahtzeeTableSet::setDiceTables(int freq_left, int min, int curr_index, int (&curr_combo)[10], int &combo_count) {
	for(int i = min; i <= 6; i++) {
		curr_combo[curr_index] = i;
		for(int j = freq_left; j > 0; j--) {
			curr_combo[curr_index + 1] = j;
			if(j == freq_left) { // No more dice can be added to combo. Map it!
				// Todo: Construct dice config struct to add to database
				combo_count++;
			}
			else if(min == 6 && j < freq_left) // No higher number than 6 is possible. Break!
				break;
			else // recurse to next highest number and place in combo array
				setDiceTables(freq_left-j, i+1, curr_index+2, curr_combo, combo_count);
		}
	}
	// About to return to previous recursion. Reset array values to 0
	curr_combo[curr_index] = 0;
	curr_combo[curr_index + 1] = 0;
	return;
}
