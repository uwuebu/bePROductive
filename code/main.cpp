#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <chrono>
#include <regex>
#include "database.hpp"
#include <thread>

#define LINK "https://github.com/uwuebu/bePROductive"
#define VERSION "v. 0.0.1/test"

#define MAXWIDTH 58

using std::cout, std::cin, std::string;

void print_intro();
//Function to INSERT data into db
int create_entry(int parentID = 0);
//Function to delete task entry from daatabase
void delete_entry(int ID, const string& name);
//Menu to navigate projects and select tasks
void project_menu();
//Menu to see details about specific tasks
void task_menu(int ID);

int main(){
    Database::initialize_database();

    char choice;
    do {
        print_intro();
        cout << " \033[2mProjects\033[36m[G]\033[m | \033[2mSchedule (Coming soon)[S]\033[m | \033[2mExit\033[36m[X]\033[m\n";
        cout << " > ";
        cin >> choice;
        cin.ignore(); // Ignore leftover newline
        switch (choice) {
            case 'G':
            case 'g':
                project_menu();
                break;
            case 'S':
            case 's':
                //CalendarMenu
                break;
            case 'X':
            case 'x':
                cout << "Exiting...";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
        cout << "\033[u\033[0J";
    } while (choice != 'X' && choice != 'x');

    return 0;
}

void print_intro(){
    //be PRODUCTIVE ver. ...
    cout << "\033[s\n\033[1m" <<
        "\033[53m"<< string(MAXWIDTH, ' ') << "\033[m\n\033[1m" <<
        " #            \033[93m####  ####   ###\033[m\033[1m    #             #        \n" <<
        " #           \033[93m#   # #   # #   #\033[m\033[1m    #                      \n" <<
        " ##  ###    \033[93m####  ####  #   #\033[m\033[1m    ## # # ### ### # # # ###\n" <<
        " # # ###   \033[93m#     #  #  #   #\033[m\033[1m    # # # # #    #  # # # ###\n" <<
        " ##  ###  \033[93m#     #   #  ###\033[m\033[1m       ## ### ###  #  #  #  ###\n" << 
        "\033[4m"<< string(MAXWIDTH, ' ') << "\033[m\n" <<
        " \033[4m\033[94m" << LINK << "\033[m\t" << VERSION << "\n\n";
}

struct Entry {
    int id;
    string name;
    string description;
    int priority;
    string deadline;
    int completion;
    int level;
    Entry(){}
    Entry(int id, const string& name, const string& description = " ", int priority = 0, const string& deadline = " ", int completion = 0, int level = 0)
        : id(id), name(name), description(description), priority(priority), deadline(deadline), completion(completion), level(level) {}
};

struct Session {
    int id;
    int taskID;
    int timeSpent;   // Total time spent in seconds
    std::string date; // Date in "YYYY-MM-DD" format
    std::string comment;

    Session() {}
    Session(int id, int taskID, int timeSpent, const std::string& date, const std::string& comment)
        : id(id), taskID(taskID), timeSpent(timeSpent), date(date), comment(comment) {}
};

size_t get_displayed_width(const string& text) {
    // Remove ANSI escape sequences using regex
    std::regex escapeSeq("\033\\[[0-9;]*m");
    string cleanedText = std::regex_replace(text, escapeSeq, "");
    return cleanedText.length();
}

void fetch_tasks(int parentID, int level, std::vector<Entry>& tasks) {
    // Fetch results for the current level
    auto result = Database::fetch_results("SELECT taskID, name, completion FROM tasks WHERE parentID = " + std::to_string(parentID) + ";");

    for (auto row : result) {
        // Parse the row to extract task details
        int taskID = std::stoi(row[0]);
        const std::string& taskName = row[1];  // Assuming row[1] contains the task name
        int completion = std::stoi(row[2]);

        // Add the task to the list
        tasks.emplace_back(taskID, taskName, " ", 0, " ",completion, level);

        // Recursive call to fetch subtasks
        fetch_tasks(taskID, level + 1, tasks);
    }
}

string wrapText(const std::string& text, int maxWidth) {
    if (maxWidth <= 0) return text;  // Handle invalid maxWidth case

    std::vector<std::string> lines;
    std::istringstream stream(text);
    std::string word;
    std::string currentLine;
    int currentLength = 0;

    while (stream >> word) {
        if (currentLength + word.length() <= static_cast<unsigned int>(maxWidth)) {
            // Add word to current line if within maxWidth
            if (!currentLine.empty()) currentLine += " ";
            currentLine += word;
            currentLength += word.length() + 1; // +1 for space
        } else {
            // If adding the word exceeds maxWidth, store the current line
            if (!currentLine.empty()) {
                lines.push_back(currentLine);
            }

            // Start a new line
            currentLine = word;
            currentLength = word.length() + 1;
        }

        // If a single word is longer than maxWidth, force a break within the word
        while (currentLine.length() > static_cast<unsigned int>(maxWidth)) {
            lines.push_back(currentLine.substr(0, maxWidth));
            currentLine = currentLine.substr(maxWidth);
        }
    }

    // Add the last line if it's not empty
    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }

    // Combine lines into a single string with newlines
    std::ostringstream result;
    for (size_t i = 0; i < lines.size(); ++i) {
        if (i > 0) result << "\n";
        result << lines[i];
    }

    return result.str();
}

