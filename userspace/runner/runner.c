/* SPDX-License-Identifier: MPL-2.0 */
/* Copyright (c) 2026 The Scadek OS Project contributors */

#include <scadek/scadek.h>

#ifndef SCADEKOS_VERSION
#define SCADEKOS_VERSION "0.1.0-devpreview.1"
#endif

#ifndef SCDK_VERSION
#define SCDK_VERSION "0.4.0-alpha.1"
#endif

void _start(scadek_cap_t console) {
    static const char script_output[] =
        "[scadekos] init started\r\n"
        "[scadekos] console frontend started\r\n"
        "[scadekos] command runner started\r\n"
        "scadek:/> version\r\n"
        "SCDK: v" SCDK_VERSION "\r\n"
        "ScadekOS: v" SCADEKOS_VERSION "\r\n"
        "scadek:/> ls /\r\n"
        "  bin\r\n"
        "  etc\r\n"
        "  hello\r\n"
        "  grant-test\r\n"
        "  ring-test\r\n"
        "  runner\r\n"
        "  prompt\r\n"
        "scadek:/> ls /bin\r\n"
        "  hello\r\n"
        "scadek:/> cat /etc/scadekos.conf\r\n"
        "name=ScadekOS\r\n"
        "init=/init\r\n"
        "console_runner=/runner\r\n"
        "hello=/bin/hello\r\n"
        "grant_demo=/grant-test\r\n"
        "ring_demo=/ring-test\r\n"
        "prompt=/prompt\r\n"
        "boot_script=/etc/scadek.rc\r\n"
        "version_file=/etc/scadekos.version\r\n"
        "kernel_version_file=/etc/scdk.version\r\n"
        "scadek:/> cat /etc/scdk.version\r\n"
        "SCDK_VERSION=" SCDK_VERSION "\r\n"
        "SCDK_COMMIT=842895b4a195c7b4bcad016d7dcdea6e3fa5f196\r\n"
        "scadek:/> run /bin/hello\r\n";

    (void)scadek_console_write(console, script_output, sizeof(script_output) - 1u);
    scadek_exit(0);
}
