/* SPDX-License-Identifier: MPL-2.0 */
/* Copyright (c) 2026 The Scadek OS Project contributors */

#include <scadek/scadek.h>

#ifndef SCADEKOS_VERSION
#define SCADEKOS_VERSION "0.1.0-devpreview.4"
#endif

#ifndef SCDK_VERSION
#define SCDK_VERSION "0.4.0-alpha.4"
#endif

#define RUNNER_PATH_MAX 128u
#define RUNNER_LINE_MAX 128u
#define RUNNER_READ_MAX 192u
#define RUNNER_DIRENTS 16u

struct runner_state {
    scadek_cap_t session;
    scadek_cap_t console;
    scadek_cap_t tty;
    scadek_cap_t vfs;
    scadek_cap_t proc;
    scadek_cap_t grant_test;
    char cwd[RUNNER_PATH_MAX];
};

static void copy_string(char *dst, uint64_t capacity, const char *src);
static void console_line(struct runner_state *state, const char *s);
static void run_boot_script(struct runner_state *state);
static void interactive_loop(struct runner_state *state);

__attribute__((section(".text.start"), used)) void _start(scadek_cap_t session) {
    struct runner_state state;

    state.session = session;
    state.console = 0;
    state.tty = 0;
    state.vfs = 0;
    state.proc = 0;
    state.grant_test = 0;
    copy_string(state.cwd, sizeof(state.cwd), "/");

    if (scadek_service_lookup(session, SCADEK_SERVICE_CONSOLE, &state.console) != SCADEK_OK) {
        scadek_exit(1);
    }

    console_line(&state, "[scadekos] init started");
    console_line(&state, "[scadekos] console frontend started");

    if (scadek_service_lookup(session, SCADEK_SERVICE_TTY, &state.tty) != SCADEK_OK ||
        scadek_service_lookup(session, SCADEK_SERVICE_VFS, &state.vfs) != SCADEK_OK ||
        scadek_service_lookup(session, SCADEK_SERVICE_PROC, &state.proc) != SCADEK_OK) {
        console_line(&state, "[scadekos] missing boot service");
        scadek_exit(1);
    }

    (void)scadek_service_lookup(session, SCADEK_SERVICE_GRANT_TEST, &state.grant_test);

    console_line(&state, "[scadekos] command runner started");
    run_boot_script(&state);
    console_line(&state, "[scadekos] interactive console ready");
    interactive_loop(&state);
    scadek_exit(0);
}

static int chr_eq(char a, char b) {
    return a == b;
}

static char ascii_lower(char c) {
    if (c >= 'A' && c <= 'Z') {
        return (char)(c - 'A' + 'a');
    }

    return c;
}

static int str_eq(const char *a, const char *b) {
    uint64_t i = 0;

    if (a == 0 || b == 0) {
        return 0;
    }

    while (a[i] != '\0' && b[i] != '\0') {
        if (!chr_eq(a[i], b[i])) {
            return 0;
        }
        i++;
    }

    return a[i] == '\0' && b[i] == '\0';
}

static int str_eq_ci(const char *a, const char *b) {
    uint64_t i = 0;

    if (a == 0 || b == 0) {
        return 0;
    }

    while (a[i] != '\0' && b[i] != '\0') {
        if (!chr_eq(ascii_lower(a[i]), ascii_lower(b[i]))) {
            return 0;
        }
        i++;
    }

    return a[i] == '\0' && b[i] == '\0';
}

static int is_space(char c) {
    return c == ' ' || c == '\t' || c == '\r' || c == '\n';
}

