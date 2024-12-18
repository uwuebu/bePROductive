#ifndef TASK_HPP
#define TASK_HPP

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <nlohmann/json.hpp>

#define TASKS_FILE_NAME "tasks.json"
#define DATES_FILE_NAME "dates.json"

using std::string, std::vector, std::pair;
using json = nlohmann::json;

string get_current_date();

string convert_time_to_string(const std::chrono::steady_clock::time_point& tp);

class TaskManager{
    struct Task {
        int taskID;
        string name;
        string description;
        int timeGoalMinutes;
        int timeCompletedMinutes = 0;

       Task(int id, const string& n, const string& d, int goal, int completed=0)
        : taskID(id), name(n), description(d), timeGoalMinutes(goal), timeCompletedMinutes(completed) {}
    };

    vector<Task> taskList;

    void save_to_json();
    void load_from_json();

    public:
    TaskManager(){load_from_json();};

    void add_task(string& taskName, string& taskDescription, int timeGoal);
    Task& get_task_data(int taskID);
    std::vector<Task> get_all_tasks() const {
        return taskList;
    }
    void change_task_data(int taskID, string& taskName, string& taskDescription, int timeGoal, int timeCompleted);
    void delete_task(int taskID);
    bool empty(){return taskList.empty();};
    size_t size(){return taskList.size();};
};

class Database {
    json database;

    void save_to_json();
    void load_from_json();

public:
    Database() {
        load_from_json();
    }
    void add_entry(int taskID, int timeSpent, const string& startTime, const string& endTime, int pauseTime);
    void delete_entry(const string& date);
    json get_current_day_data();
};

#endif // TASK_HPP