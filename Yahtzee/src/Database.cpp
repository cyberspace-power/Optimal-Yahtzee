//================-============================================================
// Name        : Database.cpp
// Author      : Gabriel Simmons, Austin Sullivan
// Version     : 0.1
// Copyright   : Your copyright notice
// Description : Handles SQLite database interactions
//============================================================================

#include "Database.h"

#include <iostream>
#include <sstream>

Database::Database() {}

Database::Database(std::string& filename) {
    setFilename(filename);
}

Database::~Database() {
    sqlite3_close(db);
}

Database::Database(const Database&) {
    // ostringstreams aren't movable so it deletes the copy constructor
    // but we don't need the copy constructor so I'm just leaving this blank lol
}

void Database::setFilename(std::string& filename) {
    char *zErrMsg = 0;
    int rc;
    std::string sql;

    // Establish connection with database
    rc = sqlite3_open(filename.c_str(), &db);
    if( rc ) {
        std::cout << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "Opened database successfully" << std::endl;
    }

    opened = true;

    // Create necessary tables
    createTableDiceConfig();
    createTableDiceProbability();
    createTableOutput();
}

// used to execute a basic query where the only info we need is whether is succeeded
int Database::exec(const std::string& str) {
    char *zErrMsg = 0;
    int rc;
    std::cout << "  SQL: " << str << std::endl;

    rc = sqlite3_exec(db, str.c_str(), NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        std::cout << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return -1;
    } else {
        return 0;
    }
}

void Database::createTableDiceConfig() {
    std::string sql;
    sql = "CREATE TABLE IF NOT EXISTS DiceConfig (" \
        "dice_key INTEGER NOT NULL UNIQUE," \
        "dice_id INTEGER NOT NULL UNIQUE," \
        "sum INTEGER," \
        "is_yahtzee INTEGER," \
        "is_long_straight INTEGER," \
        "is_short_straight INTEGER," \
        "is_full_house INTEGER," \
        "is_3_of_a_kind INTEGER," \
        "is_4_of_a_kind INTEGER," \
        "num_1s INTEGER," \
        "num_2s INTEGER," \
        "num_3s INTEGER," \
        "num_4s INTEGER," \
        "num_5s INTEGER," \
        "num_6s INTEGER," \
        "PRIMARY KEY(dice_key)" \
        ");";
    exec(sql);
}

void Database::createTableDiceProbability() {
    std::string sql;
    sql = "CREATE TABLE IF NOT EXISTS DiceProbability ("  \
        "kept_dice INTEGER NOT NULL," \
        "next_dice INTEGER," \
        "prob_num INTEGER," \
        "prob_den INTEGER" \
        ");";
    exec(sql);

    std::string str = "DiceProbability";
    if(getRowCount(str) == 0) { // Check if table has already existed. If yes, don't recreate the index
		sql = "CREATE INDEX dice_probability_index" \
		" ON DiceProbability (kept_dice, next_dice);";
		exec(sql);
	}
}

void Database::createTableOutput() {
    std::string sql;
    sql = "CREATE TABLE IF NOT EXISTS Output (" \
        "state INTEGER NOT NULL UNIQUE," \
        "optimal_play INTEGER," \
        "ev_num INTEGER," \
        "ev_den INTEGER," \
        "PRIMARY KEY(state)" \
        ");";
    exec(sql);
}

// lazy insert into the DiceConfig table
// forceCommit flag commits all outstanding inserts into this table
int Database::insertDiceConfig(diceConfig* data, bool forceCommit) {
    if (insertDiceConfigBuffer.str().length() == 0) {
        insertDiceConfigBuffer << "INSERT INTO DiceConfig\n";
        insertDiceConfigBuffer << "VALUES\n";
    }
    else // Must be done when you are sure another will be added. Commit will close final insertion
    	insertDiceConfigBuffer << "),\n"; // Close previous insertion before adding new one

    insertDiceConfigBuffer << "(";
    insertDiceConfigBuffer << data->dice_key << ", ";
    insertDiceConfigBuffer << data->dice_id << ", ";
    insertDiceConfigBuffer << data->sum << ", ";
    insertDiceConfigBuffer << data->is_yahtzee << ", ";
    insertDiceConfigBuffer << data->is_large_straight << ", ";
    insertDiceConfigBuffer << data->is_small_straight << ", ";
    insertDiceConfigBuffer << data->is_full_house << ", ";
    insertDiceConfigBuffer << data->is_3_of_a_kind << ", ";
    insertDiceConfigBuffer << data->is_4_of_a_kind << ", ";
    insertDiceConfigBuffer << data->num_1s << ", ";
    insertDiceConfigBuffer << data->num_2s << ", ";
    insertDiceConfigBuffer << data->num_3s << ", ";
    insertDiceConfigBuffer << data->num_4s << ", ";
    insertDiceConfigBuffer << data->num_5s << ", ";
    insertDiceConfigBuffer << data->num_6s;

    if (forceCommit || insertDiceConfigBufferCount >= insertLimit){
        return commitDiceConfigInsert();
    } else insertDiceConfigBufferCount++;
    return 0;
}

