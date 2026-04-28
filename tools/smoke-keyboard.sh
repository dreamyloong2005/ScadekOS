#!/usr/bin/env sh
# SPDX-License-Identifier: MPL-2.0
# Copyright (c) 2026 The Scadek OS Project contributors
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
LOG=${1:-"$ROOT/build/scadekos-keyboard.log"}
MONITOR_LOG=${SCADEKOS_KEYBOARD_MONITOR_LOG:-"$ROOT/build/scadekos-keyboard-monitor.log"}
TIMEOUT=${SCADEKOS_QEMU_TIMEOUT:-24s}

cd "$ROOT"
mkdir -p build
rm -f "$LOG" "$MONITOR_LOG"

make iso >"$MONITOR_LOG" 2>&1

status=0
(
    sleep 8
    send_key() {
        printf 'sendkey %s\n' "$1"
        sleep 0.25
    }

    send_command() {
        for key in "$@"; do
            send_key "$key"
        done
        sleep 0.5
        printf 'sendkey ret\n'
        sleep 1
    }

    send_key caps_lock
    send_command h e l p
    send_key caps_lock
    send_command v e r s i o n
    send_command h e l p
    send_key pgup
    send_key up
    send_key down
    send_key pgdn
    sleep 2
    printf 'quit\n' 2>/dev/null || true
) | timeout "$TIMEOUT" qemu-system-x86_64 \
        -M q35 \
        -m 256M \
        -cdrom "$ROOT/build/scadekos.iso" \
        -boot d \
        -serial "file:$LOG" \
        -display none \
        -monitor stdio \
        -no-reboot >>"$MONITOR_LOG" 2>&1 || status=$?

if [ "$status" -ne 0 ] && [ "$status" -ne 124 ]; then
    cat "$MONITOR_LOG"
    exit "$status"
fi

require_log() {
    pattern=$1
    if ! grep -F "$pattern" "$LOG" >/dev/null 2>&1; then
        printf 'missing keyboard log: %s\n' "$pattern" >&2
        printf 'serial log: %s\n' "$LOG" >&2
        printf 'monitor log: %s\n' "$MONITOR_LOG" >&2
        exit 1
    fi
}

require_line() {
    pattern=$1
    if ! tr -d '\r' < "$LOG" | grep -Fx "$pattern" >/dev/null 2>&1; then
        printf 'missing exact keyboard log line: %s\n' "$pattern" >&2
        printf 'serial log: %s\n' "$LOG" >&2
        printf 'monitor log: %s\n' "$MONITOR_LOG" >&2
        exit 1
    fi
}

require_log "[scadekos] interactive console ready"
require_line "scadek:/> HELP"
require_log "help version clear pwd cd ls cat run services caps grants rings exit"
require_line "scadek:/> version"
require_log "SCDK: v0.4.0-alpha.4"
require_log "ScadekOS: v0.1.0-devpreview.4"
require_line "scadek:/> help"

lower_help_count=$(grep -F "scadek:/> help" "$LOG" | wc -l)
upper_help_count=$(grep -F "scadek:/> HELP" "$LOG" | wc -l)
if [ "$lower_help_count" -lt 1 ] || [ "$upper_help_count" -lt 1 ]; then
    printf 'missing uppercase/lowercase help prompts: upper=%s lower=%s\n' \
        "$upper_help_count" "$lower_help_count" >&2
    printf 'serial log: %s\n' "$LOG" >&2
    printf 'monitor log: %s\n' "$MONITOR_LOG" >&2
    exit 1
fi

printf '[scadekos] keyboard smoke pass: %s\n' "$LOG"
