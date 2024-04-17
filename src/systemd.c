#include "systemd.h"
#include "log.h"

#include <systemd/sd-bus.h>
#include <string.h>

static sd_bus* bus = NULL; //!< Bus connection

int systemd_connect() {
    int status = sd_bus_open_system(&bus);
    if (status < 0) {
        log_error("SYSTEMD", "sd_bus_open_system() failed: %s", strerror(-status));
        return SYSTEMD_ERR;
    }
    log_trace("SYSTEMD", "sd_bus_open_system() success: %p", bus);

    log_debug("SYSTEMD", "Connected to systemd");
    return SYSTEMD_OK;
}

int systemd_check_running(char* service) {
    // check if service is active
    sd_bus_error error = SD_BUS_ERROR_NULL;
    char* reply = NULL;

    char unit_path[128];
    snprintf(unit_path, sizeof(unit_path), "/org/freedesktop/systemd1/unit/%s_2eservice", service);
    int status = sd_bus_get_property_string(bus,
        "org.freedesktop.systemd1",
        unit_path,
        "org.freedesktop.systemd1.Unit",
        "ActiveState",
        &error,
        &reply
    );
    if (status < 0) {
        log_error("SYSTEMD", "sd_bus_get_property_string() failed: %s", error.message);
        return -SYSTEMD_ERR;
    }
    log_trace("SYSTEMD", "sd_bus_get_property_string() success: %p", reply);

    // check if service is active
    int active = strcmp(reply, "active") == 0;
    log_debug("SYSTEMD", "Service %s is %s", service, active ? "active" : "inactive");

    sd_bus_error_free(&error);
    return active;
}

void systemd_disconnect() {
    if (bus) {
        sd_bus_flush_close_unref(bus);
        log_debug("SYSTEMD", "Disconnected from systemd");
    }
}
