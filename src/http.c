#include "http.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

/// Size of the buffer to read the request
#define BUFFER_SIZE 8192

/// HTTP response format
#define HTTP_RESPONSE "HTTP/1.1 %d %s\r\nContent-Length: %d\r\n\r\n%s"

/// Regex to parse 'GET /path HTTP/1.1' requests
#define REQUEST_REGEX "GET /([^ ]*) HTTP/[0-9].[0-9]"

void http_create_context(http_context* context) {
    regcomp(&context->request_regex, REQUEST_REGEX, REG_EXTENDED);
}

http_status http_handle_request(const sock_client* client, const http_context* context) {
    char buffer[BUFFER_SIZE];

    // read the request
    ssize_t read = recv(client->client_fd, buffer, BUFFER_SIZE, 0);
    if (read < 0) {
        log_trace("HTTP", "recv() failed: %s", strerror(errno));
        dprintf(client->client_fd, HTTP_RESPONSE, 500, "Internal Server Error", 0, "");
        return HTTP_RECV_ERROR;
    } else if (read == BUFFER_SIZE) {
        log_trace("HTTP", "recv() failed: buffer too small, ignoring");
        dprintf(client->client_fd, HTTP_RESPONSE, 413, "Request Entity Too Large", 0, "");
        return HTTP_OK;
    }
    buffer[read] = '\0';
    log_trace("HTTP", "recv() success: %d bytes", read);


    // parse request line
    regmatch_t matches[4];
    if (regexec(&context->request_regex, buffer, 4, matches, 0)) {
        log_error("HTTP", "regexec() failed: invalid request line");
        dprintf(client->client_fd, HTTP_RESPONSE, 400, "Bad Request", 0, "");
        return HTTP_PARSE_ERROR;
    }
    char* request = strndup(buffer + matches[1].rm_so, matches[1].rm_eo - matches[1].rm_so);
    log_trace("HTTP", "regexec() success: parsed request line");


    // handle the request
    for (int i = 0; i < context->num_endpoints; i++) {
        // check if the path matches
        if (strcmp(request, context->endpoints[i].path) != 0) continue;

        // handle the request
        http_response response;
        http_status err = context->endpoints[i].handler(client, request, &response);
        if (err) {
            log_trace("HTTP", "endpoint->handler() failed: %d", err);
            dprintf(client->client_fd, HTTP_RESPONSE, 500, "Internal Server Error", 0, "");

            return err;
        }
        log_trace("HTTP", "endpoint->handler() success: %d %s", response.status, response.message);

        log_debug("HTTP", "Sending response: %d %s", response.status, response.message);
        dprintf(client->client_fd, HTTP_RESPONSE, response.status, response.message, response.length, response.data);
        return HTTP_OK;
    }

    // no matching endpoint
    log_trace("HTTP", "no matching endpoint: 404 Not Found");
    dprintf(client->client_fd, HTTP_RESPONSE, 404, "Not Found", 0, "");
    return HTTP_OK;
}

void http_free_context(http_context* context) {
    regfree(&context->request_regex);
}