std::string colorize_number(int num) {
    switch (num) {
        case 1: return "\033[31m1\033[m"; // Red
        case 2: return "\033[33m2\033[m"; // Yellow
        case 3: return "\033[32m3\033[m"; // Green
        case 4: return "\033[34m4\033[m"; // Blue
        case 5: return "\033[35m5\033[m"; // Purple 
        default: return std::to_string(num); // Default: no color
    }
}

int days_difference(const string& date_str) {
    // Parse the input date string
    std::tm input_date = {};
    std::istringstream ss(date_str);
    ss >> std::get_time(&input_date, "%Y-%m-%d");

    if (ss.fail()) {
        return 0;
    }

    // Convert input date to time_point
    auto input_time = std::chrono::system_clock::from_time_t(std::mktime(&input_date));

    // Get current date
    auto now = std::chrono::system_clock::now();
    auto now_time = std::chrono::system_clock::to_time_t(now);
    std::tm current_date = *std::localtime(&now_time);
    auto current_time = std::chrono::system_clock::from_time_t(std::mktime(&current_date));

    // Calculate difference in days
    auto difference = std::chrono::duration_cast<std::chrono::hours>(input_time - current_time).count() / 24;

    return static_cast<int>(difference);
}

void update_entry_details(Entry& entry) {
    string query = "SELECT Description, Priority, Deadline, Completion FROM Tasks WHERE taskID = " + std::to_string(entry.id) + ";";
    
    auto result = Database::fetch_results(query);
    
    if (!result.empty()) { // Ensure there is a result
        entry.description = result[0][0]; 
        entry.priority = std::stoi(result[0][1]); 
        entry.deadline = result[0][2]; 
        entry.completion = std::stoi(result[0][3]);
    } else {
        std::cerr << "No task found with ID: " << entry.id << std::endl;
    }
}

void entries_display_horisontal(std::vector<Entry>& entries, int selectedIndex){
    // Format output within the maxWidth constraint
    string currentLine;
    for (size_t i = 0; i < entries.size(); ++i) {
        string formattedProject;
        if (static_cast<int>(i) == selectedIndex) {
            formattedProject = "\033[7m" + entries[i].name + "\033[m"; // Highlight selected
        } else {
            formattedProject = entries[i].name;
        }

        size_t formattedWidth = get_displayed_width(currentLine) + get_displayed_width(formattedProject) + 3;

        if (currentLine.empty()) {
            currentLine = formattedProject;
        } else if (formattedWidth <= static_cast<size_t>(MAXWIDTH)) {
            currentLine += " | " + formattedProject;
        } else {
            cout << currentLine << '\n';
            currentLine = formattedProject;
        }
    }

    // Print the last line if it exists
    if (!currentLine.empty()) {
        cout << currentLine << '\n';
    }

    cout << " \033[2m<<Previous\033[36m[P]\033[m "
        << string(MAXWIDTH - (std::string(" <<Previous[P] Next[N]>> ").length() + 1), ' ')
        << " \033[2mNext\033[36m[N]\033[m\033[2m>>\033[m\n";
}

