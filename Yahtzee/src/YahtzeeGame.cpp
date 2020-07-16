//================-============================================================
// Name        : YahtzeeGame.h
// Author      : Gabriel Simmons, Austin Sullivan
// Version     : 0.1
// Copyright   : Your copyright notice
// Description : Allows user to play solo or human multiplayer yahtzee games
//============================================================================

#include "Yahtzee.h"
#include "YahtzeeGame.h"

#include <vector>
#include <iostream>
#include <sstream>
#include <cstdlib>

YahtzeeGame::YahtzeeGame(int players) {
	player temp;
	for(int i = 0; i < players; i++) {
		p.push_back(temp);
	}
}

void YahtzeeGame::printScorecard(int player_num, bool is_taking_section) {
	scorecard * sc = &(p.at(player_num).sc);
	std::string sections[13] = {"Ones:\t\t\t", "Twos:\t\t\t", "Threes:\t\t\t", "Fours:\t\t\t", "Fives:\t\t\t", "Sixes:\t\t\t",
			"Three of a Kind:\t", "Four of a Kind:\t\t", "Full House:\t\t", "Small Straight:\t\t", "Large Straight:\t\t",
			"Yahtzee:\t\t", "Chance:\t\t\t" };
	int scores[13] = {sc->ones, sc->twos, sc->threes, sc->fours, sc->fives, sc->sixes, sc->three_of_kind,
			sc->four_of_kind, sc->full_house, sc->small_straight, sc->large_straight, sc->yahtzee, sc->chance};

	if(is_taking_section) {
		for(int i = 0; i < 13; i++) {
			// Individual sections
			if(scores[i] == -1)
				std::cout << sections[i] << "ENTER '" << (i+1) << "' TO TAKE\n";
			else
				std::cout << sections[i] << scores[i] << "    (already taken)\n";
			// Total score sections
			if(i == 5) {
				std::cout << "  Upper Score:\t\t  " << sc->upper_score << std::endl;
				std::cout << "  Bonus:\t\t  " << sc->upper_bonus << std::endl;
				std::cout << "  Total Upper Score:\t  " << (sc->upper_score + sc->upper_bonus) << "\n\n";
			}
			else if(i == 12) {
				std::cout << "  Yahtzee Bonus:\t  " << sc->yahtzee_bonus << std::endl;
				std::cout << "  Lower Total:\t\t  " << sc->lower_score << std::endl;
				std::cout << "  Upper Total:\t\t  " << (sc->upper_score + sc->upper_bonus) << std::endl;
				std::cout << "  Grand Total:\t\t  " << (sc->yahtzee_bonus + sc->lower_score + sc->upper_bonus + sc->upper_score) << "\n\n";
			}
		}
	}
	else {
		for(int i = 0; i < 13; i++) {
			// Individual sections --> print blank score if score is -1 (not yet taken)
			if(scores[i] == -1)
				std::cout << sections[i] << "--" << std::endl;
			else
				std::cout << sections[i] << scores[i] << std::endl;
			// Total score sections
			if(i == 5) {
				std::cout << "  Upper Score:\t\t  " << sc->upper_score << std::endl;
				std::cout << "  Bonus:\t\t  " << sc->upper_bonus << std::endl;
				std::cout << "  Total Upper Score:\t  " << (sc->upper_score + sc->upper_bonus) << "\n\n";
			}
			else if(i == 12) {
				std::cout << "  Yahtzee Bonus:\t  " << sc->yahtzee_bonus << std::endl;
				std::cout << "  Lower Total:\t\t  " << sc->lower_score << std::endl;
				std::cout << "  Upper Total:\t\t  " << (sc->upper_score + sc->upper_bonus) << std::endl;
				std::cout << "  Grand Total:\t\t  " << (sc->yahtzee_bonus + sc->lower_score + sc->upper_bonus + sc->upper_score) << "\n\n";
			}
		}
	}
	return;
}

player YahtzeeGame::getPlayer(int player_num) {
	return p.at(player_num);
}

scorecard YahtzeeGame::getScorecard(int player_num) {
	return p.at(player_num).sc;
}

