#ifndef TODO_H
#define TODO_H

#include <sqlite3.h>

typedef struct
{
  int id;
  const unsigned char *title;
  int completed;
} TodoItem;

typedef struct
{
  TodoItem *todoList;
  int numTodos;
} TodoListWithCount;

char *readSQLFromFile(const char *filename);
int executeSQLFromFile(sqlite3 *db, const char *filename);
int insertTodoItem(sqlite3 *db, const char *title, int completed);
int updateTodoCompleted(sqlite3 *db, int todoID, int completed);
int deleteTodoByID(sqlite3 *db, int todoID);
TodoListWithCount getAllTodoItemsWithCount(sqlite3 *db, const char *filename);
void displayTodos(TodoItem *todoList, int length);

#endif /* TODO_H */