string createProgressBar(int percent, int width) {
    // Ensure the percentage is within the valid range
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    // Calculate the number of '#' characters to display
    int filledWidth = (percent * width) / 100;

    // Create the progress bar string
    string bar = "|";
    for (int i = 0; i < width; ++i) {
        if (i < filledWidth) {
            bar += '#';
        } else {
            bar += ' ';
        }
    }
    bar += "|";

    return bar;
}

int get_total_time_spent(int taskID) {
    // Construct the full SQL query with the taskID directly embedded
    std::string query = 
        "WITH RECURSIVE TaskHierarchy AS ("
        "    SELECT taskID FROM Tasks WHERE taskID = " + std::to_string(taskID) + 
        "    UNION ALL "
        "    SELECT t.taskID FROM Tasks t "
        "    INNER JOIN TaskHierarchy th ON t.parentID = th.taskID"
        ") "
        "SELECT COALESCE(SUM(s.endTime - s.startTime - s.pausedTime), 0) "
        "FROM Sessions s "
        "INNER JOIN TaskHierarchy th ON s.taskID = th.taskID;";

    auto result = Database::fetch_results(query);

    return (!result.empty() && !result[0][0].empty()) ? std::stoi(result[0][0]) : 0;
}

std::string format_time(int seconds) {
    using namespace std::chrono;

    // Calculate hours, minutes, and seconds
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int remaining_seconds = seconds % 60;

    // Use stringstream to format the result
    std::stringstream time_stream;
    time_stream << std::setw(2) << std::setfill('0') << hours << ":"
                << std::setw(2) << std::setfill('0') << minutes << ":"
                << std::setw(2) << std::setfill('0') << remaining_seconds;

    return time_stream.str();
}

void entry_details_display(Entry& entry){
    int day_diff;
    string days_diff_str = "";
    if(entry.deadline != ""){
        day_diff = days_difference(entry.deadline);
        if(day_diff < 0){
            days_diff_str = " \033[31m(" + std::to_string(day_diff) + " d overdue)\033[m";
        }else{
            days_diff_str = " (" + std::to_string(day_diff) + " d left)";
        }
    }

    cout << "\033[53m"<< string(MAXWIDTH, ' ') << "\033[m\n";

    cout << "\033[2mName:\033[36m[R]\033[m " << entry.name
    << string(MAXWIDTH - string("Name[R]: Delete[J]" + entry.name).length(), ' ')
    << "\033[2mDelete\033[31m[J]\033[m\n";

    cout << "\033[2mTotal time: \033[m" << format_time(get_total_time_spent(entry.id)) << '\n';

    cout << "\033[2mPriority:\033[36m[Q]\033[m " << colorize_number(entry.priority) 
         << string(MAXWIDTH - 11 - string("Priority:[Q] Deadline:[W] ").length() - get_displayed_width(days_diff_str), ' ');
            cout << "\033[2mDeadline:\033[36m[W]\033[m " << entry.deadline;
         if(entry.deadline != "")   
            cout << "\033[2m" << days_diff_str << "\033[m";
         cout << '\n';
    
    cout << "\033[2mDescription:\033[36m[E]\033[m\n"
         << wrapText(entry.description, MAXWIDTH) << '\n';
    
    int bar_width = 20;
    cout << string((int)((MAXWIDTH-bar_width)/2), ' ') << createProgressBar(entry.completion, bar_width) << entry.completion << "%\n";
    
    cout << "\033[53m"<< string(MAXWIDTH, ' ') << "\033[m\n";
}