void YahtzeeGame::updateScorecard(int player_num, int section, int section_score) {
	scorecard * s = &p.at(player_num).sc;

	// First, handle total scores and bonuses:

	s->total_score += section_score; // Update total score

	// if the score taken was over 100, there must have been a yahtzee bonus
	if(section_score >= 100) {
		s->yahtzee_bonus += 100;
		section_score -= 100;
	}
	// if the section taken was between 1-6 and the score is >35 points, the upper bonus must have been earned
	if(section <= 6 && section_score >= 35) {
		s->upper_bonus = 35;
		section_score -= 35;
	}
	// Now update lower or upper section totals, depending which section was taken:
	if(section <= 6)
		s->upper_score += section_score;
	else
		s->lower_score += section_score;

	// Next, handle the individual sections:
	switch(section) {
	case 1:
		s->ones = section_score;
		break;
	case 2:
		s->twos = section_score;
		break;
	case 3:
		s->threes = section_score;
		break;
	case 4:
		s->fours = section_score;
		break;
	case 5:
		s->fives = section_score;
		break;
	case 6:
		s->sixes = section_score;
		break;
	case 7:
		s->three_of_kind = section_score;
		break;
	case 8:
		s->four_of_kind = section_score;
		break;
	case 9:
		s->full_house = section_score;
		break;
	case 10:
		s->small_straight = section_score;
		break;
	case 11:
		s->large_straight = section_score;
		break;
	case 12:
		s->yahtzee = section_score;
		break;
	case 13:
		s->chance = section_score;
		break;
	}

	return;
}


// Helper functions for main()

// Has user input how many players will be playing.
int getNumberOfPlayersInput() {
	std::string line;
	std::stringstream convert_to_int(line);
	int players = 0;
	std::cout << "Hello and welcome! How many players are playing?\n";
	while(1) {
		convert_to_int.clear();
		char errorCheck;
		std::getline(std::cin, line);
		convert_to_int.str(line);
		if(!(convert_to_int >> players)) {
			std::cout << "Not a number. Try again\n";
			continue;
		}
		else if(convert_to_int >> errorCheck) { // Checks if there are any extra characters in line
			std::cout << "Please enter numbers only. Try again\n";
			continue;
		}
		else if(players > 8 || players < 1) { // Only support 1-8 players
			std::cout << "Please select 1-8 players\n";
			continue;
		}
		else {
			convert_to_int.clear();
			break;
		}
	}
	return players;
}

// Has user input what section to take:
int getSectionToTakeInput(int player_num, YahtzeeGame & yg) {
	std::string line;
	std::stringstream convert_to_int(line);
	int section = 0;
	std::cout << "Please select a section to take from the scorecard:\n\n";
	yg.printScorecard(player_num, true);
	while(1) {
		convert_to_int.clear();
		char errorCheck;
		std::getline(std::cin, line);
		convert_to_int.str(line);
		if(!(convert_to_int >> section)) {
			std::cout << "Not a number. Try again\n";
			continue;
		}
		else if(convert_to_int >> errorCheck) { // Checks if there are any extra characters in line
			std::cout << "Please enter numbers only. Try again\n";
			continue;
		}
		else {
			convert_to_int.clear();
			break;
		}
	}
	return section;
}

int main() {
	int players = getNumberOfPlayersInput();
	YahtzeeGame game(players);
	Yahtzee curr_players[players];
	// Quick and dirty fix for yahtzee object reset bug
	/*for(int i = 0; i < players; i++)
		curr_players[i] = game.getPlayer(i).y();*/

	for(int turn = 1; turn <= 13; turn++) {
		for(int pl = 0; pl < players; pl++) {
			//player curr_player = game.getPlayer(pl);
			int kept_dice_state = 0; // Keep no dice

			for(int roll = 1; roll <= 3; roll++) {
				if(roll > 1) {
					// Ask user if they would like to roll again or take a section
					std::string roll_again;
					while(roll_again.compare("r") != 0 && roll_again.compare("s") != 0) {
						std::cout << "Would you like to stop here and take a section (type 's'), "
								"or would you like to select dice to keep and roll again (type 'r')\n";
						std::cin >> roll_again;
						if(roll_again.compare("r") != 0 && roll_again.compare("s") != 0)
							std::cout << "Please type either 'r' or 's'. Try again...\n";
					}
					std::cin.ignore (std::numeric_limits<std::streamsize>::max(), '\n');
					if(roll_again.compare("s") == 0)
						break;
					else {
						do {
							std::string dice;
							std::cout << "Select which dice to keep:\n";
							std::getline(std::cin, dice);
							kept_dice_state = curr_players[pl].selectDice(dice);
						} while(kept_dice_state == -1);
					}
				}
				curr_players[pl].roll(kept_dice_state);
			}
			int section = 0; int section_score = 0;
			do {
				section = getSectionToTakeInput(pl, game);
				section_score = curr_players[pl].takeSection(section);
			} while(section_score == -1);
			game.updateScorecard(pl, section, section_score);
			std::cout << "Player " << (pl+1) << "'s Scorecard:\n";
			game.printScorecard(pl, false);
		}
	}
	// Print final scores:
	for(int pl = 0; pl < players; pl++) {
		std::cout << "Player " << (pl+1) << ": " << game.getScorecard(pl).total_score << std::endl;
	}
	std::cin.clear();
}
