// todoroutes.h

#ifndef TODOROUTES_H
#define TODOROUTES_H

#include <microhttpd.h>

enum MHD_Result handleTodoRoutes(void *cls, struct MHD_Connection *connection, const char *url,
                                 const char *method, const char *version, const char *upload_data,
                                 size_t *upload_data_size, void **ptr);

#endif /* TODOROUTES_H */
