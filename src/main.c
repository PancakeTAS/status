#include "log.h"
#include "sock.h"
#include "http.h"
#include "systemd.h"

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

http_status pong_handler(const sock_client* client, char* request, http_response* response) {
    log_info("HTTP", "Handling pong request: %s", request);

    response->status = 200;
    response->message = "OK";
    response->length = 7;
    response->data = "Pong!\r\n";
    return HTTP_OK;
}

http_status systemd_handler(const sock_client* client, char* request, http_response* response) {
    log_info("HTTP", "Handling systemd request: %s", request);

    int status = systemd_check_running(request);
    if (status < 0) {
        response->status = 500;
        response->message = "Internal Server Error";
        response->length = 0;
        response->data = NULL;
        return HTTP_ERROR;
    }

    if (status) {
        response->status = 200;
        response->message = "OK";
        response->length = 1;
        response->data = "1";
    } else {
        response->status = 200;
        response->message = "OK";
        response->length = 1;
        response->data = "0";
    }
    return HTTP_OK;
}

sock_server* pServer;
sock_client* pClient;
http_context* pContext;

void sigint_handler(int signum) {
    log_info("MAIN", "Received SIGINT, exiting...");
    if (pClient) sock_close_client(pClient);
    if (pServer) sock_close_server(pServer);
    if (pContext) http_free_context(pContext);
    systemd_disconnect();
    exit(EXIT_SUCCESS);
}

int main() {
    // setup signal handler
    signal(SIGINT, sigint_handler);
    signal(SIGPIPE, SIG_IGN);

    // setup log level
    log_set_level(LOG_INFO);

    // connect to systemd
    log_info("MAIN", "Connecting to systemd...");
    if (systemd_connect() != SYSTEMD_OK) {
        log_fatal("MAIN", "Error connecting to systemd");
        return EXIT_FAILURE;
    }

    // create http context
    log_info("MAIN", "Creating http context...");
    http_context context = {
        .endpoints = (http_endpoint[]) {
            {
                .path = "",
                .handler = pong_handler
            },

            // discord bots
            {
                .path = "findseed",
                .handler = systemd_handler
            },
            {
                .path = "tas8999",
                .handler = systemd_handler
            },
            {
                .path = "purrify",
                .handler = systemd_handler
            },
            {
                .path = "qotd",
                .handler = systemd_handler
            },

            // minecraft servers
            {
                .path = "gameserver01",
                .handler = systemd_handler
            },
            {
                .path = "proxy",
                .handler = systemd_handler
            },
            {
                .path = "lobby",
                .handler = systemd_handler
            },

            // tino
            {
                .path = "tino",
                .handler = systemd_handler
            },

            // misc
            {
                .path = "httpd",
                .handler = systemd_handler
            },
            {
                .path = "postgresql",
                .handler = systemd_handler
            },
            {
                .path = "reposilite",
                .handler = systemd_handler
            }
        },
        .num_endpoints = 12
    };
    http_create_context(&context);
    pContext = &context;

    // create server socket
    log_info("MAIN", "Creating server socket...");
    sock_server server = {
        .port = 4961,
        .options = (sock_option[]) {
            {
                .level = SOL_SOCKET,
                .optname = SO_REUSEADDR,
                .optval = &(int){1},
                .optlen = sizeof(int)
            },
            {
                .level = SOL_SOCKET,
                .optname = SO_REUSEPORT,
                .optval = &(int){1},
                .optlen = sizeof(int)
            },
            {
                .level = SOL_SOCKET,
                .optname = SO_KEEPALIVE,
                .optval = &(int){1},
                .optlen = sizeof(int)
            }
        },
        .num_options = 3
    };
    sock_status serr = sock_create_server(&server);
    if (serr) {
        log_fatal("MAIN", "Error creating server socket (%d)", serr);

        http_free_context(&context);
        return EXIT_FAILURE;
    }
    pServer = &server;

    sock_client client;
    while (1) {
        log_debug("MAIN", "Accepting new client...");
        if ((serr = sock_accept_client(&server, &client))) {
            log_warn("MAIN", "Error accepting client (%d)", serr);
            continue;
        }
        pClient = &client;

        log_debug("MAIN", "Handling http client request...");
        http_status herr;
        if ((herr = http_handle_request(&client, &context)))
            log_warn("MAIN", "Error handling http request (%d)", herr);

        log_debug("MAIN", "Closing client socket...");
        if ((serr = sock_close_client(&client)))
            log_warn("MAIN", "Error closing client socket (%d)", serr);
        pClient = NULL;
    }

}
