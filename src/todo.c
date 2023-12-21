#include "../include/todo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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