void entries_display_vertical(std::vector<Entry>& entries, int selectedIndex){
    cout << " \033[2m^Up\033[36m[U]\033[m "
        << string(MAXWIDTH - (std::string(" ^Up[U] Select task[S] Delete task[K] ").length() + 1), ' ')
        << " \033[2mSelect task\033[36m[S]\033[m \033[2mDelete task\033[36m[K]\033[m\n";

    for (size_t i = 0; i < entries.size(); ++i) {
        const Entry& entry = entries[i];
        string entryName = string(entry.level*2, ' ') + entry.name;


        // Highlight selected task
        if (static_cast<int>(i) == selectedIndex) {
            entryName = "\033[7m" + entryName + "\033[m";
            if(entry.completion == 100) entryName += "\033[2m\033[32m";
        }

        // Ensure taskName fits within maxWidth
        size_t displayedWidth = get_displayed_width(entryName);
        if (displayedWidth < MAXWIDTH - 7) {
            entryName += string(MAXWIDTH - 7 - displayedWidth, '.');
        } else if (displayedWidth > MAXWIDTH - 7) {
            entryName = entryName.substr(0, MAXWIDTH - 7 - 3) + "...";
        }

        entryName += createProgressBar(entry.completion, 5);

        if(entry.completion == 100){
            entryName = "\033[2m\033[32m" + entryName + "\033[m";
        }

        cout << entryName << "\033[m\n";
    }
    cout << " \033[2m v Down\033[36m[D]\033[m "
        << string(MAXWIDTH - (std::string(" v Down[D] Mark as Completed[Z]").length() + 1), ' ')
        << " \033[2mMark as Completed\033[36m[Z]\033[m\n";

    cout << "\033[2m Add new task\033[36m[T]\033[m\n";
}

void update_completion_status(int ID, int currentCompletion) {
    // Determine new completion value
    int newCompletion = (currentCompletion == 100) ? 0 : 100;

    // Construct SQL query
    std::string query = "UPDATE Tasks SET Completion = ? WHERE taskID = ?;";

    // Execute update
    std::vector<std::variant<string, int>> params = { newCompletion, ID };
    if (Database::execute_prepared_query(query, params) == 0) {
        std::cout << "\033[32mCompletion status updated successfully!\033[m\n";
    } else {
        std::cerr << "\033[31mFailed to update completion status.\033[m\n";
    }
}

void update_entry(int ID, const std::string& field) {
    std::string newValue;

    // Prompt the user for a new value
    std::cout << "Enter new " << field << ": ";
    std::getline(std::cin >> std::ws, newValue); // Read input including spaces

    // Input validation
    if (field == "Priority") {
        try {
            int priority = std::stoi(newValue);
            if (priority < 1 || priority > 5) {
                std::cerr << "\033[31mInvalid priority! Must be between 1 and 5.\033[m\n";
                return;
            }
        } catch (...) {
            std::cerr << "\033[31mInvalid priority! Enter a number.\033[m\n";
            return;
        }
    } else if (field == "Deadline") {
        if (newValue.size() != 10 || newValue[4] != '-' || newValue[7] != '-') {
            std::cerr << "\033[31mInvalid date format! Use YYYY-MM-DD.\033[m\n";
            return;
        }
    }

    // Construct SQL query
    std::string query = "UPDATE Tasks SET " + field + " = ? WHERE taskID = ?;";

    // Execute update
    std::vector<std::variant<std::string, int>> params = { newValue, ID };
    if (Database::execute_prepared_query(query, params) == 0) {
        std::cout << "\033[32m" << field << " updated successfully!\033[m\n";
    } else {
        std::cerr << "\033[31mFailed to update " << field << ".\033[m\n";
    }
}

