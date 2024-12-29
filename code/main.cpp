#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <regex>
#include "database.hpp"

#define VERSION "v. 0.0.1/test"

using std::cout, std::cin, std::string;

int lines_printed = 0;

void print_intro();
//Function to INSERT data into db
int create_entry(int parentID = 0);
//goal menu
void goal_menu();

int main(){
    Database::initialize_database();

    char choice;
    do {
        print_intro();
        cout << " \033[2mGoals\033[m\033[36m[G]\033[m | \033[2mSchedule\033[m\033[36m[S]\033[m | \033[2mExit\033[m\033[36m[X]\033[m\n > ";
        cin >> choice;
        cin.ignore(); // Ignore leftover newline
        switch (choice) {
            case 'G':
            case 'g':
                goal_menu();
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

    return 0;
}

void print_intro(){
    //be PRODUCTIVE ver. ...
    cout << "\033[s\n\033[1m" <<
        "\033[53m"<< string(58, ' ') << "\033[m\n\033[1m" <<
        " #            \033[33m####  ####   ###\033[m\033[1m    #             #        \n" <<
        " #           \033[33m#   # #   # #   #\033[m\033[1m    #                      \n" <<
        " ##  ###    \033[33m####  ####  #   #\033[m\033[1m    ## # # ### ### # # # ###\n" <<
        " # # ###   \033[33m#     #  #  #   #\033[m\033[1m    # # # # #    #  # # # ###\n" <<
        " ##  ###  \033[33m#     #   #  ###\033[m\033[1m       ## ### ###  #  #  #  ###\n" << 
        "\033[4m"<< string(58, ' ') << "\033[m\n" <<
        " \033[4m\033[94mhttps://github.com/uwuebu/task_tracker\033[m\t" << VERSION << "\n\n";
}

struct Goal {
        int id;
        string name;
        Goal(int p_id, const string& p_name)
            : id(p_id), name(p_name) {}
    };

struct Task {
    int id;
    string name;
    int completion;
    int level;

    Task(int id, const string& name, int completion, int level)
        : id(id), name(name), completion(completion), level(level) {}
};

// Helper function to calculate displayed width
size_t get_displayed_width(const string& text) {
    // Remove ANSI escape sequences using regex
    std::regex escapeSeq("\033\\[[0-9;]*m");
    string cleanedText = std::regex_replace(text, escapeSeq, "");
    return cleanedText.length();
}

void fetch_tasks(int parentID, int level, std::vector<Task>& tasks) {
    // Fetch results for the current level
    auto result = Database::fetch_results("SELECT taskID, name, completion FROM tasks WHERE parentID = " + std::to_string(parentID) + ";");

    for (auto& row : result) {
        // Parse the row to extract task details
        int taskID = std::stoi(row[0]);
        const std::string& taskName = row[1];  // Assuming row[1] contains the task name
        int completion = std::stoi(row[2]);

        // Add the task to the list
        tasks.emplace_back(taskID, taskName, completion, level);

        // Recursive call to fetch subtasks
        fetch_tasks(taskID, level + 1, tasks);
    }
}


int print_goal(int maxWidth, int& selectedIndexGoals, int& selectedIndexTasks) {
    std::vector<std::vector<string>> result = Database::fetch_results("SELECT taskID, name FROM tasks WHERE parentID IS NULL;");
    
    std::vector<Goal> goals;

    for(auto& row : result){
        goals.emplace_back(stoi(row[0]), row[1]);
    }

    goals.emplace_back(0, "\033[36m[G]\033[m\033[2mAdd new goal\033[m");

    // Handle edge cases for selectedIndex
    if (selectedIndexGoals >= static_cast<int>(goals.size() - 1)) {
        selectedIndexGoals = goals.size() - 2;
    } else if (selectedIndexGoals < 0) {
        selectedIndexGoals = 0;
    }

    // Format output within the maxWidth constraint
    string currentLine;
    for (size_t i = 0; i < goals.size(); ++i) {
        string formattedGoal;
        if (static_cast<int>(i) == selectedIndexGoals) {
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

    // Tasks part:
    std::vector<Task> tasks;
    fetch_tasks(goals[selectedIndexGoals].id, 0, tasks); // Fetch all tasks starting from root

    // Handle edge cases for selectedIndex
    if (selectedIndexTasks >= static_cast<int>(tasks.size())) {
        selectedIndexTasks = tasks.size() - 1;
    } else if (selectedIndexTasks < 0) {
        selectedIndexTasks = 0;
    }

    cout << " \033[2m^Up\033[m\033[36m[U]\033[m "
        << string(58 - (std::string(" ^Up[U] Select task[S] Delete task[K] ").length() + 1), ' ')
        << " \033[2mSelect task\033[m\033[36m[S]\033[m \033[2mDelete task\033[m\033[36m[K]\033[m\n";

    for (size_t i = 0; i < tasks.size(); ++i) {
        const Task& task = tasks[i];
        string taskName = string(task.level*4, ' ') + task.name;


        // Highlight selected task
        if (static_cast<int>(i) == selectedIndexTasks) {
            taskName = "\033[7m" + taskName + "\033[m";
        }

        // Ensure taskName fits within maxWidth
        size_t displayedWidth = get_displayed_width(taskName);
        if (displayedWidth < static_cast<size_t>(maxWidth)) {
            taskName += string(maxWidth - displayedWidth, '.');
        } else if (displayedWidth > static_cast<size_t>(maxWidth)) {
            taskName = taskName.substr(0, maxWidth - 3) + "...";
        }

        cout << taskName << '\n';
    }
    cout << "\033[2m v Down\033[m\033[36m[D]\033[m\n";
    cout << "\033[2m Add new task\033[m\033[36m[T]\033[m\n";

    return goals[selectedIndexGoals].id;
}

void goal_menu() {
    cout << "\033[u\033[0J";
    char input;
    int selectedIndexGoals = 0;
    int selectedIndexTasks = 0;

    do {
        cout << "\033[53m"<< string(58, ' ') << "\033[m\n";
        int goalID = print_goal(58, selectedIndexGoals, selectedIndexTasks);
        cout <<"\033[4m"<< string(58, ' ') << "\033[m\n"
         << "\033[2m Back\033[m\033[36m[X]\033[m\n"
        << "Enter what's inside square brackets for navigation:\n > ";
        cin >> input;

        switch (input) {
            case 'G':
            case 'g':
                cout << "\033[u\033[0J";
                create_entry();
                cout << "\033[u\033[0J";
                break;

            case 'P':
            case 'p':
                selectedIndexGoals--; // Move to previous goal
                break;

            case 'N':
            case 'n':
                selectedIndexGoals++; // Move to next goal
                break;

            case 'T':
            case 't':
                cout << "\033[u\033[0J";
                create_entry(goalID);
                cout << "\033[u\033[0J";
                break;
            
            case 'U':
            case 'u':
                selectedIndexTasks--;
                break;
            
            case 'D':
            case 'd':
                selectedIndexTasks++;
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


int create_entry(int parentID) {
    cin.ignore();

    // Collect user input
    std::string name, description, priority, deadline;

    // Name is mandatory
    std::cout << "Enter name:\n > ";
    std::getline(std::cin, name);
    if (name.empty()) {
        std::cerr << "Goal name cannot be empty.\n";
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
