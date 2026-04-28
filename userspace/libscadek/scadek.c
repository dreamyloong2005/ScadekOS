/* SPDX-License-Identifier: MPL-2.0 */
/* Copyright (c) 2026 The Scadek OS Project contributors */

#include <scadek/scadek.h>

uint64_t scadek_strlen(const char *s) {
    uint64_t len = 0;

    if (s == 0) {
        return 0;
    }

    while (s[len] != '\0') {
        len++;
    }

    return len;
}

void scadek_message_init(struct scadek_message *message,
                         uint64_t sender,
                         uint64_t target,
                         uint64_t type) {
    if (message == 0) {
        return;
    }

    message->sender = sender;
    message->target = target;
    message->type = type;
    message->arg0 = 0;
    message->arg1 = 0;
    message->arg2 = 0;
    message->arg3 = 0;
    message->status = 0;
}

scadek_status_t scadek_console_write(scadek_cap_t console,
                                     const char *buffer,
                                     uint64_t length) {
    scadek_cap_t grant;
    uint64_t offset = 0;
    scadek_status_t status = SCADEK_OK;

    if (console == 0 || buffer == 0) {
        return SCADEK_ERR_INVAL;
    }

    if (length == 0) {
        return SCADEK_OK;
    }

    grant = scadek_grant_create(buffer, length, SCADEK_RIGHT_READ, console);
    if (grant == 0 || (scadek_status_t)grant < 0) {
        return grant == 0 ? SCADEK_ERR_NOENT : (scadek_status_t)grant;
    }

    while (offset < length) {
        struct scadek_message message;
        uint64_t chunk = length - offset;

        if (chunk > SCADEK_CONSOLE_WRITE_MAX) {
            uint64_t candidate = SCADEK_CONSOLE_WRITE_MAX;

            while (candidate > 0) {
                if (buffer[offset + candidate - 1u] == '\n') {
                    break;
                }
                candidate--;
            }

            if (candidate != 0) {
                chunk = candidate;
            } else {
                chunk = SCADEK_CONSOLE_WRITE_MAX;
            }
        }

        if (chunk > SCADEK_CONSOLE_WRITE_MAX) {
            chunk = SCADEK_CONSOLE_WRITE_MAX;
        }

        scadek_message_init(&message,
                            0,
                            SCADEK_SERVICE_CONSOLE,
                            SCADEK_MSG_CONSOLE_WRITE);
        message.arg0 = grant;
        message.arg1 = offset;
        message.arg2 = chunk;

        status = scadek_endpoint_call(console, &message);
        if (status != SCADEK_OK) {
            break;
        }
        if ((scadek_status_t)message.status != SCADEK_OK) {
            status = (scadek_status_t)message.status;
            break;
        }

        offset += chunk;
    }

    {
        scadek_status_t revoke_status = scadek_grant_revoke(grant);
        if (status == SCADEK_OK && revoke_status != SCADEK_OK) {
            status = revoke_status;
        }
    }

    return status;
}

scadek_status_t scadek_console_puts(scadek_cap_t console, const char *s) {
    return scadek_console_write(console, s, scadek_strlen(s));
}

scadek_status_t scadek_console_clear(scadek_cap_t console) {
    struct scadek_message message;

    if (console == 0) {
        return SCADEK_ERR_INVAL;
    }

    scadek_message_init(&message,
                        0,
                        SCADEK_SERVICE_CONSOLE,
                        SCADEK_MSG_CONSOLE_CLEAR);
    return scadek_endpoint_call(console, &message);
}

scadek_status_t scadek_console_scroll(scadek_cap_t console, int32_t lines) {
    struct scadek_message message;

    if (console == 0) {
        return SCADEK_ERR_INVAL;
    }

    scadek_message_init(&message,
                        0,
                        SCADEK_SERVICE_CONSOLE,
                        SCADEK_MSG_CONSOLE_SCROLL);
    message.arg0 = (uint64_t)(int64_t)lines;
    return scadek_endpoint_call(console, &message);
}