void project_menu() {
    cout << "\033[u\033[0J";
    char input;
    int selectedIndexProjects = 0;
    int selectedIndexTasks = 0;

    do {
        cout << "\n";

        std::vector<std::vector<string>> result = Database::fetch_results("SELECT taskID, Name, Description, Priority, Deadline FROM Tasks WHERE parentID IS NULL;");
        
        // Projects part:
        std::vector<Entry> projects;
        for(auto row : result){
            projects.emplace_back(stoi(row[0]), row[1]);
        }
        projects.emplace_back(0, "\033[2m\033[36m[G]\033[m\033[2mAdd new project\033[m");
        // Handle edge cases for selectedIndex
        if (selectedIndexProjects >= static_cast<int>(projects.size() - 1)) {
            selectedIndexProjects = projects.size() - 2;
        } else if (selectedIndexProjects < 0) {
            selectedIndexProjects = 0;
        }

        entries_display_horisontal(projects, selectedIndexProjects);
        // Only call these functions if there are valid projects
        if (!projects.empty() && selectedIndexProjects >= 0) {
            update_entry_details(projects[selectedIndexProjects]);
            entry_details_display(projects[selectedIndexProjects]);
        }
        // Tasks part:
        std::vector<Entry> tasks;
        fetch_tasks(projects[selectedIndexProjects].id, 0, tasks); // Fetch all tasks starting from root
        // Handle edge cases for selectedIndex
        if (selectedIndexTasks >= static_cast<int>(tasks.size())) {
            selectedIndexTasks = tasks.size() - 1;
        } else if (selectedIndexTasks < 0) {
            selectedIndexTasks = 0;
        }
        cout << "\tTasks:\n";
        entries_display_vertical(tasks, selectedIndexTasks);

        cout <<"\033[4m"<< string(MAXWIDTH, ' ') << "\033[m\n" //divider
         << "\033[2m Back\033[36m[X]\033[m\n";
        cout << " > ";
        cin >> input;

        switch (input) {
            case 'S': case 's':
                cout << "\033[u\033[0J";
                task_menu(tasks[selectedIndexTasks].id);
                break;
            case 'G': case 'g':
                cout << "\033[u\033[0J";
                create_entry();
                break;
            case 'P': case 'p':
                selectedIndexProjects--; 
                break;
            case 'N': case 'n':
                selectedIndexProjects++; 
                break;
            case 'T': case 't':
                cout << "\033[u\033[0J";
                create_entry(projects[selectedIndexProjects].id);
                break;
            case 'U': case 'u':
                selectedIndexTasks--;
                break;
            case 'D': case 'd':
                selectedIndexTasks++;
                break;
            case 'K': case 'k':
                cout << "\033[u\033[0J";
                delete_entry(tasks[selectedIndexTasks].id, tasks[selectedIndexTasks].name);
                break;
            case 'J': case 'j':
                cout << "\033[u\033[0J";
                delete_entry(projects[selectedIndexProjects].id, projects[selectedIndexProjects].name);
                break;
            case 'R': case 'r':
                cout << "\033[u\033[0J";
                update_entry(projects[selectedIndexProjects].id, "Name");
                break;
            case 'Q': case 'q':
                cout << "\033[u\033[0J";
                update_entry(projects[selectedIndexProjects].id, "Priority");
                break;
            case 'W': case 'w':
                cout << "\033[u\033[0J";
                update_entry(projects[selectedIndexProjects].id, "Deadline");
                break;
            case 'E': case 'e':
                cout << "\033[u\033[0J";
                update_entry(projects[selectedIndexProjects].id, "Description");
                break;
            case 'Z': case 'z':
                update_completion_status(tasks[selectedIndexTasks].id, tasks[selectedIndexTasks].completion);
                break;
            case 'X': case 'x':
                break;
            default:
                cout << "\033[31mInvalid input! Try again.\033[m\n";
                break;
        }

        cout << "\033[u\033[0J"; // Clear previous output
    } while (input != 'X' && input != 'x');
}

