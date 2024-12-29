#include <sqlite3.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include "database.hpp"

#define DATABASE_PATH "database/taskMdatabase.db"

void Database::initialize_database() {
    // Check if the database file exists
    std::ifstream dbFile(DATABASE_PATH);
    bool databaseExists = dbFile.good();
    dbFile.close();

    sqlite3* db;
    if (sqlite3_open(DATABASE_PATH, &db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open or create the database");
    }

    // If the database doesn't exist, create tables
    if (!databaseExists) {
        const char* createTablesSQL = R"(
            CREATE TABLE "Tasks" (
                "taskID" INTEGER NOT NULL UNIQUE,
                "parentID" INTEGER,
                "Name" TEXT NOT NULL,
                "Description" TEXT,
                "Priority" INTEGER,
                "Completion" INTEGER NOT NULL DEFAULT 0,
                "Deadline" TEXT,
                PRIMARY KEY("taskID" AUTOINCREMENT),
                CHECK("Deadline" LIKE '____-__-__'),
                CHECK("Priority" >= 1 AND "Priority" <= 5)
            );

            CREATE TABLE "Sessions" (
                "sessionID" INTEGER NOT NULL UNIQUE,
                "taskID" INTEGER NOT NULL,
                "startTime" TEXT NOT NULL,
                "endTime" TEXT NOT NULL,
                "pausedTime" INTEGER NOT NULL DEFAULT 0,
                "comment" TEXT,
                PRIMARY KEY("sessionID" AUTOINCREMENT),
                FOREIGN KEY("taskID") REFERENCES "Tasks"("taskID"),
                CHECK("startTime" LIKE '____-__-__, __:__:__'),
                CHECK("endTime" LIKE '____-__-__, __:__:__')
            );
        )";

        char* errMsg = nullptr;
        if (sqlite3_exec(db, createTablesSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::string error = errMsg;
            sqlite3_free(errMsg);
            sqlite3_close(db);
            throw std::runtime_error("Failed to create tables: " + error);
        }

        std::cout << "Database and tables created successfully.\n";
    } else {
        std::cout << "Database already exists. Connection successful.\n";
    }

    sqlite3_close(db);
}

void Database::execute_query(const std::string& query){
    sqlite3* db;
    if(sqlite3_open(DATABASE_PATH, &db)){
        throw std::runtime_error("SQL error: failed to open database");
    }

    char* errMsg = nullptr;
    if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string error = errMsg;
        sqlite3_free(errMsg);
        throw std::runtime_error("SQL error: " + error);
    }

    sqlite3_close(db);
}

std::vector<std::vector<std::string>> Database::fetch_results(const std::string& query){
    sqlite3* db;
    if(sqlite3_open(DATABASE_PATH, &db)){
        throw std::runtime_error("SQL error: failed to open database");
    }

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    std::vector<std::vector<std::string>> results;
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int cols = sqlite3_column_count(stmt);
        std::vector<std::string> row;
        for (int i = 0; i < cols; ++i) {
            const unsigned char* text = sqlite3_column_text(stmt, i);
            row.push_back(text ? reinterpret_cast<const char*>(text) : "");
        }
        results.push_back(row);
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return results;
}

std::string& Database::fetch_single_result(const std::string& query){
    sqlite3* db;
    if(sqlite3_open(DATABASE_PATH, &db)){
        throw std::runtime_error("SQL error: failed to open database");
    }

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    std::string result;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const unsigned char* text = sqlite3_column_text(stmt, 0);
        result = text ? reinterpret_cast<const char*>(text) : "";
    } else {
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        throw std::runtime_error("No result found for the query: " + query);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return result;
}

int Database::insert_data(const std::string& query, const std::vector<std::variant<std::string, int, std::nullptr_t>>& data) {
    sqlite3* db;
    if(sqlite3_open(DATABASE_PATH, &db)){
        throw std::runtime_error("SQL error: failed to open database");
    }

    sqlite3_stmt* stmt = nullptr;

    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    // Bind each parameter from the data vector
    for (size_t i = 0; i < data.size(); ++i) {
        const auto& value = data[i];
        if (std::holds_alternative<std::string>(value)) {
            sqlite3_bind_text(stmt, static_cast<int>(i + 1), std::get<std::string>(value).c_str(), -1, SQLITE_STATIC);
        } else if (std::holds_alternative<int>(value)) {
            sqlite3_bind_int(stmt, static_cast<int>(i + 1), std::get<int>(value));
        } else if (std::holds_alternative<std::nullptr_t>(value)) {
            sqlite3_bind_null(stmt, static_cast<int>(i + 1));
        }
    }

    // Execute the query
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Error inserting data: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);
    return 0;
}