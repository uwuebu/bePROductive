#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread> // For sleep
#include <iomanip> // For formatting output
#include <nlohmann/json.hpp>

struct Task {
    std::string name;
    std::string description;
    int timeGoalMinutes;
    int timeCompletedMinutes = 0;

    Task(const std::string& taskName, const std::string& taskDescription, int timeGoal)
        : name(taskName), description(taskDescription), timeGoalMinutes(timeGoal) {}
};

void createTask(std::vector<Task>& tasks) {
    std::string name, description;
    int timeGoal;

    std::cout << "Enter task name: ";
    std::getline(std::cin, name);

    std::cout << "Enter task description: ";
    std::getline(std::cin, description);

    std::cout << "Enter time goal (in minutes): ";
    std::cin >> timeGoal;

    tasks.emplace_back(name, description, timeGoal);

    std::cout << "Task created successfully!\n";
}

void showTasks(const std::vector<Task>& tasks) {
    if (tasks.empty()) {
        std::cout << "No tasks available.\n";
        return;
    }

    for (size_t i = 0; i < tasks.size(); ++i) {
        std::cout << i + 1 << ". Name: " << tasks[i].name << "\n"
                  << "   Description: " << tasks[i].description << "\n"
                  << "   Time Goal: " << tasks[i].timeGoalMinutes << " minutes\n"
                  << "   Time Completed: " << tasks[i].timeCompletedMinutes << " minutes\n\n";
    }
}

void startTask(std::vector<Task>& tasks) {
    if (tasks.empty()) {
        std::cout << "No tasks available to start.\n";
        return;
    }

    showTasks(tasks);

    size_t taskIndex;
    std::cout << "Enter the task number to start: ";
    std::cin >> taskIndex;
    std::cin.ignore();

    if (taskIndex < 1 || taskIndex > tasks.size()) {
        std::cout << "Invalid task number.\n";
        return;
    }

    Task& task = tasks[taskIndex - 1];

    std::cout << "Starting task: " << task.name << "\n"
              << "Controls: [P] Pause, [C] Continue, [X] Stop\n";

    auto startTime = std::chrono::steady_clock::now();
    bool running = true;
    bool paused = false;
    std::chrono::steady_clock::time_point pauseStartTime;
    int pausedSeconds = 0;

    // Start a thread to handle timer display
    std::thread timerThread([&]() {
        while (running) {
            if (!paused) {
                auto currentTime = std::chrono::steady_clock::now();
                int elapsedSeconds = std::chrono::duration_cast<std::chrono::seconds>(currentTime - startTime).count() - pausedSeconds;
                int minutes = elapsedSeconds / 60;
                int seconds = elapsedSeconds % 60;

                // Move cursor up and overwrite the timer line
                std::cout << "\rElapsed time: " << std::setw(2) << std::setfill('0') << minutes << ":"
                          << std::setw(2) << std::setfill('0') << seconds << "   " << std::flush;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Shorter interval for responsiveness
        }
    });

    while (running) {
        char command;
        std::cin >> command;
        std::cin.ignore();

        switch (command) {
            case 'P':
            case 'p': // Pause
                if (!paused) {
                    paused = true;
                    pauseStartTime = std::chrono::steady_clock::now();
                    std::cout << "\nTimer paused. Press [C] to continue or [X] to stop.\n";
                }
                break;

            case 'C':
            case 'c': // Continue
                if (paused) {
                    paused = false;
                    auto pauseEndTime = std::chrono::steady_clock::now();
                    pausedSeconds += std::chrono::duration_cast<std::chrono::seconds>(pauseEndTime - pauseStartTime).count();
                    std::cout << "\nTimer continued. Press [P] to pause or [X] to stop.\n";
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

    std::cout << "\nTask stopped. Time added: " << elapsedMinutes << " minutes.\n";
}


int main() {
    std::vector<Task> taskList;

    char choice;
    do {
        std::cout << "\nMenu:\n1. Create Task\n2. Show Tasks\n3. Start Task\n4. Exit\nEnter your choice: ";
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
                startTask(taskList);
                break;
            case '4':
                std::cout << "Exiting...\n";
                break;
            default:
                std::cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != '4');

    return 0;
}