int create_entry(int parentID) {
    cin.ignore();

    // Collect user input
    std::string name, description, priority, deadline;

    // Name is mandatory
    std::cout << "Enter name:\n > ";
    std::getline(std::cin, name);
    if (name.empty()) {
        std::cerr << "Project name cannot be empty.\n";
        return -1;
    }

    // Optional fields
    std::cout << "Enter description:\n > ";
    std::getline(std::cin, description);

    std::cout << "Enter priority (1-5):\n > ";
    std::getline(std::cin, priority);

    std::cout << "Enter deadline (format: YYYY-MM-DD):\n > ";
    std::getline(std::cin, deadline);

    // Prepare data for insertion
    std::vector<std::variant<std::string, int, std::nullptr_t>> data;

    data.push_back(name); // Mandatory name

    data.push_back(parentID == 0 
    ? std::variant<std::string, int, std::nullptr_t>(nullptr) 
    : std::variant<std::string, int, std::nullptr_t>(parentID));
    data.push_back(description.empty()
    ? std::variant<std::string, int, std::nullptr_t>(nullptr)  
    : std::variant<std::string, int, std::nullptr_t>(description)); // Description
    data.push_back(priority.empty() 
    ? std::variant<std::string, int, std::nullptr_t>(nullptr)  
    : std::variant<std::string, int, std::nullptr_t>(priority)); // Priority
    data.push_back(deadline.empty() 
    ? std::variant<std::string, int, std::nullptr_t>(nullptr)  
    : std::variant<std::string, int, std::nullptr_t>(deadline)); // Deadline

    // Insert data using the Database class
    std::string sql = "INSERT INTO tasks (name, parentID, description, priority, deadline) VALUES (?, ?, ?, ?, ?);";
    if (Database::insert_data(sql, data) != 0) {
        return -1;
    }

    return 0;
}

void delete_entry(int ID, const string& name){
    char input;
    cout << "Are you sure you want to delete " << name << ", any subtasks and related data will also be deleted: "<< " (Y/N): ";
    cin >> input;
    do{
        switch(input){
            case 'Y':
            case 'y':
                Database::delete_entry(ID, "Tasks");
                break;
            case 'N':
            case 'n':
                break;
            default:
                cout << "\033[31mInvalid input! Try again.\033[m\n";
                break;
        }
    }while(input != 'N' && input != 'n' && input != 'Y' && input != 'y');
}

void delete_session(int ID){
    char input;
    cout << "Are you sure you want to delete this session "<< " (Y/N): ";
    cin >> input;
    do{
        switch(input){
            case 'Y':
            case 'y':
                Database::delete_entry(ID, "Sessions");
                break;
            case 'N':
            case 'n':
                break;
            default:
                cout << "\033[31mInvalid input! Try again.\033[m\n";
                break;
        }
    }while(input != 'N' && input != 'n' && input != 'Y' && input != 'y');
}

void fetch_task_info(Entry& entry){
    string query = "SELECT Name, Description, Priority, Deadline, Completion FROM Tasks WHERE taskID = " + std::to_string(entry.id) + ";";

    auto result = Database::fetch_results(query);

    if (!result.empty()) { // Ensure there is a result
        entry.name = result[0][0]; 
        entry.description = result[0][1]; 
        entry.priority = std::stoi(result[0][2]);
        entry.deadline = result[0][3];
        entry.completion = std::stoi(result[0][4]); 
    } else {
        std::cerr << "No task found with ID: " << entry.id << std::endl;
    }
}

void fetch_sessions(int ID, std::vector<Session>& sessions) {
    auto result = Database::fetch_results(
        "SELECT sessionID, (endTime - startTime - pausedTime) AS timeSpent, "
        "DATE(startTime, 'unixepoch') AS sessionDate, comment "
        "FROM sessions WHERE taskID = " + std::to_string(ID) + ";"
    );

    for (const auto& row : result) {
        sessions.emplace_back(std::stoi(row[0]), ID, std::stoi(row[1]), row[2], row[3]);
    }
}

