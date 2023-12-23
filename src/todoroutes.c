// todoroutes.c

#include "../include/todoroutes.h"
#include "../include/todo.h"
#include <jansson.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

int extractIDFromURLPath(const char *urlPath) {
    // Find the last '/' in the URL
    const char *lastSlash = strrchr(urlPath, '/');
    if (lastSlash == NULL) {
        // No '/' found in the URL, meaning the ID is missing or the URL is invalid
        return -1;
    }

    // Get the segment after the last '/'
    const char *idSegment = lastSlash + 1;

    // Attempt to convert the segment to an integer ID
    int extractedID;
    int numScanned = sscanf(idSegment, "%d", &extractedID);
    if (numScanned != 1) {
        // Failed to extract a valid ID
        return -1;
    }

    // Return the extracted ID
    return extractedID;
}

typedef struct {
    int completed;
} InputUpdateTodo;

enum MHD_Result handleTodoRoutes(void *cls,
                                 struct MHD_Connection *connection,
                                 const char *url,
                                 const char *method,
                                 const char *version,
                                 const char *upload_data,
                                 size_t *upload_data_size,
                                 void **ptr)
{
  static int dummy;
  struct MHD_Response *response;
  int ret;
  sqlite3 *db = initializeDatabase();
  printf("URL: %s\n", url); 

  if (strcmp(url, "/api/todos") == 0)
  {
    if (strcmp(method, "GET") == 0)
    {
      printf("Get todos!\n");
      // Call SQLite function to retrieve todos
      TodoListWithCount todosWithCount = getAllTodoItemsWithCount(db, "sql/get_all_todos.sql");
      TodoItem *todoList = todosWithCount.todoList;

      // Format retrieved todos into JSON response
      json_t *root = json_array();

      // Loop through the retrieved TODO items and add them to the JSON array
      for (int i = 0; i < todosWithCount.numTodos; ++i) {
          json_t *todo = json_object(); // Create a JSON object for each TODO item

          json_object_set_new(todo, "id", json_integer(todoList[i].id));
          json_object_set_new(todo, "title", json_string((const char *)todoList[i].title));
          json_object_set_new(todo, "completed", json_boolean(todoList[i].completed));

          json_array_append_new(root, todo); // Append each TODO object to the array
      }

      // Convert JSON array to string
      char *jsonResponse = json_dumps(root, JSON_INDENT(2));

      // Cleanup JSON resources
      json_decref(root);

      // Create and return MHD response
      response = MHD_create_response_from_buffer(strlen(jsonResponse),
                                                  (void *)jsonResponse,
                                                  MHD_RESPMEM_MUST_FREE); // Set MHD_RESPMEM_MUST_FREE to free memory
      MHD_add_response_header(response, "Content-Type", "application/json");
      ret = MHD_queue_response(connection,
                              MHD_HTTP_OK,
                              response);
      MHD_destroy_response(response);
      return ret;
    }
    else if (strcmp(method, "POST") == 0)
    {
      // Log received data and its size
      printf("Received data: %.*s\n", (int)(*upload_data_size), upload_data);
      printf("Data size: %zu bytes\n", *upload_data_size);
      if (*upload_data_size == 0) {
        printf("Processing POST data...\n");
        // Data is received in two chunks
        return MHD_YES;
      } else {
        // Parse the incoming JSON data to extract the new todo's title
        printf("Parsing JSON data...\n");
        json_error_t error;
        json_t *root = json_loadb(upload_data, *upload_data_size, 0, &error);
        
        if (!root)
        {
            // Error parsing JSON data
            printf("Failed to parse JSON data.\n");
            const char *errorResponse = "{\"error\": \"Failed to parse JSON data.\"}";
            response = MHD_create_response_from_buffer(strlen(errorResponse),
                                                      (void *)errorResponse,
                                                      MHD_RESPMEM_PERSISTENT);
            MHD_add_response_header(response, "Content-Type", "application/json");
            ret = MHD_queue_response(connection,
                                    MHD_HTTP_BAD_REQUEST,
                                    response);
            MHD_destroy_response(response);
            return ret;
        }

        const char *title = json_string_value(json_object_get(root, "title"));

        if (!title)
        {
            // Title not found in the JSON or not a string
            printf("Title not provided or invalid.\n");
            const char *errorResponse = "{\"error\": \"Title not provided or invalid.\"}";
            response = MHD_create_response_from_buffer(strlen(errorResponse),
                                                      (void *)errorResponse,
                                                      MHD_RESPMEM_PERSISTENT);
            MHD_add_response_header(response, "Content-Type", "application/json");
            ret = MHD_queue_response(connection,
                                    MHD_HTTP_BAD_REQUEST,
                                    response);
            MHD_destroy_response(response);
            json_decref(root);
            return ret;
        }

        // Create a new todo with the extracted title
        printf("Creating new todo item...\n");
        int result = insertTodoItem(db, title, 0);

        if (result != 0)
        {
            // Error creating todo
            printf("Failed to create todo.\n");
            const char *errorResponse = "{\"error\": \"Failed to create todo.\"}";
            response = MHD_create_response_from_buffer(strlen(errorResponse),
                                                      (void *)errorResponse,
                                                      MHD_RESPMEM_PERSISTENT);
            MHD_add_response_header(response, "Content-Type", "application/json");
            ret = MHD_queue_response(connection,
                                    MHD_HTTP_INTERNAL_SERVER_ERROR,
                                    response);
            MHD_destroy_response(response);
            json_decref(root);
            return ret;
        }

        // (Todo) Created successfully
        printf("Todo created successfully.\n");
        json_t *newroot = json_object();
        json_object_set_new(newroot, "message", json_string("Todo created"));

        const char *postResponse = json_dumps(newroot, JSON_COMPACT);
        json_decref(newroot);

        if (postResponse != NULL) {
            printf("Response created successfully.\n");
            struct MHD_Response *response = MHD_create_response_from_buffer(strlen(postResponse),
                                                                            (void *)postResponse,
                                                                            MHD_RESPMEM_MUST_FREE);
            MHD_add_response_header(response, "Content-Type", "application/json");
            ret = MHD_queue_response(connection, MHD_HTTP_CREATED, response);
            MHD_destroy_response(response);
            return ret;
        } else {
            response = MHD_create_response_from_buffer(0, NULL, MHD_RESPMEM_PERSISTENT);
            ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
            MHD_destroy_response(response);
            return ret;
        }
      }
    } else {
      return MHD_NO;
    }
  } else if (strstr(url, "/api/todos/") != NULL) {
    int todoID = extractIDFromURLPath(url);

    // Handling operations for specific todo IDs, e.g., "/api/todos/{id}"
    // Extract the ID from the URL here and perform specific operations
    if (strcmp(method, "GET") == 0) {
      // Retrieve the todo item by ID
        TodoItem *todo = getTodoByID(db, todoID);

        if (todo == NULL) {
          printf("Todo item not found...\n");
          // If todo item with given ID is not found, return a 404 response
          const char *notFoundResponse = "{\"error\": \"Todo item not found.\"}";
          response = MHD_create_response_from_buffer(strlen(notFoundResponse),
                                                      (void *)notFoundResponse,
                                                      MHD_RESPMEM_PERSISTENT);
          MHD_add_response_header(response, "Content-Type", "application/json");
          ret = MHD_queue_response(connection,
                                    MHD_HTTP_NOT_FOUND,
                                    response);
          MHD_destroy_response(response);
          return ret;
        }

        // If the todo item with the given ID is found, return it as a response
        json_t *todoJson = json_object();
        json_object_set_new(todoJson, "id", json_integer(todo->id));
        json_object_set_new(todoJson, "title", json_string((const char *)todo->title));
        json_object_set_new(todoJson, "completed", json_boolean(todo->completed));

        char *jsonResponse = json_dumps(todoJson, JSON_INDENT(2));
        json_decref(todoJson);

        printf("Todo item found...\n");
        response = MHD_create_response_from_buffer(strlen(jsonResponse),
                                                    (void *)jsonResponse,
                                                    MHD_RESPMEM_MUST_FREE); // Set MHD_RESPMEM_MUST_FREE to free memory
        MHD_add_response_header(response, "Content-Type", "application/json");
        ret = MHD_queue_response(connection,
                                MHD_HTTP_OK,
                                response);
        MHD_destroy_response(response);
        return ret;
    }
    else if (strcmp(method, "PUT") == 0)
    {
        // Parse the incoming JSON data to extract the new todo's title
        json_error_t error;
        json_t *root = json_loadb(upload_data, *upload_data_size, 0, &error);
        
        if (!root)
        {
            // Error parsing JSON data
            const char *errorResponse = "{\"error\": \"Failed to parse JSON data.\"}";
            response = MHD_create_response_from_buffer(strlen(errorResponse),
                                                      (void *)errorResponse,
                                                      MHD_RESPMEM_PERSISTENT);
            MHD_add_response_header(response, "Content-Type", "application/json");
            ret = MHD_queue_response(connection,
                                    MHD_HTTP_BAD_REQUEST,
                                    response);
            MHD_destroy_response(response);
            return ret;
        }

        // Extract 'completed' property from JSON
        InputUpdateTodo updateTodo;
        json_t *completedVal = json_object_get(root, "completed");

        if (!json_is_boolean(completedVal))
        {
            // 'completed' property not found or invalid
            const char *errorResponse = "{\"error\": \"Completed status not provided or invalid.\"}";
            response = MHD_create_response_from_buffer(strlen(errorResponse),
                                                      (void *)errorResponse,
                                                      MHD_RESPMEM_PERSISTENT);
            MHD_add_response_header(response, "Content-Type", "application/json");
            ret = MHD_queue_response(connection,
                                    MHD_HTTP_BAD_REQUEST,
                                    response);
            MHD_destroy_response(response);
            json_decref(root);
            return ret;
        }

        updateTodo.completed = json_is_true(completedVal) ? 1 : 0;
        json_decref(root); // Release JSON resources

        // Update the 'completed' status for the todo
        int result = updateTodoCompleted(db, todoID, updateTodo.completed); // Replace with your update function

        if (result != 0)
        {
            // Error updating todo
            const char *errorResponse = "{\"error\": \"Failed to update todo.\"}";
            response = MHD_create_response_from_buffer(strlen(errorResponse),
                                                      (void *)errorResponse,
                                                      MHD_RESPMEM_PERSISTENT);
            MHD_add_response_header(response, "Content-Type", "application/json");
            ret = MHD_queue_response(connection,
                                    MHD_HTTP_INTERNAL_SERVER_ERROR,
                                    response);
            MHD_destroy_response(response);
            return ret;
        }

        // (Todo) updated successfully
        const char *putResponse = "{\"message\": \"Todo updated.\"}";
        response = MHD_create_response_from_buffer(strlen(putResponse),
                                                  (void *)putResponse,
                                                  MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "application/json");
        ret = MHD_queue_response(connection,
                                MHD_HTTP_OK,
                                response);
        MHD_destroy_response(response);
        return ret;
    }
    else if (strcmp(method, "DELETE") == 0)
    {
        // Delete the todo item by ID
        int result = deleteTodoByID(db, todoID);

        if (result != 0) {
            // If deletion fails, return an error response
            const char *errorResponse = "{\"error\": \"Failed to delete todo.\"}";
            response = MHD_create_response_from_buffer(strlen(errorResponse),
                                                        (void *)errorResponse,
                                                        MHD_RESPMEM_PERSISTENT);
            MHD_add_response_header(response, "Content-Type", "application/json");
            ret = MHD_queue_response(connection,
                                    MHD_HTTP_INTERNAL_SERVER_ERROR,
                                    response);
            MHD_destroy_response(response);
            return ret;
        }

        // (Todo) item deleted successfully
        const char *deleteResponse = "{\"message\": \"Todo deleted.\"}";
        response = MHD_create_response_from_buffer(strlen(deleteResponse),
                                                    (void *)deleteResponse,
                                                    MHD_RESPMEM_PERSISTENT);
        MHD_add_response_header(response, "Content-Type", "application/json");
        ret = MHD_queue_response(connection,
                                MHD_HTTP_OK,
                                response);
        MHD_destroy_response(response);
        return ret;
    } else {
      printf("URL: %s\n", url); 
      return MHD_NO;
    }
  } else {
    /* For other routes, return a 404 */
    response = MHD_create_response_from_buffer(0, NULL, MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
    MHD_destroy_response(response);
    return ret;
  }
}
