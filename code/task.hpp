#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <nlohmann/json.hpp>

#define TASKS_FILE_NAME "tasks.json"

using std::string, std::vector;
using json = nlohmann::json;

class TaskManager{
    struct Task {
    std::string name;
    std::string description;
    int timeGoalMinutes;
    int timeCompletedMinutes = 0;

    Task(const string& taskName, const string& taskDescription, int timeGoal, int timeCompleted = 0)
        : name(taskName), description(taskDescription), timeGoalMinutes(timeGoal), timeCompletedMinutes(timeCompleted) {}
    };

    vector<Task> taskList;

    void save_to_json();
    void load_from_json();

    public:
    TaskManager(){load_from_json();};

    void add_task(string& taskName, string& taskDescription, int timeGoal){taskList.emplace_back(taskName, taskDescription, timeGoal); save_to_json();};
    Task& get_task_data(int taskNumber);
    void change_task_data(int taskNumber, string& taskName, string& taskDescription, int timeGoal, int timeCompleted);
    void delete_task(int taskNumber);
    bool empty(){return taskList.empty();};
    size_t size(){return taskList.size();};
};