// lazy insert into the DiceProbability table
// forceCommit flag commits all outstanding inserts into this table
int Database::insertDiceProbability(diceProbability* data, bool forceCommit) {
    if (insertDiceProbabilityBuffer.str().length() == 0) {
        insertDiceProbabilityBuffer << "INSERT INTO DiceProbability\n";
        insertDiceProbabilityBuffer << "VALUES\n";
    }
    else // Must be done when you are sure another will be added. Commit will close final insertion
        insertDiceProbabilityBuffer << "),\n"; // Close previous insertion before adding new one

    insertDiceProbabilityBuffer << "(";
    insertDiceProbabilityBuffer << data->kept_dice << ", ";
    insertDiceProbabilityBuffer << data->next_dice << ", ";
    insertDiceProbabilityBuffer << data->prob_num << ", ";
    insertDiceProbabilityBuffer << data->prob_den;

    if (forceCommit || insertDiceProbabilityBufferCount >= insertLimit){
        return commitDiceProbabilityInsert();
    } else insertDiceProbabilityBufferCount++;
    return 0;
}

// lazy insert into the Output table
// forceCommit flag commits all outstanding inserts into this table
int Database::insertOutput(output* data, bool forceCommit) {
    if (insertOutputBuffer.str().length() == 0) {
        insertOutputBuffer << "INSERT INTO Output\n";
        insertOutputBuffer << "VALUES\n";
    }
    else // Must be done when you are sure another will be added. Commit will close final insertion
        insertOutputBuffer << "),\n"; // Close previous insertion before adding new one

    insertOutputBuffer << "(";
    insertOutputBuffer << data->state << ", ";
    insertOutputBuffer << data->optimal_play << ", ";
    insertOutputBuffer << data->prob_num << ", ";
    insertOutputBuffer << data->prob_den;

    if (forceCommit || insertOutputBufferCount >= insertLimit){
        return commitOutputInsert();
    } else insertOutputBufferCount++;
    return 0;
}

// fills in the data parameter with the contents of the corresponding row in the database
// data.dice_id or data.dice_key needs to be set before this method is called
// sets the data.dice_id field to -1 on error
// bool dice_key: true = dice key lookup; false = dice_id lookup
void Database::selectDiceConfig(diceConfig* data, bool dice_key) {
    char *zErrMsg = 0;
    int rc;
    std::ostringstream sql;

    // commit any outstanding INSERTs before performing SELECT
    if (insertDiceConfigBufferCount > 0) {
        commitDiceConfigInsert();
    }

    if(dice_key) { // If dice_key is used for the lookup:
		sql << "SELECT * FROM DiceConfig\n";
		sql << "WHERE dice_key=" << data->dice_key << ";";

		rc = sqlite3_exec(db, sql.str().c_str(), selectDiceConfigCallback, (void*)data, &zErrMsg);
		if( rc != SQLITE_OK ){
			std::cout << "SQL error: " << zErrMsg << "; " << sql.str() << std::endl;
			sqlite3_free(zErrMsg);
		}
    }
    else { // Else if dice_id is used for lookup
    	sql << "SELECT * FROM DiceConfig\n";
		sql << "WHERE dice_id=" << data->dice_id << ";";

		rc = sqlite3_exec(db, sql.str().c_str(), selectDiceConfigCallback, (void*)data, &zErrMsg);
		if( rc != SQLITE_OK ){
			std::cout << "SQL error: " << zErrMsg << "; " << sql.str() << std::endl;
			sqlite3_free(zErrMsg);
		}
    }
}

// fills in the data parameter with the contents of the corresponding row in the database
// data.kept_dice and data.next_dice need to be set before this method is called
// sets the data.kept_dice field to -1 on error
void Database::selectDiceProbability(diceProbability* data) {
    char *zErrMsg = 0;
    int rc;
    std::ostringstream sql;

    // commit any outstanding INSERTs before performing SELECT
    if (insertDiceProbabilityBufferCount > 0) {
        commitDiceProbabilityInsert();
    }

    sql << "SELECT * FROM DiceProbability\n";
    sql << "WHERE kept_dice=" << data->kept_dice << "\n"; 
    sql << "AND next_dice=" << data->next_dice << ";";

    rc = sqlite3_exec(db, sql.str().c_str(), selectDiceProbabilityCallback, (void*)data, &zErrMsg);
    if( rc != SQLITE_OK ){
        std::cout << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }
}

