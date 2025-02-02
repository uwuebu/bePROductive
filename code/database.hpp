#pragma once

#include <string>
#include <vector>
#include <variant>

class Database{
    public:
    static void initialize_database();
    static void execute_query(const std::string& query);
    static std::vector<std::vector<std::string>> fetch_results(const std::string& query);
    static int insert_data(const std::string& query, const std::vector<std::variant<std::string, int, std::nullptr_t>>& data);
    static void delete_entry(int ID, const std::string& table);
    static int execute_prepared_query(const std::string& query, const std::vector<std::variant<std::string, int>>& params);
};