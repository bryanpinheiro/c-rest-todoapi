// todoroutes.c

#include "../include/todoroutes.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct Todo
{
  int id;
  char title[100];
  char description[200];
};

struct Todo todos[10]; // Array to store todos (limited to 10 for demonstration)

void initTodos()
{
  for (int i = 0; i < 10; ++i)
  {
    todos[i].id = i + 1;
    snprintf(todos[i].title, sizeof(todos[i].title), "Todo %d", i + 1);
    snprintf(todos[i].description, sizeof(todos[i].description), "Description %d", i + 1);
  }
}

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

  if (0 != strcmp(method, "GET"))
    return MHD_NO; /* unexpected method */

  if (strcmp(url, "/api/todos") == 0)
  {
    if (strcmp(method, "GET") == 0)
    {
      // Handle GET request for todos
      // Return todos array as JSON
      // You can use a JSON library for a proper JSON conversion
      // For demonstration, converting manually
      char jsonResponse[1024];
      snprintf(jsonResponse, sizeof(jsonResponse), "[{\"id\": %d, \"title\": \"%s\", \"description\": \"%s\"}]", todos[0].id, todos[0].title, todos[0].description);

      response = MHD_create_response_from_buffer(strlen(jsonResponse),
                                                 (void *)jsonResponse,
                                                 MHD_RESPMEM_PERSISTENT);
      MHD_add_response_header(response, "Content-Type", "application/json");
      ret = MHD_queue_response(connection,
                               MHD_HTTP_OK,
                               response);
      MHD_destroy_response(response);
      return ret;
    }
    else if (strcmp(method, "POST") == 0)
    {
      // Handle POST request to create a new todo
      // This is where you'd parse the incoming data and create a new todo
      // For demonstration, let's just acknowledge the request
      const char *postResponse = "{\"message\": \"Todo created.\"}";
      response = MHD_create_response_from_buffer(strlen(postResponse),
                                                 (void *)postResponse,
                                                 MHD_RESPMEM_PERSISTENT);
      MHD_add_response_header(response, "Content-Type", "application/json");
      ret = MHD_queue_response(connection,
                               MHD_HTTP_CREATED,
                               response);
      MHD_destroy_response(response);
      return ret;
    }
    else if (strcmp(method, "PUT") == 0)
    {
      // Handle PUT request to update a todo
      // This is where you'd update a todo based on the incoming data
      // For demonstration, let's acknowledge the request
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
      // Handle DELETE request to delete a todo
      // This is where you'd delete a todo based on the incoming data
      // For demonstration, let's acknowledge the request
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
    }
  }

  /* For other routes, return a 404 */
  response = MHD_create_response_from_buffer(0, NULL, MHD_RESPMEM_PERSISTENT);
  ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
  MHD_destroy_response(response);
  return ret;
}
