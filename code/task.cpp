#include "task.hpp"

#define Task TaskManager::Task

Task& TaskManager::get_task_data(int taskNumber) {
    if (taskNumber < 1 || taskNumber > taskList.size()) {
        throw std::out_of_range("Invalid task number.");
    }
    return taskList[taskNumber - 1];
}

void TaskManager::change_task_data(int taskNumber, string& taskName, string& taskDescription, int timeGoal, int timeCompleted) {
    if (taskNumber < 1 || taskNumber > taskList.size()) {
        throw std::out_of_range("Invalid task number.");
    }
    Task& task = taskList[taskNumber - 1];
    task.name = taskName;
    task.description = taskDescription;
    if (timeGoal > 0) {
        task.timeGoalMinutes = timeGoal;
    }
    task.timeCompletedMinutes = timeCompleted;
    save_to_json(); // Save tasks to file after changing a task
}

void TaskManager::delete_task(int taskNumber) {
    if (taskNumber < 1 || taskNumber > taskList.size()) {
        throw std::out_of_range("Invalid task number.");
    }
    taskList.erase(taskList.begin() + taskNumber - 1);
    save_to_json(); // Save tasks to file after deleting a task
}

void TaskManager::save_to_json() {
    std::ofstream file(TASKS_FILE_NAME);
    if (file.is_open()) {
        json j;
        for (const auto& task : taskList) {
            j.push_back({
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
            taskList.emplace_back(
                taskData.at("name").get<string>(),
                taskData.at("description").get<string>(),
                taskData.at("timeGoalMinutes").get<int>(),
                taskData.at("timeCompletedMinutes").get<int>()
            );
        }
        file.close();
        std::cout << "Tasks loaded from " << TASKS_FILE_NAME << " successfully.\n";
    } else {
        std::cerr << "Failed to open file for reading: " << TASKS_FILE_NAME << "\n";
    }
}