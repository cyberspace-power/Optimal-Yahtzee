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


Database::Database(std::string& filename) {
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

    // Create necessary tables
    // TODO don't bother trying to create if they already exist
    sql = "CREATE TABLE DiceProbability ("  \
        "kept_dice INTEGER NOT NULL UNIQUE," \
        "next_dice INTEGER," \
        "prob_num INTEGER," \
        "prob_den INTEGER," \
        "PRIMARY KEY(kept_dice)" \
        ");";
    exec(sql);

    sql = "CREATE TABLE Output (" \
        "state INTEGER NOT NULL UNIQUE," \
        "optimal_play INTEGER," \
        "ev_num INTEGER," \
        "ev_den INTEGER," \
        "PRIMARY KEY(state)" \
        ");";
    exec(sql);

    sql = "CREATE TABLE DiceConfig (" \
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
        "PRIMARY KEY(dice_id)" \
        ");";
    exec(sql);
}

Database::~Database() {
    sqlite3_close(db);
}

Database::Database(const Database&) {
    // ostringstreams aren't movable so it deletes the copy constructor
    // but we don't need the copy constructor so I'm just leaving this blank lol
}

// used to execute a basic query where the only info we need is whether is succeeded
int Database::exec(const std::string& str) {
    char *zErrMsg = 0;
    int rc;

    std::cout << str << std::endl << std::endl;

    rc = sqlite3_exec(db, str.c_str(), NULL, 0, &zErrMsg);
    if( rc != SQLITE_OK ){
        std::cout << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
        return -1;
    } else {
        // std::cout << "Completed operation successfully" << std::endl;
        return 0;
    }
}

// lazy insert into the DiceConfig table
// forceCommit flag commits all outstanding inserts into this table
int Database::insertDiceConfig(diceConfig* data, bool forceCommit) {
    if (insertDiceConfigBuffer.str().length() == 0) {
        insertDiceConfigBuffer << "INSERT INTO DiceConfig\n";
        insertDiceConfigBuffer << "VALUES\n";
    }

    insertDiceConfigBuffer << "(";
    insertDiceConfigBuffer << data->dice_id << ", ";
    insertDiceConfigBuffer << data->sum << ", ";
    insertDiceConfigBuffer << data->is_yahtzee << ", ";
    insertDiceConfigBuffer << data->is_long_straight << ", ";
    insertDiceConfigBuffer << data->is_short_straight << ", ";
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
    } else {
        insertDiceConfigBuffer << "),\n";
        insertDiceConfigBufferCount++;
    }
    return 0;
}

// lazy insert into the DiceProbability table
// forceCommit flag commits all outstanding inserts into this table
int Database::insertDiceProbability(diceProbability* data, bool forceCommit) {
    if (insertDiceProbabilityBuffer.str().length() == 0) {
        insertDiceProbabilityBuffer << "INSERT INTO DiceProbability\n";
        insertDiceProbabilityBuffer << "VALUES\n";
    }

    insertDiceProbabilityBuffer << "(";
    insertDiceProbabilityBuffer << data->kept_dice << ", ";
    insertDiceProbabilityBuffer << data->next_dice << ", ";
    insertDiceProbabilityBuffer << data->prob_num << ", ";
    insertDiceProbabilityBuffer << data->prob_den;

    if (forceCommit || insertDiceProbabilityBufferCount >= insertLimit){
        return commitDiceProbabilityInsert();
    } else {
        insertDiceProbabilityBuffer << "),\n";
        insertDiceProbabilityBufferCount++;
    }
    return 0;
}

// fills in the data parameter with the contents of the corresponding row in the database
// data.dice_id needs to be set before this method is called
// sets the data.dice_id field to -1 on error
void Database::selectDiceConfig(diceConfig* data) {
    char *zErrMsg = 0;
    int rc;
    std::ostringstream sql;

    // commit any outstanding INSERTs before performing SELECT
    if (insertDiceConfigBufferCount > 0) {
        commitDiceConfigInsert();
    }

    sql << "SELECT * FROM DiceConfig\n";
    sql << "WHERE dice_id=" << data->dice_id << ";";

    rc = sqlite3_exec(db, sql.str().c_str(), selectDiceConfigCallback, (void*)data, &zErrMsg);
    if( rc != SQLITE_OK ){
        std::cout << "SQL error: " << zErrMsg << std::endl;
        sqlite3_free(zErrMsg);
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

int Database::commitDiceConfigInsert() {
    int ret;

    insertDiceConfigBuffer << ");";
    ret = exec(insertDiceConfigBuffer.str());

    if (ret != 0) {
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

int Database::selectDiceConfigCallback(void *void_data, int argc, char **argv, char **azColName){
    diceConfig* data = (diceConfig*)void_data;

    if (argc != 14) {
        data->dice_id = -1;  // to indicate error
        return -1;
    }

    data->dice_id = atoi(argv[0]);
    data->sum = atoi(argv[1]);
    data->is_yahtzee = atoi(argv[2]);
    data->is_long_straight = atoi(argv[3]);
    data->is_short_straight = atoi(argv[4]);
    data->is_full_house = atoi(argv[5]);
    data->is_3_of_a_kind = atoi(argv[6]);
    data->is_4_of_a_kind = atoi(argv[7]);
    data->num_1s = atoi(argv[8]);
    data->num_2s = atoi(argv[9]);
    data->num_3s = atoi(argv[10]);
    data->num_4s = atoi(argv[11]);
    data->num_5s = atoi(argv[12]);
    data->num_6s = atoi(argv[13]);

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
