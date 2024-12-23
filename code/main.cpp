#include <sqlite3.h>
#include <iostream>
#include <string>

using std::cout, std::cin, std::string;

// Callback function to handle query results *to be refined later!!!!
int callback(void* data, int argc, char** argv, char** colName) {
    cout << (const char*)data << "\n";
    for (int i = 0; i < argc; i++) {
        cout << colName[i] << " = " << (argv[i] ? argv[i] : "NULL") << "\n";
    }
    cout << "\n";
    return 0;
}

int main(){
    sqlite3* db;
    sqlite3_stmt* stmt;
    char* errorMessage = nullptr;
    int rc = sqlite3_open("database/taskMdatabase.db", &db);
    if (rc) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << '\n';
        return rc;
    } else {
        cout << "Opened database successfully!\n";
    }

   /*  string name, description, priority;
    cout << "Enter Task name:\t";
    cin >> name;

    cout << "Enter Task description:\t";
    cin >> description;

    cout << "Enter Task priority:\t";
    cin >> priority;

    //insert data into table:
    string insertSQL = R"(
        INSERT
    )";` */

    // Prepare an SQL INSERT statement
    std::string sql = "INSERT INTO tasks (name, description, priority) VALUES (?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return -1;
    }

    // Get user input
    std::string name, description, priority;
    std::cout << "Enter task name: ";
    std::getline(std::cin, name);
    std::cout << "Enter description: ";
    std::getline(std::cin, description);
    std::cout << "Enter priority: ";
    std::getline(std::cin, priority);

    // Bind user input to the prepared statement
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, priority.c_str(), -1, SQLITE_STATIC);

    // Execute the statement
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Error inserting data: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "Task inserted successfully!\n";
    }

     // Query the table and print results
    const char* data = "Query Results:";
    std::string selectSQL = "SELECT * FROM tasks;";

    rc = sqlite3_exec(db, selectSQL.c_str(), callback, (void*)data, &errorMessage);
    if (exit != SQLITE_OK) {
        std::cerr << "Error selecting data: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
    } else {
        std::cout << "Data selected successfully!\n";
    }

    sqlite3_close(db);
    return 0;
}