scadek_status_t scadek_console_get_info(scadek_cap_t console,
                                        struct scadek_console_info *out) {
    struct scadek_message message;
    scadek_status_t status;

    if (console == 0 || out == 0) {
        return SCADEK_ERR_INVAL;
    }

    scadek_message_init(&message,
                        0,
                        SCADEK_SERVICE_CONSOLE,
                        SCADEK_MSG_CONSOLE_GET_INFO);
    status = scadek_endpoint_call(console, &message);
    if (status != SCADEK_OK) {
        return status;
    }
    if ((scadek_status_t)message.status != SCADEK_OK) {
        return (scadek_status_t)message.status;
    }

    out->columns = (uint32_t)message.arg0;
    out->rows = (uint32_t)message.arg1;
    out->cursor_x = (uint32_t)(message.arg2 >> 32u);
    out->cursor_y = (uint32_t)message.arg2;
    out->flags = (uint32_t)message.arg3;
    return SCADEK_OK;
}

scadek_status_t scadek_tty_poll_event(scadek_cap_t tty,
                                      struct scadek_input_event *out) {
    struct scadek_message message;
    scadek_status_t status;

    if (tty == 0 || out == 0) {
        return SCADEK_ERR_INVAL;
    }

    scadek_message_init(&message,
                        0,
                        SCADEK_SERVICE_TTY,
                        SCADEK_MSG_TTY_POLL_EVENT);
    status = scadek_endpoint_call(tty, &message);
    if (status != SCADEK_OK) {
        return status;
    }

    out->timestamp = message.arg0;
    out->type = (uint32_t)(message.arg1 >> 32u);
    out->keycode = (uint32_t)message.arg1;
    out->ascii = (uint32_t)message.arg2;
    out->modifiers = (uint32_t)(message.arg3 >> 32u);
    out->flags = (uint32_t)message.arg3;
    return (scadek_status_t)message.status;
}

scadek_status_t scadek_tty_read_line(scadek_cap_t tty,
                                     char *buffer,
                                     uint64_t capacity) {
    uint64_t pos = 0;

    if (tty == 0 || buffer == 0 || capacity == 0) {
        return SCADEK_ERR_INVAL;
    }

    for (;;) {
        struct scadek_input_event event;
        scadek_status_t status = scadek_tty_poll_event(tty, &event);

        if (status == SCADEK_ERR_NOENT) {
            (void)scadek_yield();
            continue;
        }
        if (status != SCADEK_OK) {
            return status;
        }
        if (event.type != SCADEK_INPUT_KEY_DOWN) {
            continue;
        }
        if (event.ascii == '\n' || event.ascii == '\r') {
            buffer[pos] = '\0';
            return SCADEK_OK;
        }
        if (event.ascii == '\b') {
            if (pos > 0) {
                pos--;
            }
            continue;
        }
        if (event.ascii >= 0x20u && event.ascii < 0x7fu && pos + 1u < capacity) {
            buffer[pos++] = (char)event.ascii;
        }
    }
}

scadek_status_t scadek_service_lookup(scadek_cap_t session,
                                      uint64_t service_id,
                                      scadek_cap_t *out_endpoint) {
    struct scadek_message message;
    scadek_status_t status;

    if (session == 0 || service_id == SCADEK_SERVICE_NONE || out_endpoint == 0) {
        return SCADEK_ERR_INVAL;
    }

    scadek_message_init(&message,
                        0,
                        SCADEK_SERVICE_SESSION,
                        SCADEK_MSG_SERVICE_LOOKUP);
    message.arg0 = service_id;

    status = scadek_endpoint_call(session, &message);
    if (status != SCADEK_OK) {
        return status;
    }
    if ((scadek_status_t)message.status != SCADEK_OK) {
        return (scadek_status_t)message.status;
    }

    *out_endpoint = message.arg0;
    return message.arg0 == 0 ? SCADEK_ERR_NOENT : SCADEK_OK;
}

scadek_status_t scadek_proc_spawn_with_bootstrap(scadek_cap_t proc,
                                                 const char *path,
                                                 scadek_cap_t bootstrap) {
    struct scadek_message message;
    scadek_status_t status;

    if (proc == 0 || path == 0) {
        return SCADEK_ERR_INVAL;
    }

    scadek_message_init(&message,
                        0,
                        SCADEK_SERVICE_PROC,
                        SCADEK_MSG_PROCESS_SPAWN);
    message.arg0 = (uint64_t)(uintptr_t)path;
    message.arg1 = scadek_strlen(path);
    message.arg2 = bootstrap;

    status = scadek_endpoint_call(proc, &message);
    if (status != SCADEK_OK) {
        return status;
    }

    return (scadek_status_t)message.status;
}

scadek_status_t scadek_proc_spawn(scadek_cap_t proc, const char *path) {
    return scadek_proc_spawn_with_bootstrap(proc, path, 0);
}

