#!/usr/bin/env sh
# SPDX-License-Identifier: MPL-2.0
# Copyright (c) 2026 The Scadek OS Project contributors
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
LOG=${1:-"$ROOT/build/scadekos-boot.log"}
TIMEOUT=${SCADEKOS_QEMU_TIMEOUT:-12s}

cd "$ROOT"
mkdir -p build
rm -f "$LOG"

status=0
timeout "$TIMEOUT" make run >"$LOG" 2>&1 || status=$?
if [ "$status" -ne 0 ] && [ "$status" -ne 124 ]; then
    cat "$LOG"
    exit "$status"
fi

require_log() {
    pattern=$1
    if ! grep -F "$pattern" "$LOG" >/dev/null 2>&1; then
        printf 'missing boot log: %s\n' "$pattern" >&2
        printf 'log file: %s\n' "$LOG" >&2
        exit 1
    fi
}

require_log "[initrd] file: /init"
require_log "[initrd] file: /hello"
require_log "[initrd] file: /etc/scdk.conf"
require_log "[initrd] file: /etc/scadekos.conf"
require_log "[initrd] file: /etc/scadekos.version"
require_log "[initrd] file: /etc/scdk.version"
require_log "[loader] loading /init"
require_log "[proc] spawn /hello"
require_log "[scadekos] version 0.1.0-dev.1"
require_log "[scadekos] hello from /hello"
require_log "[boot] scadekos m0 complete"
require_log "[boot] milestone 22 complete"

printf '[scadekos] smoke pass: %s\n' "$LOG"
