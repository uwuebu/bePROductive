#include <sqlite3.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include "database.hpp"
#include <filesystem>

#define DATABASE_PATH "database/taskMdatabase.db"

void Database::initialize_database() {
    // Extract the directory path from DATABASE_PATH
    std::filesystem::path dbPath(DATABASE_PATH);
    std::filesystem::path dbDirectory = dbPath.parent_path();

    // Ensure the directory exists
    if (!std::filesystem::exists(dbDirectory)) {
        std::filesystem::create_directories(dbDirectory);
    }

    // Check if the database file exists
    std::ifstream dbFile(DATABASE_PATH);
    bool databaseExists = dbFile.good();
    dbFile.close();

    // Open (or create) the database
    sqlite3* db;
    if (sqlite3_open(DATABASE_PATH, &db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open or create the database");
    }

    // If the database doesn't exist, create tables
    if (!databaseExists) {
        const char* createTablesSQL = R"(
            CREATE TABLE "Tasks" (
                "taskID"	INTEGER NOT NULL UNIQUE,
                "parentID"	INTEGER,
                "Name"	TEXT NOT NULL,
                "Description"	TEXT,
                "Priority"	INTEGER,
                "Completion"	INTEGER NOT NULL DEFAULT 0,
                "Deadline"	TEXT,
                PRIMARY KEY("taskID" AUTOINCREMENT),
                FOREIGN KEY("parentID") REFERENCES "Tasks"("taskID") ON DELETE CASCADE,
                CHECK("Deadline" LIKE '____-__-__'),
                CHECK("priority" >= 1 AND "priority" <= 5),
                CHECK("completion" >= 0 AND "completion" <= 100)
            );

            CREATE TABLE "Sessions" (
                "sessionID"	INTEGER NOT NULL UNIQUE,
                "taskID"	INTEGER NOT NULL,
                "startTime"	INTEGER NOT NULL,
                "endTime"	INTEGER NOT NULL,
                "pausedTime"	INTEGER NOT NULL DEFAULT 0,
                "comment"	TEXT,
                PRIMARY KEY("sessionID" AUTOINCREMENT),
                FOREIGN KEY("taskID") REFERENCES "Tasks"("taskID") ON DELETE CASCADE
            );

            CREATE TRIGGER delete_update_parent_completion
                AFTER DELETE ON Tasks
                WHEN OLD.parentID IS NOT NULL
                BEGIN
                    UPDATE Tasks
                    SET Completion = (
                        SELECT IFNULL(AVG(Completion), 0)
                        FROM Tasks
                        WHERE parentID = OLD.parentID
                    )
                    WHERE taskID = OLD.parentID;
                END;

            CREATE TRIGGER insert_update_parent_completion
                AFTER INSERT ON Tasks
                WHEN NEW.parentID IS NOT NULL
                BEGIN
                    UPDATE Tasks
                    SET Completion = (
                        SELECT AVG(Completion)
                        FROM Tasks
                        WHERE parentID = NEW.parentID
                    )
                    WHERE taskID = NEW.parentID;
                END;

            CREATE TRIGGER update_parent_completion
                AFTER UPDATE OF Completion ON Tasks
                WHEN NEW.parentID IS NOT NULL
                BEGIN
                    UPDATE Tasks
                    SET Completion = (
                        SELECT AVG(Completion)
                        FROM Tasks
                        WHERE parentID = NEW.parentID
                    )
                    WHERE taskID = NEW.parentID;
                END;
        )";

        char* errMsg = nullptr;
        if (sqlite3_exec(db, createTablesSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
            std::string error = errMsg;
            sqlite3_free(errMsg);
            sqlite3_close(db);
            throw std::runtime_error("Failed to create tables: " + error);
        }

        //std::cout << "Database and tables created successfully.\n";
    } else {
        //std::cout << "Database already exists. Connection successful.\n";
    }

    sqlite3_close(db);
}

void Database::execute_query(const std::string& query) {
    sqlite3* db;
    
    // Open the SQLite database at the specified path
    if (sqlite3_open(DATABASE_PATH, &db)) {
        throw std::runtime_error("SQL error: failed to open database");
    }

    char* errMsg = nullptr;
    
    // Execute the SQL query using sqlite3_exec and check for errors
    if (sqlite3_exec(db, query.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::string error = errMsg;  // Capture the error message
        sqlite3_free(errMsg);         // Free the memory allocated for the error message
        throw std::runtime_error("SQL error: " + error); // Throw an exception with the error details
    }

    // Close the database connection
    sqlite3_close(db);
}

std::vector<std::vector<std::string>> Database::fetch_results(const std::string& query) {
    sqlite3* db;
    
    // Open the SQLite database at the specified path
    if (sqlite3_open(DATABASE_PATH, &db)) {
        throw std::runtime_error("SQL error: failed to open database");
    }

    sqlite3_stmt* stmt;
    
    // Prepare the SQL query for execution
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement: " + std::string(sqlite3_errmsg(db)));
    }

    std::vector<std::vector<std::string>> results;
    
    // Execute the query and fetch rows one by one
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int cols = sqlite3_column_count(stmt);  // Get the number of columns in the current row
        std::vector<std::string> row;
        
        // Loop through each column in the current row and store its value in the 'row' vector
        for (int i = 0; i < cols; ++i) {
            const unsigned char* text = sqlite3_column_text(stmt, i);
            row.push_back(text ? reinterpret_cast<const char*>(text) : ""); // Handle NULL values
        }
        
        // Add the row to the results vector
        results.push_back(row);
    }

    sqlite3_finalize(stmt);  // Finalize the prepared statement
    sqlite3_close(db);       // Close the database connection
    return results;          // Return the results as a 2D vector
}