// fills in the data parameter with the contents of the corresponding row in the database
// data.state needs to be set before this method is called
// sets the data.kept_dice field to -1 on error
void Database::selectOutput(output* data) {
    char *zErrMsg = 0;
    int rc;
    std::ostringstream sql;

    // commit any outstanding INSERTs before performing SELECT
    if (insertOutputBufferCount > 0) {
        commitOutputInsert();
    }

    sql << "SELECT * FROM Output\n";
    sql << "WHERE state=" << data->state << ";"; 

    rc = sqlite3_exec(db, sql.str().c_str(), selectOutputCallback, (void*)data, &zErrMsg);
    if( rc != SQLITE_OK ){
        std::cout << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }
}

// fills in the data parameter with the contents of the corresponding row in the database
// data.kept_dice and data.next_dice need to be set before this method is called
// sets the data.kept_dice field to -1 on error
int Database::getRowCount(std::string& table) {
    char *zErrMsg = 0;
    int count = 0;
    int rc;
    std::ostringstream sql;

    // commit any outstanding INSERTs before performing SELECT
    if (insertDiceProbabilityBufferCount > 0) {
		commitDiceProbabilityInsert();
	}
    if (insertDiceConfigBufferCount > 0) {
    	commitDiceConfigInsert();
    }
    if (insertOutputBufferCount > 0) {
    	commitOutputInsert();
    }

    if(table.compare("DiceConfig") == 0)
    	sql << "SELECT Count(*) FROM DiceConfig;\n";
    else if(table.compare("DiceProbability") == 0)
    	sql << "SELECT Count(*) FROM DiceProbability;\n";
    else
    	sql << "SELECT Count(*) FROM Output;\n";

    rc = sqlite3_exec(db, sql.str().c_str(), countCallback, &count, &zErrMsg);
    if( rc != SQLITE_OK ){
        std::cout << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
    }
    return count;
}

int Database::commitDiceConfigInsert() {
    int ret;

    insertDiceConfigBuffer << ");";
    ret = exec(insertDiceConfigBuffer.str());

    if (ret != 0) {
    	std::cout << "commit error\n";
        return ret;
    }

    // reset buffer
    insertDiceConfigBuffer.str("");
    insertDiceConfigBuffer.clear();
    insertDiceConfigBufferCount = 0;

    return ret;
}

int Database::commitDiceProbabilityInsert() {
    int ret;
    std::cout << "hello\n";

    insertDiceProbabilityBuffer << ");";
    ret = exec(insertDiceProbabilityBuffer.str());

    if (ret != 0) {
        return ret;
    }

    // reset buffer
    insertDiceProbabilityBuffer.str("");
    insertDiceProbabilityBuffer.clear();
    insertDiceProbabilityBufferCount = 0;

    return ret;
}

int Database::commitOutputInsert() {
    int ret;

    insertOutputBuffer << ");";
    ret = exec(insertOutputBuffer.str());

    if (ret != 0) {
        return ret;
    }

    // reset buffer
    insertOutputBuffer.str("");
    insertOutputBuffer.clear();
    insertOutputBufferCount = 0;

    return ret;
}

int Database::selectDiceConfigCallback(void *void_data, int argc, char **argv, char **azColName){
    diceConfig* data = (diceConfig*)void_data;

    if (argc != 15) {
        data->dice_id = -1;  // to indicate error
        return -1;
    }

    data->dice_key = atoi(argv[0]);
    data->dice_id = atoi(argv[1]);
    data->sum = atoi(argv[2]);
    data->is_yahtzee = atoi(argv[3]);
    data->is_large_straight = atoi(argv[4]);
    data->is_small_straight = atoi(argv[5]);
    data->is_full_house = atoi(argv[6]);
    data->is_3_of_a_kind = atoi(argv[7]);
    data->is_4_of_a_kind = atoi(argv[8]);
    data->num_1s = atoi(argv[9]);
    data->num_2s = atoi(argv[10]);
    data->num_3s = atoi(argv[11]);
    data->num_4s = atoi(argv[12]);
    data->num_5s = atoi(argv[13]);
    data->num_6s = atoi(argv[14]);

    return 0;
}

int Database::selectDiceProbabilityCallback(void *void_data, int argc, char **argv, char **azColName){
    diceProbability* data = (diceProbability*)void_data;

    if (argc != 4) {
        data->kept_dice = -1;  // to indicate error
        return -1;
    }

    data->kept_dice = atoi(argv[0]);
    data->next_dice = atoi(argv[1]);
    data->prob_num  = atoi(argv[2]);
    data->prob_den  = atoi(argv[3]);

    return 0;
}

int Database::selectOutputCallback(void *void_data, int argc, char **argv, char **azColName){
    output* data = (output*)void_data;

    if (argc != 4) {
        data->state = -1;  // to indicate error
        return -1;
    }

    data->state = atoi(argv[0]);
    data->optimal_play = atoi(argv[1]);
    data->prob_num  = atoi(argv[2]);
    data->prob_den  = atoi(argv[3]);

    return 0;
}

int Database::countCallback(void *count, int argc, char **argv, char **azColName) {
	int *c = (int*) count;
	*c = atoi(argv[0]);
	return 0;
}