void sessions_display_vertical(const std::vector<Session>& sessions, int selectedIndex) {
    // Table header
    cout << "\tSessions:\n" << " \033[2m^Up\033[36m[O]\033[m "
         << string(MAXWIDTH - (std::string(" ^Up[O] Delete session[H] ").length() + 1), ' ')
         << "\033[2mDelete session\033[36m[H]\033[m\n";

    cout << "\n Date      | Time     | Comment\n";

    // Display each session
    for (size_t i = 0; i < sessions.size(); ++i) {
        const Session& session = sessions[i];

        // Convert timeSpent (stored in seconds) to HH:MM:SS format
        int hours = session.timeSpent / 3600;
        int minutes = (session.timeSpent % 3600) / 60;
        int seconds = session.timeSpent % 60;
        std::string timeSpent = (hours < 10 ? "0" : "") + std::to_string(hours) + ":" +
                                (minutes < 10 ? "0" : "") + std::to_string(minutes) + ":" +
                                (seconds < 10 ? "0" : "") + std::to_string(seconds);

        // Format the session details into a string
        std::string sessionDetails = session.date + " | " + timeSpent + " | " + session.comment;

        // Highlight the selected session
        if (static_cast<int>(i) == selectedIndex) {
            sessionDetails = "\033[7m" + sessionDetails + "\033[m";
        }

        // Ensure the session details fit within MAXWIDTH
        size_t displayedWidth = get_displayed_width(sessionDetails);
        if (displayedWidth > MAXWIDTH) {
            sessionDetails = sessionDetails.substr(0, MAXWIDTH - 3) + "...";
        } else if (displayedWidth < MAXWIDTH) {
            sessionDetails += std::string(MAXWIDTH - displayedWidth, ' ');
        }

        cout << sessionDetails << '\n';
    }

    // Footer
    cout << "\033[2m v Down\033[36m[L]\033[m\n";
    cout << "\033[2m Start new session\033[36m[C]\033[m\n";
}

int get_current_time() {
    return static_cast<int>(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now()));
}

