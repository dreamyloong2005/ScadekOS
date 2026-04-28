/* SPDX-License-Identifier: MPL-2.0 */
/* Copyright (c) 2026 The Scadek OS Project contributors */

#include <scadek/scadek.h>

void _start(scadek_cap_t proc) {
    (void)scadek_proc_spawn(proc, "/runner");
    (void)scadek_proc_spawn(proc, "/bin/hello");
    (void)scadek_proc_spawn(proc, "/ring-test");
    (void)scadek_proc_spawn(proc, "/prompt");

    scadek_exit(0);
}
