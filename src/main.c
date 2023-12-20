#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/todoroutes.h" // Include the todoroutes.h header file

// Function to read HTML content from a file
char *readHTMLFile(const char *filename)
{
  FILE *file = fopen(filename, "r");
  if (file == NULL)
  {
    fprintf(stderr, "Error opening file %s\n", filename);
    exit(EXIT_FAILURE);
  }

  fseek(file, 0, SEEK_END);
  long length = ftell(file);
  fseek(file, 0, SEEK_SET);

  char *buffer = (char *)malloc(length + 1);
  if (buffer == NULL)
  {
    fprintf(stderr, "Memory allocation error\n");
    fclose(file);
    exit(EXIT_FAILURE);
  }

  fread(buffer, 1, length, file);
  fclose(file);
  buffer[length] = '\0';

  return buffer;
}

static enum MHD_Result
handleRequest(void *cls,
              struct MHD_Connection *connection,
              const char *url,
              const char *method,
              const char *version,
              const char *upload_data,
              size_t *upload_data_size,
              void **ptr)
{
  // Handling the root route
  if (strcmp(url, "/") == 0)
  {
    char *HTML_RESPONSE = readHTMLFile("index.html");

    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(HTML_RESPONSE),
                                                                    (void *)HTML_RESPONSE,
                                                                    MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(response, "Content-Type", "text/html");
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
  }

  if (strcmp(url, "/api-docs") == 0)
  {
    // Serve Swagger JSON file
    char *swaggerJSON = readHTMLFile("swagger.json");

    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(swaggerJSON),
                                                                    (void *)swaggerJSON,
                                                                    MHD_RESPMEM_MUST_FREE); // Free memory after use
    MHD_add_response_header(response, "Content-Type", "application/json");
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
  }

  if (strcmp(url, "/swagger") == 0)
  {
    // Serve Swagger UI files here
    char *swaggerHTML = readHTMLFile("swaggerui/index.html");

    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(swaggerHTML),
                                                                    (void *)swaggerHTML,
                                                                    MHD_RESPMEM_MUST_FREE); // Free memory after use
    MHD_add_response_header(response, "Content-Type", "text/html");
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
  }

  // Handling the /api/todos route
  if (strcmp(url, "/api/todos") == 0)
  {
    return handleTodoRoutes(cls, connection, url, method, version, upload_data, upload_data_size, ptr);
  }

  // For other routes, return a 404
  struct MHD_Response *response = MHD_create_response_from_buffer(0, NULL, MHD_RESPMEM_PERSISTENT);
  int ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
  MHD_destroy_response(response);
  return ret;
}

int main(int argc, char **argv)
{
  struct MHD_Daemon *d;

  if (argc != 2)
  {
    printf("%s PORT\n", argv[0]);
    return 1;
  }

  initTodos(); // Initialize todos

  // Start the MicroHTTPD daemon, passing the handleRequest function for routing
  d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
                       atoi(argv[1]),
                       NULL,
                       NULL,
                       &handleRequest, // Use handleRequest for routing
                       NULL,           // No default response for unhandled routes
                       MHD_OPTION_END);

  if (d == NULL)
    return 1;

  (void)getc(stdin);
  MHD_stop_daemon(d);
  return 0;
}