int Database::insert_data(const std::string& query, const std::vector<std::variant<std::string, int, std::nullptr_t>>& data) {
    sqlite3* db;
    
    // Open the SQLite database at the specified path
    if (sqlite3_open(DATABASE_PATH, &db)) {
        throw std::runtime_error("SQL error: failed to open database");
    }

    sqlite3_stmt* stmt = nullptr;
    
    // Prepare the SQL query for execution
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    // Bind data to the prepared statement based on their types (string, int, or null)
    for (size_t i = 0; i < data.size(); ++i) {
        const auto& value = data[i];
        
        // If the value is a string, bind it as a text field
        if (std::holds_alternative<std::string>(value)) {
            sqlite3_bind_text(stmt, static_cast<int>(i + 1), std::get<std::string>(value).c_str(), -1, SQLITE_STATIC);
        } 
        // If the value is an integer, bind it as an integer field
        else if (std::holds_alternative<int>(value)) {
            sqlite3_bind_int(stmt, static_cast<int>(i + 1), std::get<int>(value));
        } 
        // If the value is null, bind it as a null field
        else if (std::holds_alternative<std::nullptr_t>(value)) {
            sqlite3_bind_null(stmt, static_cast<int>(i + 1));
        }
    }

    // Execute the query and check if it was successful
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Error inserting data: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt);  // Finalize the prepared statement
    return 0;                 // Return 0 on success
}


void Database::delete_entry(int ID, const std::string& table) {
    sqlite3* db;
    if (sqlite3_open(DATABASE_PATH, &db) != SQLITE_OK) {
        throw std::runtime_error("SQL error: failed to open database");
    }

    // Ensure foreign key constraints are enforced
    sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);

    // Start a transaction
    if (sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr) != SQLITE_OK) {
        sqlite3_close(db);
        throw std::runtime_error("Failed to start transaction");
    }

    try {
        // Create delete query dynamically
        std::string deleteSQL = "DELETE FROM " + table + " WHERE " + table.substr(0, table.length() - 1) + "ID = ?;";

        sqlite3_stmt* stmt = nullptr;
        if (sqlite3_prepare_v2(db, deleteSQL.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Failed to prepare delete statement for table '" + table + "': " + std::string(sqlite3_errmsg(db)));
        }

        if (sqlite3_bind_int(stmt, 1, ID) != SQLITE_OK) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to bind ID for deletion: " + std::string(sqlite3_errmsg(db)));
        }

        if (sqlite3_step(stmt) != SQLITE_DONE) {
            sqlite3_finalize(stmt);
            throw std::runtime_error("Failed to delete entry from '" + table + "': " + std::string(sqlite3_errmsg(db)));
        }

        sqlite3_finalize(stmt);

        // Commit transaction
        if (sqlite3_exec(db, "COMMIT;", nullptr, nullptr, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Failed to commit transaction");
        }

        sqlite3_close(db);
    } catch (const std::exception& e) {
        // Rollback in case of failure
        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);
        sqlite3_close(db);
        throw;
    }
}


int Database::execute_prepared_query(const std::string& query, const std::vector<std::variant<std::string, int>>& params) {
    sqlite3* db;
    
    // Open the SQLite database at the specified path
    if (sqlite3_open(DATABASE_PATH, &db) != SQLITE_OK) {
        throw std::runtime_error("SQL error: failed to open database");
    }

    // Enable recursive triggers for foreign key operations (if any)
    sqlite3_exec(db, "PRAGMA recursive_triggers = ON;", nullptr, nullptr, nullptr);
    
    sqlite3_stmt* stmt;
    
    // Prepare the SQL query for execution
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        return -1;
    }

    // Bind parameters to the prepared statement based on their types (string or int)
    for (size_t i = 0; i < params.size(); ++i) {
        if (std::holds_alternative<std::string>(params[i])) {
            sqlite3_bind_text(stmt, i + 1, std::get<std::string>(params[i]).c_str(), -1, SQLITE_STATIC);
        } else if (std::holds_alternative<int>(params[i])) {
            sqlite3_bind_int(stmt, i + 1, std::get<int>(params[i]));
        }
    }

    // Execute the statement and check if it was successful
    int result = sqlite3_step(stmt);
    if (result != SQLITE_DONE) {
        std::cerr << "SQL error: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        sqlite3_close(db);
        return -1;
    }

    sqlite3_finalize(stmt);  // Finalize the prepared statement
    sqlite3_close(db);       // Close the database connection
    return 0;                 // Return 0 on success
}

