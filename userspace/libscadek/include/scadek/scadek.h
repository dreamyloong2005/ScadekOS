/* SPDX-License-Identifier: MPL-2.0 */
/* Copyright (c) 2026 The Scadek OS Project contributors */

#ifndef SCADEK_SCADEK_H
#define SCADEK_SCADEK_H

#include <stdint.h>

typedef int64_t scadek_status_t;
typedef uint64_t scadek_cap_t;

#define SCADEK_OK             0
#define SCADEK_ERR_INVAL     -1
#define SCADEK_ERR_NOMEM     -2
#define SCADEK_ERR_NOENT     -3
#define SCADEK_ERR_PERM      -4
#define SCADEK_ERR_BOUNDS    -5
#define SCADEK_ERR_BUSY      -6
#define SCADEK_ERR_NOTSUP    -7

#define SCADEK_RIGHT_READ     (1ull << 0)
#define SCADEK_RIGHT_WRITE    (1ull << 1)
#define SCADEK_RIGHT_EXEC     (1ull << 2)
#define SCADEK_RIGHT_MAP      (1ull << 3)
#define SCADEK_RIGHT_SEND     (1ull << 4)
#define SCADEK_RIGHT_RECV     (1ull << 5)
#define SCADEK_RIGHT_BIND     (1ull << 6)
#define SCADEK_RIGHT_REVOKE   (1ull << 7)

#define SCADEK_SERVICE_NONE       0ull
#define SCADEK_SERVICE_CONSOLE    1ull
#define SCADEK_SERVICE_TMPFS      2ull
#define SCADEK_SERVICE_VFS        3ull
#define SCADEK_SERVICE_PROC       4ull
#define SCADEK_SERVICE_GRANT_TEST 5ull
#define SCADEK_SERVICE_DEVMGR     6ull
#define SCADEK_SERVICE_TTY        7ull
#define SCADEK_SERVICE_SESSION    8ull

#define SCADEK_MSG_NONE              0ull
#define SCADEK_MSG_OPEN              1ull
#define SCADEK_MSG_CLOSE             2ull
#define SCADEK_MSG_READ              3ull
#define SCADEK_MSG_WRITE             4ull
#define SCADEK_MSG_DEVICE_REGISTER   5ull
#define SCADEK_MSG_DEVICE_QUEUE_BIND 6ull
#define SCADEK_MSG_SERVICE_REGISTER  7ull
#define SCADEK_MSG_SERVICE_LOOKUP    8ull
#define SCADEK_MSG_PROCESS_SPAWN     9ull
#define SCADEK_MSG_PROCESS_EXIT      10ull
#define SCADEK_MSG_PROCESS_WAIT_STUB 11ull
#define SCADEK_MSG_RING_PROCESS      12ull

#define SCADEK_MSG_CONSOLE_WRITE            0x2000ull
#define SCADEK_MSG_CONSOLE_CLEAR            0x2001ull
#define SCADEK_MSG_CONSOLE_GET_INFO         0x2002ull
#define SCADEK_MSG_CONSOLE_BIND_OUTPUT_RING 0x2003ull

#define SCADEK_MSG_TTY_POLL_EVENT      0x2100ull
#define SCADEK_MSG_TTY_BIND_INPUT_RING 0x2101ull
#define SCADEK_MSG_TTY_GET_INFO        0x2102ull

#define SCADEK_MSG_STAT    0x3000ull
#define SCADEK_MSG_LISTDIR 0x3001ull

#define SCADEK_RING_OP_NONE          0ull
#define SCADEK_RING_OP_CONSOLE_WRITE 1ull
#define SCADEK_RING_BATCH            16ull

#define SCADEK_INPUT_NONE     0u
#define SCADEK_INPUT_KEY_DOWN 1u
#define SCADEK_INPUT_KEY_UP   2u

#define SCADEK_INPUT_MOD_SHIFT (1u << 0)

#define SCADEK_CONSOLE_WRITE_MAX 128ull
#define SCADEK_VFS_MAX_NAME 64ull

