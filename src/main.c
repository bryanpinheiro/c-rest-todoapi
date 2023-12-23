#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "../include/todoroutes.h" // Include the todoroutes.h header file

#define TEXT_PLAIN "text/plain; charset=utf-8"
#define TEXT_HTML "text/html; charset=utf-8"
#define TEXT_CSS "text/css; charset=utf-8"
#define APP_JS "application/javascript; charset=utf-8"
#define IMAGE_PNG "image/png"
#define APPLICATION_MAP "application/map"

static int send_response(struct MHD_Connection *connection, const char *content, int status_code, const char *content_type)
{
  struct MHD_Response *response = MHD_create_response_from_buffer(strlen(content), (void *)content, MHD_RESPMEM_MUST_COPY);
  if (!response)
  {
    return MHD_NO;
  }
  
  MHD_add_response_header(response, "Content-Type", content_type); // Set Content-Type header

  int ret = MHD_queue_response(connection, status_code, response);
  MHD_destroy_response(response);
  return ret;
}

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

static enum MHD_Result serve_js_file(struct MHD_Connection *connection, const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        return send_response(connection, "Error: File not found", MHD_HTTP_NOT_FOUND, TEXT_PLAIN);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(file_size + 1); // Allocate memory for file content (+1 for null terminator)
    if (!buffer) {
        fclose(file);
        return send_response(connection, "Error: Memory allocation failed", MHD_HTTP_INTERNAL_SERVER_ERROR, TEXT_PLAIN);
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);

    if (bytes_read != file_size) {
        free(buffer);
        return send_response(connection, "Error: Failed to read file", MHD_HTTP_INTERNAL_SERVER_ERROR, TEXT_PLAIN);
    }

    buffer[file_size] = '\0'; // Null-terminate the buffer

    return send_response(connection, buffer, MHD_HTTP_OK, APP_JS);
}

static enum MHD_Result serve_css_file(struct MHD_Connection *connection, const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        return send_response(connection, "Error: File not found", MHD_HTTP_NOT_FOUND, TEXT_PLAIN);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(file_size + 1); // Allocate memory for file content (+1 for null terminator)
    if (!buffer) {
        fclose(file);
        return send_response(connection, "Error: Memory allocation failed", MHD_HTTP_INTERNAL_SERVER_ERROR, TEXT_PLAIN);
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);

    if (bytes_read != file_size) {
        free(buffer);
        return send_response(connection, "Error: Failed to read file", MHD_HTTP_INTERNAL_SERVER_ERROR, TEXT_PLAIN);
    }

    buffer[file_size] = '\0'; // Null-terminate the buffer

    return send_response(connection, buffer, MHD_HTTP_OK, TEXT_CSS);
}

