#!/usr/bin/env sh
# SPDX-License-Identifier: MPL-2.0
# Copyright (c) 2026 The Scadek OS Project contributors
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
LOG=${1:-"$ROOT/build/scadekos-keyboard.log"}
MONITOR_LOG=${SCADEKOS_KEYBOARD_MONITOR_LOG:-"$ROOT/build/scadekos-keyboard-monitor.log"}
TIMEOUT=${SCADEKOS_QEMU_TIMEOUT:-16s}

cd "$ROOT"
mkdir -p build
rm -f "$LOG" "$MONITOR_LOG"

make iso >"$MONITOR_LOG" 2>&1

status=0
(
    sleep 8
    printf 'sendkey h\n'
    sleep 0.1
    printf 'sendkey e\n'
    sleep 0.1
    printf 'sendkey l\n'
    sleep 0.1
    printf 'sendkey p\n'
    sleep 0.1
    printf 'sendkey ret\n'
    sleep 1
    printf 'quit\n'
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

require_log "[scadekos] interactive console ready"
require_log "scadek:/> help"
require_log "help version clear pwd cd ls cat run services caps grants rings exit"

printf '[scadekos] keyboard smoke pass: %s\n' "$LOG"
