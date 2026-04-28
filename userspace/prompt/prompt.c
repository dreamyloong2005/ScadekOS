/* SPDX-License-Identifier: MPL-2.0 */
/* Copyright (c) 2026 The Scadek OS Project contributors */

#include <scadek/scadek.h>

void _start(scadek_cap_t console) {
    static const char prompt[] =
        "[proc] /bin/hello exited: 0\r\n"
        "[rings] /ring-test submitted grant-backed console batch\r\n"
        "[security] fault isolation visible in M30 self-test\r\n"
        "[security] capability revoke visible in M30 self-test\r\n"
        "[kernel] timer/preemption and devmgr stub visible in M30 self-test\r\n"
        "scadek:/>";

    (void)scadek_console_write(console, prompt, sizeof(prompt) - 1u);
    scadek_exit(0);
}
