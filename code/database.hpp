#pragma once

#include <string>
#include <vector>
#include <variant>

class Database{
    public:

    /*
    This function initializes the SQLite database and creates the necessary tables if they don't already exist.
    Steps:
        * Extracts the directory path from DATABASE_PATH and ensures that the directory exists. If it doesn't, it creates it.
        * Checks if the database file already exists by opening it as an ifstream.
        * Opens the database using SQLite (sqlite3_open).
        * If the database doesn't exist, it creates the tables for storing Tasks and Sessions, and defines the required triggers for maintaining parent task completion when tasks are added, deleted, or updated.
        * Closes the database connection after ensuring tables and triggers are properly created or updated.
    Exception Handling: Throws a std::runtime_error if any error occurs during the database creation or table creation process.
    */
    static void initialize_database();

    /*
    Executes a raw SQL query on the database.
    Parameters:
        query: A string representing the SQL query to be executed.
    Steps:
        * Opens the database using sqlite3_open.
        * Executes the query using sqlite3_exec and checks for errors.
        * Closes the database connection after the query execution is completed.
    Exception Handling: Throws a std::runtime_error if the database cannot be opened or if there is an error executing the query.
    */
    static void execute_query(const std::string& query);

    /*
    Executes a SQL query and fetches the results into a 2D vector of strings.
    Parameters:
        query: A string representing the SQL query to fetch results from the database.
    Returns:
        A 2D vector of strings, where each inner vector represents a row in the result set.
    Steps:
        * Opens the database and prepares the SQL query using sqlite3_prepare_v2.
        * Iterates over the rows returned by the query and collects column data into a vector of strings.
        * Closes the database connection and finalizes the statement.
    Exception Handling: Throws a std::runtime_error if the database cannot be opened or if there is an error during the query preparation or execution.
    */
    static std::vector<std::vector<std::string>> fetch_results(const std::string& query);

    /*
    Inserts data into the database using a prepared statement.
    Parameters:
        * query: A string representing the SQL query to insert data.
        * data: A vector containing the values to be bound to the prepared statement. The values can be of type std::string, int, or nullptr_t (for NULL values).
    Returns:
        0 if the insertion is successful, -1 if there is an error.
    Steps:
        * Opens the database and prepares the SQL query using sqlite3_prepare_v2.
        * Binds the parameters from the data vector to the query.
        * Executes the query using sqlite3_step and checks for completion.
        * Finalizes the statement and closes the database connection.
    Exception Handling: If any error occurs while preparing or executing the statement, an error message is printed, and -1 is returned.
    */
    static int insert_data(const std::string& query, const std::vector<std::variant<std::string, int, std::nullptr_t>>& data);

    /*
    Deletes an entry from a specified table based on the given ID.
    Parameters:
        * ID: The ID of the entry to delete.
        * table: The name of the table from which the entry should be deleted.
    Steps:
        * Opens the database and enables foreign key constraints using PRAGMA foreign_keys = ON.
        * Begins a transaction using BEGIN TRANSACTION.
        * Constructs a dynamic DELETE query based on the table name and binds the ID to the query.
        * Executes the deletion query and checks for completion.
        * Commits the transaction using COMMIT, or rolls back the transaction in case of failure.
        * Closes the database connection.
    Exception Handling: Throws a std::runtime_error if any error occurs during the transaction or query execution.
    */
    static void delete_entry(int ID, const std::string& table);

    /*
    Executes a prepared SQL query with bound parameters.
    Parameters:
        * query: A string representing the SQL query to execute.
        * params: A vector containing parameters to bind to the prepared statement. The parameters can be of type std::string or int.
    Returns:
        0 if the query executes successfully, -1 if there is an error.
    Steps:
        * Opens the database and prepares the query using sqlite3_prepare_v2.
        * Binds the parameters from the params vector to the prepared statement.
        * Executes the query using sqlite3_step and checks for completion.
        * Finalizes the statement and closes the database connection.
    Exception Handling: If any error occurs while preparing or executing the statement, an error message is printed, and -1 is returned.
    */
    static int execute_prepared_query(const std::string& query, const std::vector<std::variant<std::string, int>>& params);
};