#!/usr/bin/env sh
# SPDX-License-Identifier: MPL-2.0
# Copyright (c) 2026 The Scadek OS Project contributors
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
LOG=${1:-"$ROOT/build/scadekos-no-serial.log"}
SHOT=${SCADEKOS_NO_SERIAL_SCREENSHOT:-"$ROOT/build/scadekos-no-serial.ppm"}
BOOT_DELAY=${SCADEKOS_NO_SERIAL_DELAY:-5.3}
TIMEOUT=${SCADEKOS_QEMU_TIMEOUT:-14s}

cd "$ROOT"
mkdir -p build
rm -f "$LOG" "$SHOT"

make iso >"$LOG" 2>&1

status=0
(
    sleep "$BOOT_DELAY"
    printf 'screendump %s\n' "$SHOT"
    printf 'quit\n'
) | timeout "$TIMEOUT" qemu-system-x86_64 \
        -M q35 \
        -m 256M \
        -cdrom "$ROOT/build/scadekos.iso" \
        -boot d \
        -serial none \
        -display none \
        -monitor stdio \
        -no-reboot >>"$LOG" 2>&1 || status=$?

if [ "$status" -ne 0 ] && [ "$status" -ne 124 ]; then
    cat "$LOG"
    exit "$status"
fi

if [ ! -s "$SHOT" ]; then
    printf 'missing framebuffer screenshot: %s\n' "$SHOT" >&2
    printf 'log file: %s\n' "$LOG" >&2
    exit 1
fi

bytes=$(wc -c < "$SHOT")
if [ "$bytes" -le 4096 ]; then
    printf 'framebuffer screenshot too small: %s bytes\n' "$bytes" >&2
    printf 'screenshot: %s\n' "$SHOT" >&2
    exit 1
fi

unique_values=$(tail -c +128 "$SHOT" | od -An -tu1 | tr ' ' '\n' | sed '/^$/d' | sort -u | wc -l)
if [ "$unique_values" -le 2 ]; then
    printf 'framebuffer screenshot appears blank: %s unique byte values\n' "$unique_values" >&2
    printf 'screenshot: %s\n' "$SHOT" >&2
    exit 1
fi

printf '[scadekos] no-serial framebuffer smoke pass: %s\n' "$SHOT"
printf '[scadekos] inspect screenshot for prompt: scadek:/>\n'
