#include "Database.h"

#include <string>
#include <iostream>

// Example database usage
int main() {
    std::string filename = "test.db";
    Database db = Database(filename);

    diceProbability dp_data;
    dp_data.kept_dice = 1;
    dp_data.next_dice = 2;
    db.selectDiceProbability(&dp_data);

    std::cout << "PROBABILITY: " << dp_data.prob_num << "/" << dp_data.prob_den << std::endl;

    diceConfig dc_data;
    dc_data.dice_id = 123;
    db.selectDiceConfig(&dc_data);

    std::cout << "DICE CONFIG: " << dc_data.dice_id << " - num 1s: " << dc_data.num_1s << std::endl;

    return 0;
}