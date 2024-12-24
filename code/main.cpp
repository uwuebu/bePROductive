#include <sqlite3.h>
#include <iostream>
#include <iomanip>
#include <string>

using std::cout, std::cin, std::string;

// Callback function to handle query results
int callback(void*, int, char**, char**);
//Function to INSERT data into db
int create_task(sqlite3* db, sqlite3_stmt* stmt);
//Function to QUERY the db for all rows and print them
int print_table(sqlite3* db);

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
    
    if(print_table(db) == -1) return -1;
    
    char choice;
    do {
        cout << "\nMenu:\n1. Create Task | 2. Show Tasks | 3. Exit\nEnter your choice: ";
        cin >> choice;
        cin.ignore(); // Ignore leftover newline
        switch (choice) {
            case '1':
                if(create_task(db, stmt) == -1) return -1;
                break;
            case '2':
                if(print_table(db) == -1) return -1;;
                break;
            case '3':
                cout << "Exiting...";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != '3');

    sqlite3_close(db);
    return 0;
}

int create_task(sqlite3* db, sqlite3_stmt* stmt){
    // Prepare an SQL INSERT statement
    std::string sql = "INSERT INTO tasks (name, description, priority, deadline) VALUES (?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return -1;
    }

    // Get user input
    std::string name, description, priority, deadline;
    cout << "Enter task name:\t";
    std::getline(cin, name);
    cout << "Enter description:\t";
    getline(cin, description);
    cout << "Enter priority(1-5):\t";
    std::getline(cin, priority);
    cout << "Enter deadline(format: year-month-day):\t";
    std::getline(cin, deadline);

    // Bind user input to the prepared statement
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, description.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 3, priority.c_str(), -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 4, deadline.c_str(), -1, SQLITE_STATIC);

    // Execute the statement
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Error inserting data: " << sqlite3_errmsg(db) << std::endl;
    } else {
        std::cout << "Task inserted successfully!\n";
    }
    return 0;
}

int print_table(sqlite3* db){
    // Query the table and print results
    const char* data = "Query Results:";
    std::string selectSQL = "SELECT * FROM tasks;";
    char* errorMessage = nullptr;

    int rc = sqlite3_exec(db, selectSQL.c_str(), callback, (void*)data, &errorMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "Error selecting data: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
        return -1;
    } else {
        std::cout << "Data selected successfully!\n";
    }
    return 0;
}

int callback(void* data, int argc, char** argv, char** colName) {
    static bool headerPrinted = false;

    if (!headerPrinted) {
        cout << (const char*)data << "\n";
        // Print column names as table header
        for (int i = 0; i < argc; i++) {
            cout << std::setw(15) << std::left << colName[i];
        }
        cout << "\n";
        cout << std::string(15 * argc, '-') << "\n"; // Separator line
        headerPrinted = true;
    }

    // Print row data
    for (int i = 0; i < argc; i++) {
        cout << std::setw(15) << std::left << (argv[i] ? argv[i] : "NULL");
    }
    cout << "\n";

    return 0;
}