#define SCADEK_VFS_NODE_NONE 0ull
#define SCADEK_VFS_NODE_FILE 1ull
#define SCADEK_VFS_NODE_DIR  2ull

struct scadek_message {
    uint64_t sender;
    uint64_t target;
    uint64_t type;
    uint64_t arg0;
    uint64_t arg1;
    uint64_t arg2;
    uint64_t arg3;
    uint64_t status;
};

struct scadek_ring_desc {
    uint64_t op;
    uint64_t cap;
    uint64_t grant;
    uint64_t offset;
    uint64_t length;
    uint64_t flags;
};

struct scadek_completion {
    uint64_t op;
    uint64_t status;
    uint64_t result0;
    uint64_t result1;
};

struct scadek_input_event {
    uint64_t timestamp;
    uint32_t type;
    uint32_t keycode;
    uint32_t ascii;
    uint32_t modifiers;
    uint32_t flags;
};

struct scadek_vfs_stat {
    uint64_t type;
    uint64_t size;
};

struct scadek_vfs_dirent {
    char name[SCADEK_VFS_MAX_NAME];
    uint64_t type;
    uint64_t size;
};

uint64_t scadek_strlen(const char *s);
void scadek_message_init(struct scadek_message *message,
                         uint64_t sender,
                         uint64_t target,
                         uint64_t type);
scadek_status_t scadek_endpoint_call(scadek_cap_t endpoint,
                                     struct scadek_message *message);
scadek_cap_t scadek_grant_create(const void *buffer,
                                 uint64_t length,
                                 uint64_t rights,
                                 scadek_cap_t target);
scadek_status_t scadek_grant_revoke(scadek_cap_t grant);
scadek_cap_t scadek_ring_create(uint64_t entries);
scadek_status_t scadek_ring_bind(scadek_cap_t ring, scadek_cap_t endpoint);
scadek_status_t scadek_ring_submit(scadek_cap_t ring,
                                   const struct scadek_ring_desc *descs,
                                   uint64_t count);
uint64_t scadek_ring_poll(scadek_cap_t ring,
                          struct scadek_completion *completions,
                          uint64_t count);
scadek_status_t scadek_yield(void);
scadek_status_t scadek_console_write(scadek_cap_t console,
                                     const char *buffer,
                                     uint64_t length);
scadek_status_t scadek_console_puts(scadek_cap_t console, const char *s);
scadek_status_t scadek_console_clear(scadek_cap_t console);
scadek_status_t scadek_tty_poll_event(scadek_cap_t tty,
                                      struct scadek_input_event *out);
scadek_status_t scadek_tty_read_line(scadek_cap_t tty,
                                     char *buffer,
                                     uint64_t capacity);
scadek_status_t scadek_service_lookup(scadek_cap_t session,
                                      uint64_t service_id,
                                      scadek_cap_t *out_endpoint);
scadek_status_t scadek_proc_spawn(scadek_cap_t proc, const char *path);
scadek_status_t scadek_proc_spawn_with_bootstrap(scadek_cap_t proc,
                                                 const char *path,
                                                 scadek_cap_t bootstrap);
scadek_status_t scadek_vfs_stat(scadek_cap_t vfs,
                                const char *path,
                                struct scadek_vfs_stat *out);
scadek_status_t scadek_vfs_listdir(scadek_cap_t vfs,
                                   const char *path,
                                   struct scadek_vfs_dirent *entries,
                                   uint64_t capacity,
                                   uint64_t *out_count);
scadek_status_t scadek_vfs_open(scadek_cap_t vfs,
                                const char *path,
                                scadek_cap_t *out_file,
                                uint64_t *out_size);
scadek_status_t scadek_vfs_read(scadek_cap_t vfs,
                                scadek_cap_t file,
                                uint64_t offset,
                                char *buffer,
                                uint64_t capacity,
                                uint64_t *out_read);
scadek_status_t scadek_vfs_close(scadek_cap_t vfs, scadek_cap_t file);
__attribute__((noreturn)) void scadek_exit(uint64_t status);

#endif
