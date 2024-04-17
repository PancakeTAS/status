/// \file systemd.h This file interacts with systemd

#pragma once

typedef enum SYSTEMDSTATUS {
    SYSTEMD_OK, //!< No error
    SYSTEMD_ERR, //!< Error with libsystemd
} systemd_status; //!< Socket status enumeration

/**
 * Connect to systemd
 *
 * \return
 *   SYSTEMD_OK if no error, SYSTEMD_ERR if error connecting to systemd
 */
int systemd_connect();

/**
 * Check if a service is running
 *
 * \param service
 *   The service to check
 *
 * \return
 *   1 if the service is running, 0 if the service is not running, negative error code if error
 */
int systemd_check_running(char* service);

/**
 * Disconnect from systemd
 */
void systemd_disconnect();