static enum MHD_Result serve_png_file(struct MHD_Connection *connection, const char *file_path) {
    FILE *file = fopen(file_path, "rb"); // Open in binary mode for PNG files
    if (!file) {
        return send_response(connection, "Error: File not found", MHD_HTTP_NOT_FOUND, TEXT_PLAIN);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(file_size);
    if (!buffer) {
        fclose(file);
        return send_response(connection, "Error: Memory allocation failed", MHD_HTTP_INTERNAL_SERVER_ERROR, TEXT_PLAIN);
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);

    if (bytes_read != file_size) {
        free(buffer);
        return send_response(connection, "Error: Failed to read file", MHD_HTTP_INTERNAL_SERVER_ERROR, TEXT_PLAIN);
    }

    // Send the response asynchronously
    struct MHD_Response *response = MHD_create_response_from_buffer(file_size, buffer, MHD_RESPMEM_MUST_FREE);
    if (!response) {
        free(buffer);
        return send_response(connection, "Error: Failed to create response", MHD_HTTP_INTERNAL_SERVER_ERROR, TEXT_PLAIN);
    }

    enum MHD_Result result = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return result;
}

static enum MHD_Result serve_map_file(struct MHD_Connection *connection, const char *file_path) {
    FILE *file = fopen(file_path, "r");
    if (!file) {
        return send_response(connection, "Error: File not found", MHD_HTTP_NOT_FOUND, TEXT_PLAIN);
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buffer = (char *)malloc(file_size + 1); // Allocate memory for file content (+1 for null terminator)
    if (!buffer) {
        fclose(file);
        return send_response(connection, "Error: Memory allocation failed", MHD_HTTP_INTERNAL_SERVER_ERROR, TEXT_PLAIN);
    }

    size_t bytes_read = fread(buffer, 1, file_size, file);
    fclose(file);

    if (bytes_read != file_size) {
        free(buffer);
        return send_response(connection, "Error: Failed to read file", MHD_HTTP_INTERNAL_SERVER_ERROR, TEXT_PLAIN);
    }

    buffer[file_size] = '\0'; // Null-terminate the buffer

    return send_response(connection, buffer, MHD_HTTP_OK, APPLICATION_MAP);
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
  } else if (strcmp(url, "/api-docs") == 0)
  {
    char *swaggerJSON = readHTMLFile("swagger.json");

    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(swaggerJSON),
                                                                    (void *)swaggerJSON,
                                                                    MHD_RESPMEM_MUST_FREE); // Free memory after use
    MHD_add_response_header(response, "Content-Type", "application/json");
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
  } else if (strcmp(url, "/swagger-initializer.js") == 0)
  {
    return serve_js_file(connection, "swaggerui/swagger-initializer.js");
  } else if (strcmp(url, "/swagger-ui-bundle.js") == 0)
  {
    return serve_js_file(connection, "swaggerui/swagger-ui-bundle.js");
  } else if (strcmp(url, "/swagger-ui-es-bundle-core.js") == 0)
  {
    return serve_js_file(connection, "swaggerui/swagger-ui-es-bundle-core.js");
  } else if (strcmp(url, "/swagger-ui-es-bundle-core.js") == 0)
  {
    return serve_js_file(connection, "swaggerui/swagger-ui-es-bundle-core.js");
  } else if (strcmp(url, "/swagger-ui-es-bundle.js") == 0)
  {
    return serve_js_file(connection, "swaggerui/swagger-ui-es-bundle.js");
  } else if (strcmp(url, "/swagger-ui-standalone-preset.js") == 0)
  {
    return serve_js_file(connection, "swaggerui/swagger-ui-standalone-preset.js");
  } else if (strcmp(url, "/swagger-ui.js") == 0)
  {
    return serve_js_file(connection, "swaggerui/swagger-ui.js");
  } else if (strcmp(url, "/swagger-ui.css") == 0)
  {
    return serve_css_file(connection, "swaggerui/swagger-ui.css");
  } else if (strcmp(url, "/index.css") == 0)
  {
    return serve_css_file(connection, "swaggerui/index.css");
  } else if (strcmp(url, "/favicon-16x16.png") == 0)
  {
    return serve_png_file(connection, "swaggerui/favicon-16x16.png");
  } else if (strcmp(url, "/favicon-32x32.png") == 0)
  {
    return serve_png_file(connection, "swaggerui/favicon-32x32.png");
  } else if (strcmp(url, "/oauth2-redirect.html") == 0)
  {
    // Serve Swagger UI files here
    char *oauthHTML = readHTMLFile("swaggerui/oauth2-redirect.html");

    struct MHD_Response *response = MHD_create_response_from_buffer(strlen(oauthHTML),
                                                                    (void *)oauthHTML,
                                                                    MHD_RESPMEM_MUST_FREE); // Free memory after use
    MHD_add_response_header(response, "Content-Type", "text/html");
    int ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);
    return ret;
  } else if (strcmp(url, "/swagger-ui.css.map") == 0)
  {
    return serve_map_file(connection, "swaggerui/swagger-ui.css.map");
  } else if (strcmp(url, "/swagger") == 0)
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
  } else if (strcmp(url, "/api/todos") == 0 || strstr(url, "/api/todos/") != NULL)
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
