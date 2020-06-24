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

	void initializeDiceConfigTable(); // Wrapper for setDiceTables
  private:
	void setDiceTables(int freq_left, int min, int curr_index, int (&curr_combo)[10], int &combo_count);

	Database d;
};


#endif /* YAHTZEETABLESET_H_ */