scadek_status_t scadek_vfs_stat(scadek_cap_t vfs,
                                const char *path,
                                struct scadek_vfs_stat *out) {
    struct scadek_message message;
    scadek_status_t status;

    if (vfs == 0 || path == 0 || out == 0) {
        return SCADEK_ERR_INVAL;
    }

    scadek_message_init(&message, 0, SCADEK_SERVICE_VFS, SCADEK_MSG_STAT);
    message.arg0 = (uint64_t)(uintptr_t)path;
    message.arg1 = scadek_strlen(path);

    status = scadek_endpoint_call(vfs, &message);
    if (status != SCADEK_OK) {
        return status;
    }
    if ((scadek_status_t)message.status != SCADEK_OK) {
        return (scadek_status_t)message.status;
    }

    out->type = message.arg0;
    out->size = message.arg1;
    return SCADEK_OK;
}

scadek_status_t scadek_vfs_listdir(scadek_cap_t vfs,
                                   const char *path,
                                   struct scadek_vfs_dirent *entries,
                                   uint64_t capacity,
                                   uint64_t *out_count) {
    struct scadek_message message;
    scadek_status_t status;

    if (vfs == 0 || path == 0 || entries == 0 || capacity == 0 || out_count == 0) {
        return SCADEK_ERR_INVAL;
    }

    scadek_message_init(&message, 0, SCADEK_SERVICE_VFS, SCADEK_MSG_LISTDIR);
    message.arg0 = (uint64_t)(uintptr_t)path;
    message.arg1 = scadek_strlen(path);
    message.arg2 = (uint64_t)(uintptr_t)entries;
    message.arg3 = capacity * sizeof(entries[0]);

    status = scadek_endpoint_call(vfs, &message);
    if (status != SCADEK_OK) {
        return status;
    }
    if ((scadek_status_t)message.status != SCADEK_OK) {
        return (scadek_status_t)message.status;
    }

    *out_count = message.arg0;
    return SCADEK_OK;
}

scadek_status_t scadek_vfs_open(scadek_cap_t vfs,
                                const char *path,
                                scadek_cap_t *out_file,
                                uint64_t *out_size) {
    struct scadek_message message;
    scadek_status_t status;

    if (vfs == 0 || path == 0 || out_file == 0 || out_size == 0) {
        return SCADEK_ERR_INVAL;
    }

    scadek_message_init(&message, 0, SCADEK_SERVICE_VFS, SCADEK_MSG_OPEN);
    message.arg0 = (uint64_t)(uintptr_t)path;
    message.arg1 = scadek_strlen(path);

    status = scadek_endpoint_call(vfs, &message);
    if (status != SCADEK_OK) {
        return status;
    }
    if ((scadek_status_t)message.status != SCADEK_OK) {
        return (scadek_status_t)message.status;
    }

    *out_file = message.arg0;
    *out_size = message.arg1;
    return message.arg0 == 0 ? SCADEK_ERR_NOENT : SCADEK_OK;
}

scadek_status_t scadek_vfs_read(scadek_cap_t vfs,
                                scadek_cap_t file,
                                uint64_t offset,
                                char *buffer,
                                uint64_t capacity,
                                uint64_t *out_read) {
    struct scadek_message message;
    scadek_status_t status;

    if (vfs == 0 || file == 0 || buffer == 0 || capacity == 0 || out_read == 0) {
        return SCADEK_ERR_INVAL;
    }

    scadek_message_init(&message, 0, SCADEK_SERVICE_VFS, SCADEK_MSG_READ);
    message.arg0 = file;
    message.arg1 = offset;
    message.arg2 = (uint64_t)(uintptr_t)buffer;
    message.arg3 = capacity;

    status = scadek_endpoint_call(vfs, &message);
    if (status != SCADEK_OK) {
        return status;
    }
    if ((scadek_status_t)message.status != SCADEK_OK) {
        return (scadek_status_t)message.status;
    }

    *out_read = message.arg0;
    return SCADEK_OK;
}

scadek_status_t scadek_vfs_close(scadek_cap_t vfs, scadek_cap_t file) {
    struct scadek_message message;
    scadek_status_t status;

    if (vfs == 0 || file == 0) {
        return SCADEK_ERR_INVAL;
    }

    scadek_message_init(&message, 0, SCADEK_SERVICE_VFS, SCADEK_MSG_CLOSE);
    message.arg0 = file;

    status = scadek_endpoint_call(vfs, &message);
    if (status != SCADEK_OK) {
        return status;
    }

    return (scadek_status_t)message.status;
}
