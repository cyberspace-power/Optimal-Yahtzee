//================-============================================================
// Name        : Database.h
// Author      : Gabriel Simmons, Austin Sullivan
// Version     : 0.1
// Copyright   : Your copyright notice
// Description : Header file for Database.h
//============================================================================

#ifndef DATABASE_H
#define DATABASE_H

#include <sstream>
#include <string>
#include <sqlite3.h> 

typedef struct diceConfig {
    unsigned int dice_key;
    unsigned short dice_id;  // this should be an unsigned short, but getDiceKey() currently returns an int
    unsigned short sum = 0;
    bool is_yahtzee = false;
    bool is_large_straight = false;
    bool is_small_straight = false;
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
        unsigned int prob_num = 0;  // probabilities stored as fractions to retain information
        unsigned int prob_den = 1;
} diceProbability;

typedef struct output {
        int state;
        int optimal_play;
        unsigned int prob_num = 0;  // probabilities stored as fractions to retain information
        unsigned int prob_den = 1;
} output;

class Database {
  public:
    Database();  // default constructor requires calling of setFilename()
    Database(std::string& filename);
    ~Database();
    Database(const Database&);

    void setFilename(std::string& filename);  // avoids the need for an operator=

    int exec(const std::string& str);

    // create the table
    void createTableDiceConfig();
    void createTableDiceProbability();
    void createTableOutput();

    // add a row to the table
    int insertDiceConfig(diceConfig* data, bool forceCommit);
    int insertDiceProbability(diceProbability* data, bool forceCommit);
    int insertOutput(output* data, bool forceCommit);

    // get a row from the table
    void selectDiceConfig(diceConfig* data);
    void selectDiceProbability(diceProbability* data);
    void selectOutput(output* data);

  private:
    bool opened = false;  // has the database connection been established?

     // manages bulk insert
    int commitDiceConfigInsert();
    int commitDiceProbabilityInsert();
    int commitOutputInsert();

    // helper required for select<Tablename>() methods
    static int selectDiceConfigCallback(void *void_data, int argc, char **argv, char **azColName);
    static int selectDiceProbabilityCallback(void *void_data, int argc, char **argv, char **azColName);
    static int selectOutputCallback(void *void_data, int argc, char **argv, char **azColName);

    sqlite3 *db;

    const unsigned int insertLimit = 10000;  // for INSERT optimization (if using sqlite version < 3.8.8, set to 500)

    // buffer used for bulk insert
    std::ostringstream insertDiceConfigBuffer;
    std::ostringstream insertDiceProbabilityBuffer;
    std::ostringstream insertOutputBuffer;

    // number of outstanding items to be inserted not yet committed to the database
    unsigned int insertDiceConfigBufferCount = 0;
    unsigned int insertDiceProbabilityBufferCount = 0;
    unsigned int insertOutputBufferCount = 0;
};

#endif /* DATABASE_H */