void start_session_menu(Entry& task) {
    // Get current time as an integer timestamp
    int startTime = get_current_time();

    bool running = true, paused = false;
    int pausedSeconds = 0;
    auto startTimePoint = std::chrono::steady_clock::now();
    auto pauseStartTime = startTimePoint;

    std::cout << "\n Started session for task: " << task.name << "\n"
              << " Controls: \033[2m Pause\033[36m[P]\033[m, \033[2m Stop\033[36m[X]\033[m\n";

    std::thread timerThread([&]() {
        while (running) {
            if (!paused) {
                auto elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(
                                          std::chrono::steady_clock::now() - startTimePoint)
                                          .count() - pausedSeconds;
                int hours = elapsedSeconds / 3600;
                int minutes = (elapsedSeconds / 60) % 60;
                int seconds = elapsedSeconds % 60;

                std::cout << "\r Elapsed Time: " << std::setw(2) << std::setfill('0') << hours << ":"
                          << std::setw(2) << std::setfill('0') << minutes << ":"
                          << std::setw(2) << std::setfill('0') << seconds << "    " << std::flush;
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    });

    while (running) {
        char command;
        std::cin >> command;
        std::cin.ignore();

        switch (command) {
            case 'P': case 'p':
                if (!paused) {
                    paused = true;
                    pauseStartTime = std::chrono::steady_clock::now();
                    std::cout << "\n Timer paused. Press [C] to continue or [X] to stop.\n";
                }
                break;
            case 'C': case 'c':
                if (paused) {
                    paused = false;
                    pausedSeconds += std::chrono::duration_cast<std::chrono::seconds>(
                                         std::chrono::steady_clock::now() - pauseStartTime)
                                         .count();
                    std::cout << "\033[4A\033[0J";
                } else {
                    std::cout << "\033[1A\033[0J";
                }
                break;
            case 'X': case 'x':
                running = false;
                break;
            default:
                std::cout << "\n Invalid command. Use [P] to pause, [C] to continue, [X] to stop.\n";
                std::cout << "\033[4A\033[0J";
        }
    }

    timerThread.join();

    // Get end time
    int endTime = get_current_time();

    // Prompt user for a comment
    std::string comment;
    std::cout << "\n Enter a comment for this session (or press Enter to skip): ";
    std::getline(std::cin, comment);

    // Trim whitespace (optional but useful)
    comment.erase(comment.find_last_not_of(" \t\n\r\f\v") + 1);
    comment.erase(0, comment.find_first_not_of(" \t\n\r\f\v"));

    // Insert session into the database
    std::string query = "INSERT INTO Sessions (taskID, startTime, endTime, pausedTime, comment) VALUES (?, ?, ?, ?, ?);";
    Database::insert_data(query, {task.id, startTime, endTime, pausedSeconds, comment});
}

void task_menu(int ID){
    cout << "\033[u\033[0J";

    char input;
    int selectedIndexSubT = 0;
    int selectedIndexSession = 0;

    do{
        Entry task;
        task.id = ID;
        std::vector<Entry> subtasks;
        
        fetch_task_info(task);
        fetch_tasks(task.id, 0, subtasks);
        // Handle edge cases for selectedIndex
        if (selectedIndexSubT >= static_cast<int>(subtasks.size())) {
            selectedIndexSubT = subtasks.size() - 1;
        } else if (selectedIndexSubT < 0) {
            selectedIndexSubT = 0;
        }
        cout << '\n';
        entry_details_display(task);
        cout << "\tSubtasks:\n";
        entries_display_vertical(subtasks, selectedIndexSubT);

        std::vector<Session> sessions;
        fetch_sessions(task.id, sessions);
        // Handle edge cases for selectedIndexSessions
        if (selectedIndexSession >= static_cast<int>(sessions.size())) {
            selectedIndexSession = sessions.size() - 1;
        } else if (selectedIndexSession < 0) {
            selectedIndexSession = 0;
        }

        cout <<"\033[4m"<< string(MAXWIDTH, ' ') << "\033[m\n"; //divider

        sessions_display_vertical(sessions, selectedIndexSession);

        cout <<"\033[4m"<< string(MAXWIDTH, ' ') << "\033[m\n" //divider
         << "\033[2m Back\033[36m[X]\033[m\n";

        cout << " > ";
        cin >> input;

        switch(input){
            case 'S': case 's':
                cout << "\033[u\033[0J";
                task_menu(subtasks[selectedIndexSubT].id);
                break;
            case 'T': case 't':
                cout << "\033[u\033[0J";
                create_entry(task.id);
                break;
            case 'U': case 'u':
                selectedIndexSubT--;
                break;
            case 'D': case 'd':
                selectedIndexSubT++;
                break;
            case 'K': case 'k':
                cout << "\033[u\033[0J";
                delete_entry(subtasks[selectedIndexSubT].id, subtasks[selectedIndexSubT].name);
                break;
            case 'J': case 'j':
                cout << "\033[u\033[0J";
                delete_entry(task.id, task.name);
                break;
            case 'R': case 'r':
                cout << "\033[u\033[0J";
                update_entry(task.id, "Name");
                break;
            case 'Q': case 'q':
                cout << "\033[u\033[0J";
                update_entry(task.id, "Priority");
                break;
            case 'W': case 'w':
                cout << "\033[u\033[0J";
                update_entry(task.id, "Deadline");
                break;
            case 'E': case 'e':
                cout << "\033[u\033[0J";
                update_entry(task.id, "Description");
                break;
            case 'Z': case 'z':
                update_completion_status(subtasks[selectedIndexSubT].id, subtasks[selectedIndexSubT].completion);
                break;
            case 'O': case 'o':
                selectedIndexSession--;
                break;
            case 'L': case 'l':
                selectedIndexSession++;
                break;
            case 'C': case 'c':
                cout << "\033[u\033[0J";
                start_session_menu(task);
                break;
            case 'H': case 'h':
                delete_session(sessions[selectedIndexSession].id);
                break;
            case 'X': case 'x':
                break;
            default:
                cout << "\033[31mInvalid input! Try again.\033[m\n";
                break;
        }
    cout << "\033[u\033[0J";
    }while(input != 'X' && input != 'x');
}