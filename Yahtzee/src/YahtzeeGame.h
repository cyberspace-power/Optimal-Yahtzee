//================-============================================================
// Name        : YahtzeeGame.h
// Author      : Gabriel Simmons, Austin Sullivan
// Version     : 0.1
// Copyright   : Your copyright notice
// Description : Header file for YahtzeeGame.h
//============================================================================

#ifndef YAHTZEEGAME_H_
#define YAHTZEEGAME_H_

#include "Yahtzee.h"
#include <vector>

// scorecard struct really only meant for if you play a full game. Computer will have no need of it
typedef struct scorecard {
	// Sections
	int ones = -1;
	int twos = -1;
	int threes = -1;
	int fours = -1;
	int fives = -1;
	int sixes = -1;
	int three_of_kind = -1;
	int four_of_kind = -1;
	int full_house = -1;
	int small_straight = -1;
	int large_straight = -1;
	int yahtzee = -1;
	int chance = -1;
	// Total Scores
	int upper_score = 0;
	int lower_score = 0;
	int total_score = 0;
	// Bonuses
	int upper_bonus = 0;
	int yahtzee_bonus = 0;
} scorecard;

typedef struct player {
	scorecard sc;
	Yahtzee y;
	int turn;

	player() {
		y = Yahtzee();
		turn = 1;
	}
} player;

class YahtzeeGame {
  public:
	// Constructor/Destructor(s):
	YahtzeeGame(int players);  // For now, you can only start games from the beginning

	// Other functions
	void printScorecard(int player_num, bool is_taking_section);
	scorecard getScorecard(int player_num);
	player getPlayer(int player);
	void updateScorecard(int player_num, int section, int section_score);

	// Input functions

  private:
	std::vector<player> p;
};

#endif /* YAHTZEEGAME_H_ */
