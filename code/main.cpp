#include <sqlite3.h>
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <regex>

#define VERSION "v. 0.0.1/test"

using std::cout, std::cin, std::string;

int lines_printed = 0;

void print_intro();
// Callback function to handle query results
int callback(void*, int, char**, char**);
//Function to INSERT data into db
int create_goal(sqlite3* db, sqlite3_stmt* stmt);
//Function to QUERY the db for all rows and print them
int print_table_raw(sqlite3* db);
//goal menu
void goal_menu(sqlite3* db, sqlite3_stmt* stmt);

int main(){
    sqlite3* db;
    sqlite3_stmt* stmt;
    char* errorMessage = nullptr;
    int rc = sqlite3_open("database/taskMdatabase.db", &db);
    if (rc) {
        std::cerr << "Error opening database: " << sqlite3_errmsg(db) << '\n';
        return rc;
    } else {
        //cout << "Opened database successfully!\n";
    }
    
    print_intro();
    
    char choice;
    do {
        cout << " \033[2mGoals\033[m\033[36m[G]\033[m | \033[2mSchedule\033[m\033[36m[S]\033[m | \033[2mExit\033[m\033[36m[X]\033[m\n > ";
        cin >> choice;
        cin.ignore(); // Ignore leftover newline
        switch (choice) {
            case 'G':
            case 'g':
                goal_menu(db, stmt);
                break;
            case 'S':
            case 's':
                //CalendarMenuChoice
                break;
            case 'X':
            case 'x':
                cout << "Exiting...";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 'X' && choice != 'x');

    sqlite3_close(db);
    return 0;
}

void print_intro(){
    //be PRODUCTIVE ver. ...
    cout << "\n\033[1m" <<
        "\033[53m"<< string(58, ' ') << "\033[m\n\033[1m" <<
        " #            \033[33m####  ####   ###\033[m\033[1m    #             #        \n" <<
        " #           \033[33m#   # #   # #   #\033[m\033[1m    #                      \n" <<
        " ##  ###    \033[33m####  ####  #   #\033[m\033[1m    ## # # ### ### # # # ###\n" <<
        " # # ###   \033[33m#     #  #  #   #\033[m\033[1m    # # # # #    #  # # # ###\n" <<
        " ##  ###  \033[33m#     #   #  ###\033[m\033[1m       ## ### ###  #  #  #  ###\n" << 
        "\033[4m"<< string(58, ' ') << "\033[m\n" <<
        " \033[4m\033[94mhttps://github.com/uwuebu/task_tracker\033[m\t" << VERSION << "\n\n\033[s";
}

// Helper function to calculate displayed width
size_t get_displayed_width(const string& text) {
    // Remove ANSI escape sequences using regex
    std::regex escapeSeq("\033\\[[0-9;]*m");
    string cleanedText = std::regex_replace(text, escapeSeq, "");
    return cleanedText.length();
}

void print_goal(sqlite3* db, int maxWidth, int& selectedIndex) {
    sqlite3_stmt* stmt;
    string query = "SELECT taskID, name FROM tasks WHERE parentID IS NULL;";
    int rc = sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << '\n';
        return;
    }

    struct Goal {
        int id;
        string name;
        Goal(int p_id, const string& p_name)
            : id(p_id), name(p_name) {}
    };
    std::vector<Goal> goals;

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int goalID = sqlite3_column_int(stmt, 0);
        const unsigned char* goalName = sqlite3_column_text(stmt, 1);

        if (goalName) {
            goals.emplace_back(goalID, reinterpret_cast<const char*>(goalName));
        }
    }

    goals.emplace_back(0, "\033[36m[G]\033[m\033[2mAdd new goal\033[m");

    sqlite3_finalize(stmt);

    // Handle edge cases for selectedIndex
    if (selectedIndex >= static_cast<int>(goals.size() - 1)) {
        selectedIndex = goals.size() - 2;
    } else if (selectedIndex < 0) {
        selectedIndex = 0;
    }

    // Format output within the maxWidth constraint
    string currentLine;
    for (size_t i = 0; i < goals.size(); ++i) {
        string formattedGoal;
        if (static_cast<int>(i) == selectedIndex) {
            formattedGoal = "\033[7m" + goals[i].name + "\033[m"; // Highlight selected goal
        } else {
            formattedGoal = goals[i].name;
        }

        size_t formattedWidth = get_displayed_width(currentLine) + get_displayed_width(formattedGoal) + 3;

        if (currentLine.empty()) {
            currentLine = formattedGoal;
        } else if (formattedWidth <= static_cast<size_t>(maxWidth)) {
            currentLine += " | " + formattedGoal;
        } else {
            cout << currentLine << '\n';
            currentLine = formattedGoal;
        }
    }

    // Print the last line if it exists
    if (!currentLine.empty()) {
        cout << currentLine << '\n';
    }

    cout << " \033[2m<<Previous\033[m\033[36m[P]\033[m "
        << string(58 - (std::string(" <<Previous[P] Next[N]>> ").length() + 1), ' ')
        << " \033[2mNext\033[m\033[36m[N]\033[m\033[2m>>\033[m\n"
        << "\033[53m"<< string(58, ' ') << "\033[m\n";
}

