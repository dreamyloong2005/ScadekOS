/* SPDX-License-Identifier: MPL-2.0 */
/* Copyright (c) 2026 The Scadek OS Project contributors */

#include <scadek/scadek.h>

#ifndef SCADEKOS_VERSION
#define SCADEKOS_VERSION "0.1.0-devpreview.4"
#endif

void _start(scadek_cap_t console) {
    static const char message[] =
        "[scadekos] version " SCADEKOS_VERSION "\r\n"
        "[hello] hello from ScadekOS\r\n"
        "[user] hello from spawned process\r\n"
        "[boot] scadekos devpreview.3 user payload reached console endpoint\r\n";

    (void)scadek_console_write(console, message, sizeof(message) - 1u);
    scadek_exit(0);
}
