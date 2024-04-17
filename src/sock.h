/// \file sock.h This file manages socket creation and management

#pragma once

#include <netinet/in.h>

typedef struct SOCKOption {
    int level; //!< [in] Socket level
    int optname; //!< [in] Socket option name
    const void* optval; //!< [in] Socket option value
    socklen_t optlen; //!< [in] Socket option length
} sock_option; //!< Socket option structure

typedef struct SOCKServer {
    int port; //!< [in] Port number
    sock_option* options; //!< [in] Socket options
    int num_options; //!< [in] Number of socket options
    int sock_fd; //!< Socket file descriptor
    struct sockaddr_in sock_addr; //!< Socket address
} sock_server; //!< Socket server structure

typedef struct SOCKClient {
    int client_fd; //!< Client file descriptor
    struct sockaddr_in client_addr; //!< Client address
} sock_client; //!< Socket client structure

typedef enum SOCKSTATUS {
    SOCK_OK, //!< No error
    SOCK_SOCKET_ERROR, //!< Error creating socket
    SOCK_BIND_ERROR, //!< Error binding socket
    SOCK_LISTEN_ERROR, //!< Error listening on socket
    SOCK_ACCEPT_ERROR, //!< Error accepting client
    SOCK_CLOSE_ERROR //!< Error closing socket
} sock_status; //!< Socket status enumeration

/**
 * Create a socket server
 *
 * \param server
  *  Struct containing the server information
  *
  * \return
  *   SOCK_OK if no error, SOCK_SOCKET_ERROR if error creating socket, SOCK_BIND_ERROR if error binding socket, SOCK_LISTEN_ERROR if error listening on socket
 */
sock_status sock_create_server(sock_server* server);

/**
 * Accept a client connection
 *
 * \param server
 *   Struct containing the server information
 * \param client
 *   Struct containing the client information
 *
 * \return
 *   SOCK_OK if no error, SOCK_ACCEPT_ERROR if error accepting client
 */
sock_status sock_accept_client(const sock_server* server, sock_client* client);

/**
 * Close a client socket
 *
 * \param client
 *   Struct containing the client information
 *
 * \return
 *   SOCK_OK if no error, SOCK_CLOSE_ERROR if error closing socket
 */
sock_status sock_close_client(const sock_client* client);

/**
 * Close a server socket
 *
 * \param server
 *   Struct containing the server information
 *
 * \return
 *   SOCK_OK if no error, SOCK_CLOSE_ERROR if error closing socket
 */
sock_status sock_close_server(const sock_server* server);