void goal_menu(sqlite3* db, sqlite3_stmt* stmt) {
    cout << "\033[u\033[0J";
    char input;
    int selectedIndex = 0;

    do {
        print_goal(db, 58, selectedIndex);
        cout <<"\033[4m"<< string(58, ' ') << "\033[m\n"
         << "\033[2m Back\033[m\033[36m[X]\033[m "
        << string(58 - (std::string(" Back[X] Output raw table data[R] ").length() + 1), ' ')
        << " \033[2mOutput raw table data\033[m\033[36m[R]\033[m\n"
        << "Enter what's inside square brackets for navigation:\n > ";
        cin >> input;

        switch (input) {
            case 'R':
            case 'r':
                cout << "\033[u\033[0J";
                print_table_raw(db);
                cin.get();
                cin.ignore();
                cout << "\033[u\033[0J";
                break;

            case 'G':
            case 'g':
                cout << "\033[u\033[0J";
                create_goal(db, stmt);
                cout << "\033[u\033[0J";
                break;

            case 'P':
            case 'p':
                selectedIndex--; // Move to previous goal
                break;

            case 'N':
            case 'n':
                selectedIndex++; // Move to next goal
                break;

            case 'X':
            case 'x':
                break;

            default:
                cout << "\033[31mInvalid input! Try again.\033[m\n";
                break;
        }

        cout << "\033[u\033[0J"; // Clear previous output
    } while (input != 'X' && input != 'x');
    cout << "\033[u\033[0J";
}


int create_goal(sqlite3* db, sqlite3_stmt* stmt) {
    // Prepare an SQL INSERT statement
    string sql = "INSERT INTO tasks (name, description, priority, deadline) VALUES (?, ?, ?, ?);";

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Failed to prepare statement: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_close(db);
        return -1;
    }

    cin.ignore();
    // Get user input
    string name, description, priority, deadline;

    // Goal name is mandatory
    cout << "Enter goal name:\n > ";
    std::getline(cin, name);
    if (name.empty()) {
        cout << "\033[1A\033[0J";
        std::cerr << "Goal name cannot be empty.\n";
        sqlite3_finalize(stmt);
        return -1;
    }

    // Description (optional)
    cout << "Enter description:\n > ";
    std::getline(cin, description);

    // Priority (optional)
    cout << "Enter priority (1-5):\n > ";
    std::getline(cin, priority);

    // Deadline (optional)
    cout << "Enter deadline (format: YYYY-MM-DD):\n > ";
    std::getline(cin, deadline);

    // Bind user input to the prepared statement
    sqlite3_bind_text(stmt, 1, name.c_str(), -1, SQLITE_STATIC); // Name is mandatory

    // Description binding
    if (description.empty()) {
        sqlite3_bind_null(stmt, 2);
    } else {
        sqlite3_bind_text(stmt, 2, description.c_str(), -1, SQLITE_STATIC);
    }

    // Priority binding
    if (priority.empty()) {
        sqlite3_bind_null(stmt, 3);
    } else {
        sqlite3_bind_text(stmt, 3, priority.c_str(), -1, SQLITE_STATIC);
    }

    // Deadline binding
    if (deadline.empty()) {
        sqlite3_bind_null(stmt, 4);
    } else {
        sqlite3_bind_text(stmt, 4, deadline.c_str(), -1, SQLITE_STATIC);
    }

    // Execute the statement
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "Error inserting data: " << sqlite3_errmsg(db) << std::endl;
        sqlite3_finalize(stmt);
        return -1;
    }

    sqlite3_finalize(stmt); // Finalize the statement to release resources
    return 0;
}



int print_table_raw(sqlite3* db){
    // Query the table and print results
    const char* data = "Query Results:";
    string selectSQL = "SELECT * FROM tasks;";
    char* errorMessage = nullptr;

    int rc = sqlite3_exec(db, selectSQL.c_str(), callback, (void*)data, &errorMessage);
    if (rc != SQLITE_OK) {
        std::cerr << "Error selecting data: " << errorMessage << std::endl;
        sqlite3_free(errorMessage);
        return -1;
    } else {
        cout << "Data selected successfully!\n";
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
        cout << string(15 * argc, '-') << "\n"; // Separator line
        headerPrinted = true;
    }

    // Print row data
    for (int i = 0; i < argc; i++) {
        cout << std::setw(15) << std::left << (argv[i] ? argv[i] : "NULL");
    }
    cout << "\n";

    return 0;
}