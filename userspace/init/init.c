/* SPDX-License-Identifier: MPL-2.0 */
/* Copyright (c) 2026 The Scadek OS Project contributors */

#include <scadek/scadek.h>

void _start(scadek_cap_t session) {
    scadek_cap_t proc = 0;

    if (scadek_service_lookup(session, SCADEK_SERVICE_PROC, &proc) == SCADEK_OK) {
        (void)scadek_proc_spawn_with_bootstrap(proc, "/runner", session);
    }

    scadek_exit(0);
}