static void copy_string(char *dst, uint64_t capacity, const char *src) {
    uint64_t i = 0;

    if (dst == 0 || capacity == 0) {
        return;
    }

    if (src == 0) {
        dst[0] = '\0';
        return;
    }

    while (src[i] != '\0' && i + 1u < capacity) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

static void console_write(struct runner_state *state, const char *s) {
    if (state != 0 && state->console != 0 && s != 0) {
        (void)scadek_console_write(state->console, s, scadek_strlen(s));
    }
}

static void console_write_char(struct runner_state *state, char c) {
    char buf[2];

    buf[0] = c;
    buf[1] = '\0';
    console_write(state, buf);
}

static void console_line(struct runner_state *state, const char *s) {
    console_write(state, s);
    console_write(state, "\r\n");
}

static int32_t console_scroll_page_lines(struct runner_state *state) {
    struct scadek_console_info info;

    if (state != 0 &&
        scadek_console_get_info(state->console, &info) == SCADEK_OK &&
        info.rows > 1u) {
        return (int32_t)(info.rows - 1u);
    }

    return 8;
}

static int handle_manual_scroll_key(struct runner_state *state,
                                    const struct scadek_input_event *event) {
    int32_t page;

    if (state == 0 || event == 0 || event->ascii != 0u) {
        return 0;
    }

    if (event->keycode == SCADEK_KEY_UP) {
        (void)scadek_console_scroll(state->console, -1);
        return 1;
    }
    if (event->keycode == SCADEK_KEY_DOWN) {
        (void)scadek_console_scroll(state->console, 1);
        return 1;
    }

    page = console_scroll_page_lines(state);
    if (event->keycode == SCADEK_KEY_PAGE_UP) {
        (void)scadek_console_scroll(state->console, -page);
        return 1;
    }
    if (event->keycode == SCADEK_KEY_PAGE_DOWN) {
        (void)scadek_console_scroll(state->console, page);
        return 1;
    }

    return 0;
}

static void path_pop(char *out, uint64_t *len) {
    if (out == 0 || len == 0 || *len <= 1u) {
        return;
    }

    while (*len > 1u && out[*len - 1u] != '/') {
        *len -= 1u;
    }

    if (*len > 1u) {
        *len -= 1u;
    }

    out[*len] = '\0';
}

static void append_component(char *out,
                             uint64_t capacity,
                             uint64_t *len,
                             const char *component,
                             uint64_t component_len) {
    if (out == 0 || len == 0 || component == 0 || component_len == 0u) {
        return;
    }

    if (*len > 1u && *len + 1u < capacity) {
        out[*len] = '/';
        *len += 1u;
    }

    for (uint64_t i = 0; i < component_len && *len + 1u < capacity; i++) {
        out[*len] = component[i];
        *len += 1u;
    }

    out[*len] = '\0';
}

static void normalize_path(struct runner_state *state,
                           const char *input,
                           char *out,
                           uint64_t capacity) {
    char temp[RUNNER_PATH_MAX];
    uint64_t pos = 0;
    uint64_t out_len = 1;

    if (out == 0 || capacity < 2u) {
        return;
    }

    if (input == 0 || input[0] == '\0') {
        input = ".";
    }

    if (input[0] == '/') {
        copy_string(temp, sizeof(temp), input);
    } else {
        copy_string(temp, sizeof(temp), state->cwd);
        if (!str_eq(temp, "/")) {
            uint64_t len = scadek_strlen(temp);
            if (len + 1u < sizeof(temp)) {
                temp[len] = '/';
                temp[len + 1u] = '\0';
            }
        }
        {
            uint64_t len = scadek_strlen(temp);
            uint64_t i = 0;
            while (input[i] != '\0' && len + 1u < sizeof(temp)) {
                temp[len++] = input[i++];
            }
            temp[len] = '\0';
        }
    }

    out[0] = '/';
    out[1] = '\0';

    while (temp[pos] != '\0') {
        const char *component;
        uint64_t component_len = 0;

        while (temp[pos] == '/') {
            pos++;
        }

        component = &temp[pos];
        while (temp[pos] != '\0' && temp[pos] != '/') {
            component_len++;
            pos++;
        }

        if (component_len == 0u ||
            (component_len == 1u && component[0] == '.')) {
            continue;
        }

        if (component_len == 2u && component[0] == '.' && component[1] == '.') {
            path_pop(out, &out_len);
            continue;
        }

        append_component(out, capacity, &out_len, component, component_len);
    }
}

static char *first_token(char *line, char **rest) {
    char *cursor = line;

    while (*cursor != '\0' && is_space(*cursor)) {
        cursor++;
    }

    if (*cursor == '\0' || *cursor == '#') {
        if (rest != 0) {
            *rest = cursor;
        }
        return 0;
    }

    char *token = cursor;
    while (*cursor != '\0' && !is_space(*cursor)) {
        cursor++;
    }

    if (*cursor != '\0') {
        *cursor++ = '\0';
    }

    while (*cursor != '\0' && is_space(*cursor)) {
        cursor++;
    }

    if (rest != 0) {
        *rest = cursor;
    }

    return token;
}

static void cmd_help(struct runner_state *state) {
    console_line(state, "help version clear pwd cd ls cat run services caps grants rings exit");
}

static void cmd_version(struct runner_state *state) {
    console_write(state, "SCDK: v");
    console_line(state, SCDK_VERSION);
    console_write(state, "ScadekOS: v");
    console_line(state, SCADEKOS_VERSION);
}

static void cmd_pwd(struct runner_state *state) {
    console_line(state, state->cwd);
}

static void cmd_cd(struct runner_state *state, const char *arg) {
    char path[RUNNER_PATH_MAX];
    struct scadek_vfs_stat stat;

    normalize_path(state, arg == 0 || arg[0] == '\0' ? "/" : arg, path, sizeof(path));
    if (scadek_vfs_stat(state->vfs, path, &stat) != SCADEK_OK ||
        stat.type != SCADEK_VFS_NODE_DIR) {
        console_line(state, "cd: not a directory");
        return;
    }

    copy_string(state->cwd, sizeof(state->cwd), path);
}

static void cmd_ls(struct runner_state *state, const char *arg) {
    char path[RUNNER_PATH_MAX];
    struct scadek_vfs_dirent entries[RUNNER_DIRENTS];
    uint64_t count = 0;

    normalize_path(state, arg == 0 || arg[0] == '\0' ? "." : arg, path, sizeof(path));
    if (scadek_vfs_listdir(state->vfs, path, entries, RUNNER_DIRENTS, &count) != SCADEK_OK) {
        console_line(state, "ls: not a directory");
        return;
    }

    for (uint64_t i = 0; i < count; i++) {
        console_write(state, "  ");
        console_write(state, entries[i].name);
        if (entries[i].type == SCADEK_VFS_NODE_DIR) {
            console_write(state, "/");
        }
        console_write(state, "\r\n");
    }
}

static void cmd_cat(struct runner_state *state, const char *arg) {
    char path[RUNNER_PATH_MAX];
    char buffer[RUNNER_READ_MAX + 1u];
    scadek_cap_t file = 0;
    uint64_t size = 0;
    uint64_t offset = 0;

    if (arg == 0 || arg[0] == '\0') {
        console_line(state, "cat: missing path");
        return;
    }

    normalize_path(state, arg, path, sizeof(path));
    if (scadek_vfs_open(state->vfs, path, &file, &size) != SCADEK_OK) {
        console_line(state, "cat: not found");
        return;
    }

    while (offset < size) {
        uint64_t got = 0;
        uint64_t want = size - offset;
        if (want > RUNNER_READ_MAX) {
            want = RUNNER_READ_MAX;
        }

        if (scadek_vfs_read(state->vfs, file, offset, buffer, want, &got) != SCADEK_OK ||
            got == 0u) {
            break;
        }

        buffer[got] = '\0';
        console_write(state, buffer);
        offset += got;
    }

    (void)scadek_vfs_close(state->vfs, file);
    console_write(state, "\r\n");
}

static void cmd_run(struct runner_state *state, const char *arg) {
    char path[RUNNER_PATH_MAX];

    if (arg == 0 || arg[0] == '\0') {
        console_line(state, "run: missing path");
        return;
    }

    normalize_path(state, arg, path, sizeof(path));
    if (scadek_proc_spawn_with_bootstrap(state->proc, path, state->console) != SCADEK_OK) {
        console_line(state, "run: failed");
        return;
    }

    console_write(state, "[proc] ");
    console_write(state, path);
    console_line(state, " exited: 0");
}

static void cmd_services(struct runner_state *state) {
    console_line(state, "console tty vfs proc session");
}

static void cmd_caps(struct runner_state *state) {
    console_line(state, "visible caps: session console tty vfs proc");
}

static void cmd_rings(struct runner_state *state) {
    if (scadek_proc_spawn_with_bootstrap(state->proc, "/ring-test", state->console) != SCADEK_OK) {
        console_line(state, "rings: not supported yet");
        return;
    }

    console_line(state, "[rings] /ring-test submitted grant-backed console batch");
}

static void cmd_grants(struct runner_state *state) {
    if (state->grant_test == 0 ||
        scadek_proc_spawn_with_bootstrap(state->proc, "/grant-test", state->grant_test) != SCADEK_OK) {
        console_line(state, "grants: not supported yet");
        return;
    }

    console_line(state, "[grants] /grant-test completed");
}

static int execute_line(struct runner_state *state, char *line) {
    char *rest = 0;
    char *cmd = first_token(line, &rest);

    if (cmd == 0) {
        return 1;
    }

    if (str_eq_ci(cmd, "help")) {
        cmd_help(state);
    } else if (str_eq_ci(cmd, "version")) {
        cmd_version(state);
    } else if (str_eq_ci(cmd, "clear")) {
        (void)scadek_console_clear(state->console);
    } else if (str_eq_ci(cmd, "pwd")) {
        cmd_pwd(state);
    } else if (str_eq_ci(cmd, "cd")) {
        cmd_cd(state, rest);
    } else if (str_eq_ci(cmd, "ls")) {
        cmd_ls(state, rest);
    } else if (str_eq_ci(cmd, "cat")) {
        cmd_cat(state, rest);
    } else if (str_eq_ci(cmd, "run")) {
        cmd_run(state, rest);
    } else if (str_eq_ci(cmd, "services")) {
        cmd_services(state);
    } else if (str_eq_ci(cmd, "caps")) {
        cmd_caps(state);
    } else if (str_eq_ci(cmd, "rings")) {
        cmd_rings(state);
    } else if (str_eq_ci(cmd, "grants")) {
        cmd_grants(state);
    } else if (str_eq_ci(cmd, "exit")) {
        return 0;
    } else {
        console_line(state, "unknown command");
    }

    return 1;
}

static void run_boot_script(struct runner_state *state) {
    char script[1024];
    char line[RUNNER_LINE_MAX];
    scadek_cap_t file = 0;
    uint64_t size = 0;
    uint64_t got = 0;
    uint64_t pos = 0;

    if (scadek_vfs_open(state->vfs, "/etc/scadek.rc", &file, &size) != SCADEK_OK) {
        return;
    }

    if (size >= sizeof(script)) {
        size = sizeof(script) - 1u;
    }

    if (scadek_vfs_read(state->vfs, file, 0, script, size, &got) != SCADEK_OK) {
        (void)scadek_vfs_close(state->vfs, file);
        return;
    }

    (void)scadek_vfs_close(state->vfs, file);
    script[got] = '\0';

    while (pos < got) {
        uint64_t len = 0;
        char exec_line[RUNNER_LINE_MAX];

        while (pos < got && script[pos] != '\n' && len + 1u < sizeof(line)) {
            if (script[pos] != '\r') {
                line[len++] = script[pos];
            }
            pos++;
        }
        while (pos < got && script[pos] != '\n') {
            pos++;
        }
        if (pos < got && script[pos] == '\n') {
            pos++;
        }

        line[len] = '\0';
        copy_string(exec_line, sizeof(exec_line), line);
        if (first_token(exec_line, 0) == 0) {
            continue;
        }

        console_write(state, "scadek:/> ");
        console_line(state, line);
        copy_string(exec_line, sizeof(exec_line), line);
        (void)execute_line(state, exec_line);
    }
}

static int read_interactive_line(struct runner_state *state,
                                 char *line,
                                 uint64_t capacity) {
    uint64_t pos = 0;

    if (line == 0 || capacity == 0u) {
        return 0;
    }

    for (;;) {
        struct scadek_input_event event;
        scadek_status_t status = scadek_tty_poll_event(state->tty, &event);

        if (status == SCADEK_ERR_NOENT) {
            (void)scadek_yield();
            continue;
        }
        if (status != SCADEK_OK) {
            return 0;
        }
        if (event.type != SCADEK_INPUT_KEY_DOWN) {
            continue;
        }
        if (handle_manual_scroll_key(state, &event)) {
            continue;
        }
        if (event.ascii == '\n' || event.ascii == '\r') {
            line[pos] = '\0';
            console_write(state, "\r\n");
            return 1;
        }
        if (event.ascii == '\b') {
            if (pos > 0u) {
                pos--;
                console_write(state, "\b");
            }
            continue;
        }
        if (event.ascii >= 0x20u && event.ascii < 0x7fu && pos + 1u < capacity) {
            line[pos++] = (char)event.ascii;
            console_write_char(state, (char)event.ascii);
        }
    }
}

static void interactive_loop(struct runner_state *state) {
    char line[RUNNER_LINE_MAX];

    for (;;) {
        console_write(state, "scadek:");
        console_write(state, state->cwd);
        console_write(state, "> ");

        if (!read_interactive_line(state, line, sizeof(line))) {
            console_line(state, "tty: input failed");
            return;
        }

        if (!execute_line(state, line)) {
            return;
        }
    }
}
