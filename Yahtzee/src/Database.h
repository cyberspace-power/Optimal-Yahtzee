#ifndef DATABASE_H
#define DATABASE_H

#include <sstream>
#include <string>
#include <sqlite3.h> 

typedef struct diceConfig {
    int dice_id;  // this should be an unsigned short, but getDiceKey() currently returns an int
    unsigned short sum = 0;
    bool is_yahtzee = false;
    bool is_long_straight = false;
    bool is_short_straight = false;
    bool is_full_house = false;
    bool is_3_of_a_kind = false;
    bool is_4_of_a_kind = false;
    unsigned short num_1s = 0;  // ostream can't handle these as chars
    unsigned short num_2s = 0;
    unsigned short num_3s = 0;
    unsigned short num_4s = 0;
    unsigned short num_5s = 0;
    unsigned short num_6s = 0;
} diceConfig;

typedef struct diceProbability {
        int kept_dice;
        int next_dice;
        unsigned int prob_num;  // probabilities stored as fractions to retain information
        unsigned int prob_den;
} diceProbability;

class Database {
  public:
    Database(std::string& filename);
    ~Database();
    Database(const Database&);

    int exec(const std::string& str);

    int insertDiceConfig(diceConfig* data, bool forceCommit);
    int insertDiceProbability(diceProbability* data, bool forceCommit);

    void selectDiceConfig(diceConfig* data);
    void selectDiceProbability(diceProbability* data);

  private:
    int commitDiceConfigInsert();
    int commitDiceProbabilityInsert();

    static int selectDiceConfigCallback(void *void_data, int argc, char **argv, char **azColName);
    static int selectDiceProbabilityCallback(void *void_data, int argc, char **argv, char **azColName);

    sqlite3 *db;

    int insertLimit = 10000;  // for INSERT optimization (if using sqlite version < 3.8.8, set to 500)

    std::ostringstream insertDiceConfigBuffer;
    unsigned int insertDiceConfigBufferCount;
    
    std::ostringstream insertDiceProbabilityBuffer;
    unsigned int insertDiceProbabilityBufferCount;
};

#endif /* DATABASE_H */