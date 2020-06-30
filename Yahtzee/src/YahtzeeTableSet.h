//================-============================================================
// Name        : YahtzeeTableSet.h
// Author      : Gabriel Simmons, Austin Sullivan
// Version     : 0.1
// Copyright   : Your copyright notice
// Description : Header file for YahtzeeTableSet.h
//============================================================================

#ifndef YAHTZEETABLESET_H_
#define YAHTZEETABLESET_H_

#include <string>
#include "Database.h"

class YahtzeeTableSet {
  public:
	YahtzeeTableSet(std::string filename); // Constructor

	static void initializeDiceConfigTable(); // Wrapper for setDiceTables
	static void initializeKeptDiceTables(bool initialize_prob_table); // Wrapper for setKeptDiceTable
  private:
	// Misc
	int getDiceKey(const int curr_combo[10]);

	// Table setters
	void setDiceConfigTables(int freq_left, int min, int curr_index, int (&curr_combo)[10], int &combo_count);
	void setKeptDiceTables(int num_of_dice, int freq_left, int min, int curr_index, int (&curr_combo)[10],
			int &combo_count, bool initialize_prob_table);
	void setDiceProbTable(int freq_left, int min, int curr_index, int (&roll_curr_combo)[10],
			const int kept_curr_combo[10]);

	Database db;
};


#endif /* YAHTZEETABLESET_H_ */
