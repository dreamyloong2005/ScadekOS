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
require_log "[initrd] file: /bin/hello"
require_log "[initrd] file: /grant-test"
require_log "[initrd] file: /ring-test"
require_log "[initrd] file: /runner"
require_log "[initrd] file: /prompt"
require_log "[initrd] file: /etc/scdk.conf"
require_log "[initrd] file: /etc/scadekos.conf"
require_log "[initrd] file: /etc/scadek.rc"
require_log "[initrd] file: /etc/scadekos.version"
require_log "[initrd] file: /etc/scdk.version"
require_log "[console] framebuffer text backend ok"
require_log "[tty] service endpoint registered"
require_log "[tty] input event path pass"
require_log "[loader] loading /init"
require_log "[proc] spawn /runner"
require_log "[scadekos] command runner started"
require_log "scadek:/> version"
require_log "SCDK: v0.4.0-alpha.1"
require_log "ScadekOS: v0.1.0-devpreview.1"
require_log "scadek:/> run /bin/hello"
require_log "[proc] spawn /bin/hello"
require_log "[proc] spawn /ring-test"
require_log "[proc] spawn /prompt"
require_log "[scadekos] version 0.1.0-devpreview.1"
require_log "[hello] hello from ScadekOS"
require_log "scadek:/>"
require_log "[loader] loading /grant-test"
require_log "[grant] user read grant pass"
require_log "[grant] write denied pass"
require_log "[grant] bounds reject pass"
require_log "[grant] revoke pass"
require_log "[revoke] cap revoke pass"
require_log "[revoke] stale generation reject pass"
require_log "[fault] user page fault"
require_log "[fault] task killed"
require_log "[timer] init ok"
require_log "[timer] tick ok"
require_log "[devmgr] service started"
require_log "[devmgr] fake device registered"
require_log "[m30] architecture review complete"
require_log "[loader] loading /ring-test"
require_log "[scadekos] libscadek ring console write"
require_log "[ring] user ring create pass"
require_log "[ring] submit batch 16"
require_log "[ring] completion batch 16 pass"
require_log "[test] all core tests passed"
require_log "[boot] milestone 30 complete"

printf '[scadekos] smoke pass: %s\n' "$LOG"
