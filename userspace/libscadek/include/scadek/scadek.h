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

#define SCADEK_SERVICE_CONSOLE    1ull
#define SCADEK_SERVICE_TMPFS      2ull
#define SCADEK_SERVICE_VFS        3ull
#define SCADEK_SERVICE_PROC       4ull
#define SCADEK_SERVICE_GRANT_TEST 5ull

#define SCADEK_MSG_NONE              0ull
#define SCADEK_MSG_OPEN              1ull
#define SCADEK_MSG_CLOSE             2ull
#define SCADEK_MSG_READ              3ull
#define SCADEK_MSG_WRITE             4ull
#define SCADEK_MSG_PROCESS_SPAWN     9ull
#define SCADEK_MSG_PROCESS_EXIT      10ull
#define SCADEK_MSG_PROCESS_WAIT_STUB 11ull
#define SCADEK_MSG_RING_PROCESS      12ull

#define SCADEK_RING_OP_NONE          0ull
#define SCADEK_RING_OP_CONSOLE_WRITE 1ull
#define SCADEK_RING_BATCH            16ull

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
__attribute__((noreturn)) void scadek_exit(uint64_t status);

#endif
