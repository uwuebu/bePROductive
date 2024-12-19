#include "task.hpp"
#include <thread>

using std::cout, std::cin, std::getline;

const string bigDigits[10] = {
    "  #######  \n"
    " ##     ## \n"
    " ##     ## \n"
    " ##     ## \n"
    " ##     ## \n"
    " ##     ## \n"
    "  #######  \n",

    "     ##    \n"
    "   ####    \n"
    "  ## ##    \n"
    "     ##    \n"
    "     ##    \n"
    "     ##    \n"
    " ######### \n",

    "  #######  \n"
    " ##     ## \n"
    "        ## \n"
    "  #######  \n"
    " ##        \n"
    " ##        \n"
    " ######### \n",

    "  #######  \n"
    " ##     ## \n"
    "        ## \n"
    "   ######  \n"
    "        ## \n"
    " ##     ## \n"
    "  #######  \n",

    "      ###  \n"
    "     ####  \n"
    "    ## ##  \n"
    "   ##  ##  \n"
    " ######### \n"
    "       ##  \n"
    "       ##  \n",

    " ######### \n"
    " ##        \n"
    " ##        \n"
    "  #######  \n"
    "        ## \n"
    " ##     ## \n"
    "  #######  \n",

    "  #######  \n"
    " ##     ## \n"
    " ##        \n"
    " ########  \n"
    " ##     ## \n"
    " ##     ## \n"
    "  #######  \n",

    " ######### \n"
    "        ## \n"
    "       ##  \n"
    "      ##   \n"
    "     ##    \n"
    "    ##     \n"
    "   ##      \n",
    
    "  #######  \n"
    " ##     ## \n"
    " ##     ## \n"
    "  #######  \n"
    " ##     ## \n"
    " ##     ## \n"
    "  #######  \n",
    
    "  #######  \n"
    " ##     ## \n"
    " ##     ## \n"
    "  ######## \n"
    "        ## \n"
    " ##     ## \n"
    "  #######  \n"
};

void fancy_separator(int length){
    cout << "\n\n\t";
    for(int i = 0; i < length; i++){
        cout << "-";
    }
    cout << "\n\t";
    for(int i = 0; i < length; i++){
        cout << "0";
    }
    cout << "\n\t";
    for(int i = 0; i < length; i++){
        cout << "-";
    }
    cout << "\n";
}

string generate_progress_bar(int timeSpent, int timeGoal, size_t barWidth = 25) {
    if (timeGoal == 0) return string(barWidth, '-'); // Handle division by zero

    double progress = static_cast<double>(timeSpent) / timeGoal;
    size_t fullSegments = static_cast<size_t>(progress * barWidth);
    double fractionalSegment = (progress * barWidth) - fullSegments;

    string progressBar;

    // Add filled squares
    progressBar.append(fullSegments, '#');

    // Add half-filled square if fractionalSegment is significant
    if (fractionalSegment >= 0.5 && fullSegments < barWidth) {
        progressBar += '=';
        ++fullSegments;
    }

    // Add empty squares
    progressBar.append(barWidth - fullSegments, '-');

    return progressBar;
}

string format_fixed_width(const string& input, size_t width) {
    if (input.length() > width) {
        return input.substr(0, width); // Trim the string if it's too long
    }
    return input + string(width - input.length(), ' '); // Pad with spaces
}

void createTask(TaskManager& tasks) {
    string name, description;
    int timeGoal;

    cout << "Enter task name: "; getline(cin, name);
    cout << "Enter task description: "; getline(cin, description);
    cout << "Enter time goal (in minutes): "; cin >> timeGoal;

    tasks.add_task(name, description, timeGoal);

    cout << "Task created successfully!\n";
}

void showTasks(TaskManager& taskManager) {
    if (taskManager.empty()) { // Check if there are no tasks
        cout << "No tasks available.\n";
        return;
    }

    cout << "List of tasks:\n";

    for (const auto& task : taskManager.get_all_tasks()) { // Iterate through all tasks
        cout << "   Task ID: " << task.taskID << "\n"
                  << "   Name: " << task.name << "\n"
                  << "   Description: " << task.description << "\n"
                  << "   Time Goal: " << task.timeGoalMinutes << " minutes\n"
                  << "   Time Completed: " << task.timeCompletedMinutes << " minutes\n\n";
    }
}

void goal_reached(TaskManager& tasks, Database& database, int taskID){
    cout << 
    "\t*****************************\n" <<
    "\t ///!!!CONGRATULATIONS!!!\\\\\\\n" <<
    "\t*****************************\n" <<
    "You have reached ur goal for this task ⸜(｡˃ ᵕ ˂ )⸝♡\n";
    
    int choice;
    auto& task = tasks.get_task_data(taskID);
    int newTimeGoal;
    do{
        cout << "0 - Delete Task | 1 - Set new goal\n" << 
                "Enter your choice: ";
        cin >> choice;
        cin.ignore();

        switch(choice){
            case 0:
                tasks.delete_task(taskID, database);
                break;
            case 1:
                cout << "\nEnter new goal (in minutes): ";
                cin >> newTimeGoal;
                tasks.change_task_data(taskID, task.name, task.description, newTimeGoal, 0);
                break;
            default:
                cout << "\nInvalid input. Please try again.\n";
                break;
        };
    }while(choice != 0 || choice != 1);
}

