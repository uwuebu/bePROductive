#include "task.hpp"

#define Task TaskManager::Task

string get_current_date() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm* date = std::localtime(&t);

    std::ostringstream oss;
    oss << std::put_time(date, "%Y-%m-%d"); // Format as YYYY-MM-DD
    return oss.str();
}

string convert_time_to_string(const std::chrono::steady_clock::time_point& tp) {
    // Get the duration in seconds since the steady clock started
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(tp.time_since_epoch()).count();
    
    // Convert seconds into hours and minutes
    int hours = duration / 3600;
    int minutes = (duration % 3600) / 60;

    std::ostringstream oss;
    oss << std::setw(2) << std::setfill('0') << hours << ":" 
        << std::setw(2) << std::setfill('0') << minutes;

    return oss.str();
}

void TaskManager::add_task(string& taskName, string& taskDescription, int timeGoal) {
    int taskID = 0; // Default ID for the first task

    // Find the highest taskID in the current taskList
    if (!taskList.empty()) {
        taskID = taskList.back().taskID + 1; // Increment the highest existing taskID
    }

    taskList.emplace_back(taskID, taskName, taskDescription, timeGoal); // Add the new task
    save_to_json(); // Save the updated task list
}

Task& TaskManager::get_task_data(int taskID) {
    for (auto& task : taskList) {
        if (task.taskID == taskID) {
            return task; // Return the matching task
        }
    }
    throw std::out_of_range("Task with the given ID not found.");
}

void TaskManager::change_task_data(int taskID, string& taskName, string& taskDescription, int timeGoal, int timeCompleted) {
    for (auto& task : taskList) {
        if (task.taskID == taskID) {
            // Update task details
            task.name = taskName;
            task.description = taskDescription;
            if (timeGoal > 0) {
                task.timeGoalMinutes = timeGoal;
            }
            task.timeCompletedMinutes = timeCompleted;

            save_to_json(); // Save tasks to file after changing a task
            return;
        }
    }
    throw std::out_of_range("Task with the given ID not found.");
}


void TaskManager::delete_task(int taskID) {
    auto it = std::find_if(taskList.begin(), taskList.end(), [&](const Task& task) {
        return task.taskID == taskID;
    });

    if (it != taskList.end()) {
        taskList.erase(it); // Remove the task from the vector
        save_to_json();     // Save changes to the file
    } else {
        throw std::out_of_range("Task with the given ID not found.");
    }
}

void TaskManager::save_to_json() {
    std::ofstream file(TASKS_FILE_NAME);
    if (file.is_open()) {
        json j;
        for (const auto& task : taskList) {
            j.push_back({
                {"taskID", task.taskID},
                {"name", task.name},
                {"description", task.description},
                {"timeGoalMinutes", task.timeGoalMinutes},
                {"timeCompletedMinutes", task.timeCompletedMinutes}
            });
        }
        file << j.dump(4); // Pretty print with an indentation of 4 spaces
        file.close();
    } else {
        std::cerr << "Failed to open file for writing: " << TASKS_FILE_NAME << "\n";
    }
}

void TaskManager::load_from_json() {
    std::ifstream file(TASKS_FILE_NAME);
    if (file.is_open()) {
        json j;
        file >> j;
        taskList.clear(); // Clear current tasks
        for (const auto& taskData : j) {
            Task newTask(
                taskData.at("taskID").get<int>(),
                taskData.at("name").get<string>(),
                taskData.at("description").get<string>(),
                taskData.at("timeGoalMinutes").get<int>(),
                taskData.at("timeCompletedMinutes").get<int>()
            );
            taskList.push_back(newTask);
        }
        file.close();
        std::cout << "Tasks loaded from " << TASKS_FILE_NAME << " successfully.\n";
    } else {
        std::cerr << "Failed to open file for reading: " << TASKS_FILE_NAME << "\n";
    }
}

void Database::save_to_json() {
    std::ofstream file("calendar.json");
    if (file.is_open()) {
        file << database.dump(4); // Pretty print with 4 spaces
        file.close();
    } else {
        std::cerr << "Error: Unable to open file for saving.\n";
    }
}


void Database::load_from_json() {
    std::ifstream file("calendar.json");
    if (file.is_open()) {
        try {
            file >> database;
        } catch (const std::exception& e) {
            std::cerr << "Error loading JSON: " << e.what() << '\n';
            database = json::array(); // Reset to empty array if load fails
        }
        file.close();
    } else {
        database = json::array(); // Initialize as empty if no file exists
    }
}


void Database::add_entry(int taskID, int timeSpent, const string& startTime, const string& endTime, int pauseTime) {
    bool dateFound = false;

    auto date = get_current_date();

    for (auto& entry : database) {
        if (entry["date"] == date) { // Date exists
            dateFound = true;

            // Add a new session to the existing date
            entry["sessions"].push_back({
                {"taskID", taskID},
                {"timeSpent", timeSpent},
                {"startTime", startTime},
                {"endTime", endTime},
                {"pauseTime", pauseTime}
            });
            break;
        }
    }

    if (!dateFound) { // Date doesn't exist, create a new entry
        json newEntry;
        newEntry["date"] = date;
        newEntry["sessions"] = {{
            {"taskID", taskID},
            {"timeSpent", timeSpent},
            {"startTime", startTime},
            {"endTime", endTime},
            {"pauseTime", pauseTime}
        }};
        database.push_back(newEntry);
    }

    
    save_to_json();
}

void Database::delete_entry(const string& date) {
    for (auto it = database.begin(); it != database.end(); ++it) {
        if ((*it)["date"] == date) {
            database.erase(it);
            save_to_json();
            return;
        }
    }
    std::cerr << "Warning: Date not found. Nothing to delete.\n";
}

json Database::get_day_data(const string& date) {
    for (const auto& entry : database) {
        if (entry["date"] == date) {
            return entry; // Return the JSON entry for the current day
        }
    }
    return {}; // Return empty JSON object if no data for the current day
}