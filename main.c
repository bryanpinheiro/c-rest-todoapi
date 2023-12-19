#include <microhttpd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define HTML_RESPONSE "<html><head><title>Hello C api</title></head><body>C api made by Bryan Pinheiro</body></html>"
#define JSON_RESPONSE "{\"message\": \"This is a JSON response.\"}"

static enum MHD_Result
ahc_echo(void *cls,
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

  if (strcmp(url, "/api/") == 0)
  {
    response = MHD_create_response_from_buffer(strlen(JSON_RESPONSE),
                                               (void *)JSON_RESPONSE,
                                               MHD_RESPMEM_PERSISTENT);
    MHD_add_response_header(response, "Content-Type", "application/json");
    ret = MHD_queue_response(connection,
                             MHD_HTTP_OK,
                             response);
    MHD_destroy_response(response);
    return ret;
  }

  if (strcmp(url, "/") == 0)
  {
    response = MHD_create_response_from_buffer(strlen(HTML_RESPONSE),
                                               (void *)HTML_RESPONSE,
                                               MHD_RESPMEM_PERSISTENT);
    ret = MHD_queue_response(connection,
                             MHD_HTTP_OK,
                             response);
    MHD_destroy_response(response);
    return ret;
  }

  /* If URL not found, return a 404 */
  response = MHD_create_response_from_buffer(0, NULL, MHD_RESPMEM_PERSISTENT);
  ret = MHD_queue_response(connection, MHD_HTTP_NOT_FOUND, response);
  MHD_destroy_response(response);
  return ret;
}

int main(int argc,
         char **argv)
{
  struct MHD_Daemon *d;

  if (argc != 2)
  {
    printf("%s PORT\n", argv[0]);
    return 1;
  }

  d = MHD_start_daemon(MHD_USE_THREAD_PER_CONNECTION,
                       atoi(argv[1]),
                       NULL,
                       NULL,
                       &ahc_echo,
                       HTML_RESPONSE,
                       MHD_OPTION_END);

  if (d == NULL)
    return 1;

  (void)getc(stdin);
  MHD_stop_daemon(d);
  return 0;
}
