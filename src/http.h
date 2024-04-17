/// \file http.h This file manages http requests and responses

#pragma once

#include "sock.h"

#include <regex.h>

typedef enum HTTPSTATUS {
    HTTP_OK, //!< No error
    HTTP_PARSE_ERROR, //!< Error parsing request
    HTTP_RECV_ERROR, //!< Error receiving request
    HTTP_ERROR //!< Error handling request
} http_status; //!< Http return status

typedef struct HTTPResponse {
    int status; //!< [in] Status code
    char* message; //!< [in] Message
    int length; //!< [in] Length of the message
    char* data; //!< [in] Data
} http_response; //!< Http response structure

typedef struct HTTPEndpoint {
    const char* path; //!< [in] Path to the endpoint
    http_status (*handler)(const sock_client*, char*, http_response*); //!< [in] Handler function
} http_endpoint; //!< Http endpoint structure

typedef struct HTTPContext {
    http_endpoint* endpoints; //!< [in] Array of endpoints
    int num_endpoints; //!< [in] Number of endpoints
    regex_t request_regex; //!< [in] Regex for request
    regex_t header_regex; //!< [in] Regex for headers
    regex_t param_regex; //!< [in] Regex for parameters
} http_context; //!< Http context structure

/**
 * Create a http context
 *
 * \param context
 *   Struct containing the http context
 */
void http_create_context(http_context* context);

/**
 * Handle a http request
 *
 * \param client
 *   Struct containing the client information
 * \param context
 *   Struct containing the http context
 *
 * \return
 *   HTTP_OK if no error, HTTP_RECV_ERROR if error receiving request, HTTP_PARSE_ERROR if error matching request, HTTP_ERROR if error handling request
 */
http_status http_handle_request(const sock_client* client, const http_context* context);

/**
 * Free the http context
 *
 * \param context
 *   Struct containing the http context
 */
void http_free_context(http_context* context);
