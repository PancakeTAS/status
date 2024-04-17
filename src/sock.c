#include "sock.h"
#include "log.h"

#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>

sock_status sock_create_server(sock_server* server) {
    // create socket
    server->sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server->sock_fd < 0) {
        log_error("SOCK", "socket() failed: %s", strerror(errno));
        return SOCK_SOCKET_ERROR;
    }
    log_trace("SOCK", "socket() success: :%d", server->port);

    // set socket options
    for (int i = 0; i < server->num_options; i++) {
        if (setsockopt(server->sock_fd, server->options[i].level, server->options[i].optname, server->options[i].optval, server->options[i].optlen) < 0) {
            log_error("SOCK", "setsockopt() failed: %s", strerror(errno));

            close(server->sock_fd);
            return SOCK_SOCKET_ERROR;
        }
        log_trace("SOCK", "setsockopt() success: :%d", server->port);
    }

    // bind socket
    server->sock_addr.sin_family = AF_INET;
    server->sock_addr.sin_addr.s_addr = INADDR_ANY;
    server->sock_addr.sin_port = htons(server->port);

    if (bind(server->sock_fd, (struct sockaddr*) &server->sock_addr, sizeof(server->sock_addr)) < 0) {
        log_error("SOCK", "bind() failed: %s", strerror(errno));

        close(server->sock_fd);
        return SOCK_BIND_ERROR;
    }
    log_trace("SOCK", "bind() success: %s:%d", inet_ntoa(server->sock_addr.sin_addr), ntohs(server->sock_addr.sin_port));

    // start listen on socket
    if (listen(server->sock_fd, 1) < 0) {
        log_error("SOCK", "listen() failed: %s", strerror(errno));

        close(server->sock_fd);
        return SOCK_LISTEN_ERROR;
    }
    log_trace("SOCK", "listen() success: %s:%d", inet_ntoa(server->sock_addr.sin_addr), ntohs(server->sock_addr.sin_port));

    log_debug("SOCK", "Server created: :%d", server->port);
    return SOCK_OK;
}

sock_status sock_accept_client(const sock_server* server, sock_client* client) {
    socklen_t client_addr_len = sizeof(client->client_addr);

    // accept client
    client->client_fd = accept(server->sock_fd, (struct sockaddr *) &client->client_addr, &client_addr_len);
    if (client->client_fd < 0) {
        log_error("SOCK", "accept() failed: %s", strerror(errno));
        return SOCK_ACCEPT_ERROR;
    }
    log_trace("SOCK", "accept() success: %s:%d", inet_ntoa(client->client_addr.sin_addr), ntohs(client->client_addr.sin_port));

    log_debug("SOCK", "Client accepted: %s:%d", inet_ntoa(client->client_addr.sin_addr), ntohs(client->client_addr.sin_port));
    return SOCK_OK;
}

sock_status sock_close_client(const sock_client* client) {
    if (!client->client_fd)
        return SOCK_OK;

    int err = close(client->client_fd);
    if (err < 0) {
        log_error("SOCK", "close() failed: %s", strerror(errno));
        return SOCK_CLOSE_ERROR;
    }
    log_trace("SOCK", "close() success: %s:%d", inet_ntoa(client->client_addr.sin_addr), ntohs(client->client_addr.sin_port));

    log_debug("SOCK", "Client closed: %s:%d", inet_ntoa(client->client_addr.sin_addr), ntohs(client->client_addr.sin_port));
    return SOCK_OK;
}

sock_status sock_close_server(const sock_server* server) {
    if (!server->sock_fd)
        return SOCK_OK;

    int err = close(server->sock_fd);
    if (err < 0) {
        log_error("SOCK", "close() failed: %s", strerror(errno));
        return SOCK_CLOSE_ERROR;
    }
    log_trace("SOCK", "close() success: :%d", server->port);

    log_debug("SOCK", "Server closed: :%d", server->port);
    return SOCK_OK;
}
