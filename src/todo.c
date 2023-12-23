#include "../include/todo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

sqlite3 *initializeDatabase() {
  sqlite3 *db;
  int rc = sqlite3_open("todos.db", &db);

  if (rc != SQLITE_OK) {
      fprintf(stderr, "Cannot open database: %s\n", sqlite3_errmsg(db));
      sqlite3_close(db);
      return NULL;
  }

  // Check if the 'todos' table exists
  int table_exists = 0;
  sqlite3_stmt *stmt;
  const char *query = "SELECT name FROM sqlite_master WHERE type='table' AND name='todos';";
  if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) == SQLITE_OK) {
      if (sqlite3_step(stmt) == SQLITE_ROW) {
          table_exists = 1;
      }
      sqlite3_finalize(stmt);
  }

  if (!table_exists) {
      // Table does not exist, create it
      if (executeSQLFromFile(db, "sql/create_todos.sql") != 0) {
          sqlite3_close(db);
          return NULL;
      }
      printf("Todos table created successfully!\n");
  }

  return db;
}

char *readSQLFromFile(const char *filename)
{
  FILE *file = fopen(filename, "r");
  if (!file)
  {
    fprintf(stderr, "Error opening file: %s\n", filename);
    return NULL;
  }

  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *sql = malloc(file_size + 1);
  fread(sql, 1, file_size, file);
  sql[file_size] = '\0';

  fclose(file);
  return sql;
}

int executeSQLFromFile(sqlite3 *db, const char *filename)
{
  char *sql = readSQLFromFile(filename);
  if (!sql)
  {
    return 1;
  }

  int rc = sqlite3_exec(db, sql, NULL, 0, NULL);
  free(sql);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
    return 1;
  }

  return 0;
}

int insertTodoItem(sqlite3 *db, const char *title, int completed)
{
  int completedInt = (completed != 0) ? 1 : 0;

  // Read the SQL query from file
  char *sql = readSQLFromFile("sql/insert_todo.sql");
  if (!sql)
  {
    return 1;
  }

  // Replace placeholders in the SQL query with provided values
  char query[1000]; // Assuming the query won't exceed 1000 characters
  snprintf(query, sizeof(query), sql, title, completedInt);
  free(sql);

  // Execute the modified SQL query
  int rc = sqlite3_exec(db, query, NULL, 0, NULL);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
    return 1;
  }

  printf("Todo item inserted successfully!\n");
  return 0;
}

TodoItem *getTodoByID(sqlite3 *db, int todoID) {
    TodoItem *todo = NULL; // Initialize with zeros

    // Read the SQL query from file
    char *sql = readSQLFromFile("sql/get_todo_by_id.sql");
    if (!sql) {
        return todo; // Return an empty TodoItem on SQL query failure
    }

    // Replace placeholders in the SQL query with the provided todoID
    size_t query_len = strlen(sql) + 20; // Initial estimation of query length
    char *query = malloc(query_len); // Allocate memory for the query
    if (!query) {
        free(sql);
        return todo; // Return an empty TodoItem on memory allocation failure
    }

    snprintf(query, query_len, sql, todoID);
    free(sql); // Free the original SQL query

    // Prepare the SQL statement
    sqlite3_stmt *stmt;
    int rc = sqlite3_prepare_v2(db, query, -1, &stmt, NULL);
    free(query); // Free the query string
    if (rc != SQLITE_OK) {
        // Handle error in preparing the statement
        return todo; // Return an empty TodoItem
    }

    // Execute the SQL statement
    rc = sqlite3_step(stmt);
    if (rc == SQLITE_ROW) {
        // Fetch the values from the query result
        todo = malloc(sizeof(TodoItem));
        if (todo) {
          todo->id = sqlite3_column_int(stmt, 0);
          const unsigned char *title = sqlite3_column_text(stmt, 1);
          if (title) {
              size_t length = strlen((const char *)title);
              todo->title = malloc(length + 1); // Allocate memory for title
              if (todo->title) {
                  strcpy((char *)todo->title, (const char *)title);
              }
          }
          todo->completed = sqlite3_column_int(stmt, 2);
        }
    }

    // Finalize the SQL statement
    sqlite3_finalize(stmt);

    return todo;
}

int updateTodoCompleted(sqlite3 *db, int todoID, int completed)
{
  int completedInt = (completed != 0) ? 1 : 0;

  // Read the SQL query from file
  char *sql = readSQLFromFile("sql/update_todo_by_id.sql");
  if (!sql)
  {
    return 1;
  }

  // Replace placeholders in the SQL query with provided values
  char query[1000]; // Assuming the query won't exceed 1000 characters
  snprintf(query, sizeof(query), sql, completedInt, todoID);
  free(sql);

  // Execute the modified SQL query
  int rc = sqlite3_exec(db, query, NULL, 0, NULL);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
    return 1;
  }

  printf("Todo item updated successfully!\n");
  return 0;
}

int deleteTodoByID(sqlite3 *db, int todoID)
{
  // Read the SQL query from file
  char *sql = readSQLFromFile("sql/delete_todo_by_id.sql");
  if (!sql)
  {
    return 1;
  }

  // Replace placeholders in the SQL query with the provided todoID
  char query[1000]; // Assuming the query won't exceed 1000 characters
  snprintf(query, sizeof(query), sql, todoID);
  free(sql);

  // Execute the modified SQL query
  int rc = sqlite3_exec(db, query, NULL, 0, NULL);

  if (rc != SQLITE_OK)
  {
    fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
    return 1;
  }

  printf("Todo item deleted successfully!\n");
  return 0;
}

TodoListWithCount getAllTodoItemsWithCount(sqlite3 *db, const char *filename)
{
  TodoListWithCount result = {NULL, 0};

  TodoItem *todoList = NULL;
  int numTodos = 0;

  // Read the SELECT query from file
  char *query = readSQLFromFile(filename);
  if (!query)
  {
    return result;
  }

  // Prepare the SELECT statement
  sqlite3_stmt *stmt;
  if (sqlite3_prepare_v2(db, query, -1, &stmt, NULL) != SQLITE_OK)
  {
    fprintf(stderr, "SQL error: %s\n", sqlite3_errmsg(db));
    return result;
  }

  // Fetch all todo items and store in the list
  while (sqlite3_step(stmt) == SQLITE_ROW)
  {
    numTodos++;
    todoList = realloc(todoList, numTodos * sizeof(TodoItem));

    todoList[numTodos - 1].id = sqlite3_column_int(stmt, 0);

    const unsigned char *columnText = sqlite3_column_text(stmt, 1);
    size_t length = strlen((const char *)columnText);
    todoList[numTodos - 1].title = (unsigned char *)malloc(length + 1);
    strcpy((char *)todoList[numTodos - 1].title, (const char *)columnText);

    todoList[numTodos - 1].completed = sqlite3_column_int(stmt, 2);
  }

  sqlite3_finalize(stmt);

  result.numTodos = numTodos;
  result.todoList = todoList;

  return result;
}

void displayTodos(TodoItem *todoList, int length)
{
  printf("Todo items:\n");
  for (int i = 0; i < length; i++)
  {
    printf("ID: %d, Title: %s, Completed: %s\n",
           todoList[i].id,
           (char *)todoList[i].title,
           (todoList[i].completed ? "true" : "false"));
  }
}