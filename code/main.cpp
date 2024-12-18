#include "task.hpp"
#include <thread>

using std::cout, std::cin, std::getline;

const std::string bigDigits[10] = {
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


void createTask(TaskManager& tasks) {
    std::string name, description;
    int timeGoal;

    cout << "Enter task name: "; getline(cin, name);
    cout << "Enter task description: "; getline(cin, description);
    cout << "Enter time goal (in minutes): "; cin >> timeGoal;

    tasks.add_task(name, description, timeGoal);

    cout << "Task created successfully!\n";
}

void showTasks(TaskManager& taskManager) {
    if (taskManager.empty()) { // Check if there are no tasks
        std::cout << "No tasks available.\n";
        return;
    }

    std::cout << "List of tasks:\n";

    for (const auto& task : taskManager.get_all_tasks()) { // Iterate through all tasks
        std::cout << "   Task ID: " << task.taskID << "\n"
                  << "   Name: " << task.name << "\n"
                  << "   Description: " << task.description << "\n"
                  << "   Time Goal: " << task.timeGoalMinutes << " minutes\n"
                  << "   Time Completed: " << task.timeCompletedMinutes << " minutes\n\n";
    }
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

    std::cout << "Starting task: " << task.name << "\n"
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
                    std::cout << "\033[18A"; // Move cursor up 17 lines to overwrite previous output
                }

                fancy_separator(82);
                cout << "\n";
                // Display the timer using bigDigits
                for (int line = 0; line < 7; ++line) { // Assuming each big digit has 7 lines
                    std::cout << "\t" << bigDigits[hourTens].substr(line * 12, 11) << "  "  // Tens of hours
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
                std::cout.flush();
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
                    std::cout << "\033[6A\033[0J";
                }
                break;

            case 'X':
            case 'x': // Stop
                running = false;
                break;

            default:
                std::cout << "\nInvalid command. Use [P] to pause, [C] to continue, [X] to stop.\n";
        }
    }

    timerThread.join(); // Wait for the timer thread to finish

    auto endTime = std::chrono::steady_clock::now();
    int elapsedMinutes = std::chrono::duration_cast<std::chrono::minutes>(endTime - startTime).count();
    elapsedMinutes -= pausedSeconds / 60;

    task.timeCompletedMinutes += elapsedMinutes;
    tasks.change_task_data(taskID, task.name, task.description, task.timeGoalMinutes, task.timeCompletedMinutes);

    database.add_entry(task.taskID, elapsedMinutes, convert_time_to_string(startTime), convert_time_to_string(endTime), pausedSeconds/60);

    std::cout << "\nTask stopped. Time added: " << elapsedMinutes << " minutes.\n";
}

void deleteTask(TaskManager& tasks){
    showTasks(tasks);    
    size_t taskID;
    cout << "\nEnter task ID to delete:\n";
    cin >> taskID;
    cin.ignore();
    tasks.delete_task(taskID);
}

void currentDayProgress(TaskManager& tasks, Database& database){

}

int main() {
    TaskManager taskList;
    Database database;

    char choice;
    do {
        std::cout << "\nMenu:\n1. Create Task\n2. Show Tasks\n3. Start Task\n4. Delete task\n5. Exit\nEnter your choice: ";
        std::cin >> choice;
        std::cin.ignore(); // Ignore leftover newline

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
                deleteTask(taskList);
                break;
            case '5':
                std::cout << "Exiting...\n";
                break;
            default:
                std::cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != '5');

    return 0;
}