void startTask(TaskManager& tasks, Database& database) {
    if (tasks.empty()) {
        cout << "No tasks available to start.\n";
        return;
    }

    showTasks(tasks);

    size_t taskID;
    cout << "Enter the task ID to start: ";
    cin >> taskID;
    cin.ignore();

    auto task = tasks.get_task_data(taskID);

    cout << "Starting task: " << task.name << "\n"
         << "Controls: [P] Pause, [C] Continue, [X] Stop\n";

    auto startTime = std::chrono::steady_clock::now();
    bool running = true;
    bool paused = false;
    std::chrono::steady_clock::time_point pauseStartTime;
    int pausedSeconds = 0;
    bool first = 1;
    // Start a thread to handle timer display
    std::thread timerThread([&]() {
        while (running) {
            if (!paused) {
                auto currentTime = std::chrono::steady_clock::now();
                int elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count() - pausedSeconds;
                int hours = elapsedSeconds / 3600;
                int minutes = elapsedSeconds / 60;
                int seconds = elapsedSeconds % 60;

                // Convert hours, minutes, and seconds into digits
                int hourTens = hours / 10;
                int hourOnes = hours % 10;
                int minTens = minutes / 10;
                int minOnes = minutes % 10;
                int secTens = seconds / 10;
                int secOnes = seconds % 10;

                if (!first) {
                    // Clear previous display (assume it spans 7 lines)
                    cout << "\033[18A"; // Move cursor up 17 lines to overwrite previous output
                }

                fancy_separator(82);
                cout << "\n";
                // Display the timer using bigDigits
                for (int line = 0; line < 7; ++line) { // Assuming each big digit has 7 lines
                    cout<< "\t" << bigDigits[hourTens].substr(line * 12, 11) << "  "  // Tens of hours
                        << bigDigits[hourOnes].substr(line * 12, 11)  // Ones of hours
                        << "  :  "  // Separator
                        << bigDigits[minTens].substr(line * 12, 11) << "  "  // Tens of minutes
                        << bigDigits[minOnes].substr(line * 12, 11)  // Ones of minutes
                        << "  :  "  // Separator
                        << bigDigits[secTens].substr(line * 12, 11) << "  "  // Tens of seconds
                        << bigDigits[secOnes].substr(line * 12, 11) << "\n"; // Ones of seconds
                }

                fancy_separator(82);

                first = 0;
                cout.flush();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Update every second
        }
    });

    while (running) {
        char command;
        cin >> command;
        cin.ignore();

        switch (command) {
            case 'P':
            case 'p': // Pause
                if (!paused) {
                    paused = true;
                    pauseStartTime = std::chrono::steady_clock::now();
                    cout << "\nTimer paused. Press [C] to continue or [X] to stop.\n";
                }
                break;

            case 'C':
            case 'c': // Continue
                if (paused) {
                    paused = false;
                    auto pauseEndTime = std::chrono::steady_clock::now();
                    pausedSeconds += std::chrono::duration_cast<std::chrono::seconds>(pauseEndTime - pauseStartTime).count();
                    cout << "\nTimer continued. Press [P] to pause or [X] to stop.\n";
                    cout << "\033[6A\033[0J";
                }
                break;

            case 'X':
            case 'x': // Stop
                running = false;
                break;

            default:
                cout << "\nInvalid command. Use [P] to pause, [C] to continue, [X] to stop.\n";
        }
    }

    timerThread.join(); // Wait for the timer thread to finish

    auto endTime = std::chrono::steady_clock::now();
    int elapsedMinutes = std::chrono::duration_cast<std::chrono::minutes>(endTime - startTime).count();
    elapsedMinutes -= pausedSeconds / 60;

    task.timeCompletedMinutes += elapsedMinutes;
    tasks.change_task_data(taskID, task.name, task.description, task.timeGoalMinutes, task.timeCompletedMinutes);

    database.add_entry(task.taskID, elapsedMinutes, convert_time_to_string(startTime), convert_time_to_string(endTime), pausedSeconds/60);

    cout << "\nTask stopped. Time added: " << elapsedMinutes << " minutes.\n";

    if(task.timeCompletedMinutes < task.timeGoalMinutes){
        cout << "Overall progress:\t[" << generate_progress_bar(task.timeCompletedMinutes, task.timeGoalMinutes, 25) << "]\n";
    }else{
         goal_reached(tasks, database, taskID);
    }
}

void deleteTask(TaskManager& tasks, Database& database){
    showTasks(tasks);    
    size_t taskID;
    cout << "\nEnter task ID to delete:\n";
    cin >> taskID;
    cin.ignore();
    tasks.delete_task(taskID, database);
}

void display_day_info(Database& db, TaskManager& taskManager, const string& date) {
    // Fetch data for the current day
    json dayData = db.get_day_data(date);

    // Check if there are any entries for the current day
    if (dayData.empty()) {
        cout << "No data for the date: " << date << "\n";
        return;
    }

    // Calculate overall hours spent
    int totalMinutes = 0;
    for (const auto& session : dayData["sessions"]) {
        totalMinutes += session.at("timeSpent").get<int>();
    }

    // Display overall hours spent
    cout << "Date: " << date << "\n";
    cout << "Overall Hours Spent: " << totalMinutes / 60 << "h " 
         << totalMinutes % 60 << "m\n\n";

    // Display table of hours spent on each task with name and goal
    std::map<int, int> taskTimeMap;
    for (const auto& session : dayData["sessions"]) {
        taskTimeMap[session.at("taskID")] += session.at("timeSpent").get<int>();
    }

    cout << "Hours Spent on Individual Tasks:\n";
    cout << "--------------------------------------------------------------------------------\n";
    cout << "Task ID | Task Name        | Goal      | Spent     | Overall progress \n";
    cout << "--------|------------------|-----------|-----------|----------------------------\n";
    for (const auto& [taskID, timeSpent] : taskTimeMap) {
        try {
            const auto& task = taskManager.get_task_data(taskID);
            cout<< format_fixed_width(std::to_string(task.taskID), 7) << " | " 
                << format_fixed_width(task.name, 16) << " | " 
                << format_fixed_width(std::to_string(task.timeGoalMinutes / 60) + "h " + std::to_string(task.timeGoalMinutes % 60) + "m", 9) << " | "
                << format_fixed_width(std::to_string(timeSpent / 60) + "h " + std::to_string(timeSpent % 60) + "m", 9) << " | " 
                << "[" << generate_progress_bar(task.timeCompletedMinutes, task.timeGoalMinutes) << "]" << "\n";
        } catch (const std::out_of_range&) {
            cout << taskID << " -> [Task not found]";
        }
    }
    cout << "\n";

    // Display table of sessions
    cout << "Task Sessions:\n";
    cout << "-----------------------------------------------------------------------------\n";
    cout << "Task ID | Task Name        | Start     | End       | Pause Time | Time Spent\n";
    cout << "--------|------------------|-----------|-----------|------------|------------\n";
    // Create a sorted vector of sessions based on taskID
    vector<json> sortedSessions = dayData["sessions"].get<vector<json>>();
    std::sort(sortedSessions.begin(), sortedSessions.end(), [](const json& a, const json& b) {
        return a.at("taskID").get<int>() < b.at("taskID").get<int>();
    });

    for (const auto& session : sortedSessions) {
        try {
            const auto& task = taskManager.get_task_data(session["taskID"]);
            cout<< format_fixed_width(std::to_string(session.at("taskID").get<int>()), 7) << " | "
                << format_fixed_width(task.name, 16) << " | "
                << format_fixed_width(session.at("startTime").get<string>(), 9) << " | "
                << format_fixed_width(session.at("endTime").get<string>(), 9) << " | "
                << format_fixed_width(std::to_string(session.at("pauseTime").get<int>() / 60) + "h " + std::to_string(session.at("pauseTime").get<int>() % 60) + "m", 10) << " | "
                << format_fixed_width(std::to_string(session.at("timeSpent").get<int>() / 60) + "h " + std::to_string(session.at("timeSpent").get<int>() % 60) + "m", 10) << "\n";
        } catch (const std::out_of_range&) {
            cout << session["taskID"] << " -> [Task not found]";
    }
}
}

int main() {
    TaskManager taskList;
    Database database;

    char choice;
    do {
        cout << "\nMenu:\n1. Create Task | 2. Show Tasks | 3. Start Task | 4. Delete task\n5. Display Info for current day | 6. Display info for specific date\n7. Exit\nEnter your choice: ";
        cin >> choice;
        cin.ignore(); // Ignore leftover newline
        string dateInput;
        switch (choice) {
            case '1':
                createTask(taskList);
                break;
            case '2':
                showTasks(taskList);
                break;
            case '3':
                startTask(taskList, database);
                break;
            case '4':
                deleteTask(taskList, database);
                break;
            case '5':
                display_day_info(database, taskList, get_current_date());
                break;
            case '6':
                cout << "Specify the date which you would like to see info on (format: year-month-day, example: 2024-12-19):\n";
                cin >> dateInput;
                display_day_info(database, taskList, dateInput);
                break;
            case '7':
                cout << "Exiting...\n";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != '7');

    return 0;
}