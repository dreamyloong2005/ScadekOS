#!/usr/bin/env sh
# SPDX-License-Identifier: MPL-2.0
# Copyright (c) 2026 The Scadek OS Project contributors
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
PAYLOAD_USER_DIR=${1:-"$ROOT/build/payload/user"}
INITRD_ROOT=${2:-"$ROOT/kernel/scdk/build/initrd_root"}
BUILD_USER_DIR="$ROOT/build/user"
LIB_BUILD_DIR="$BUILD_USER_DIR/libscadek"
LIBSCADEK_A="$LIB_BUILD_DIR/libscadek.a"
USER_LOAD_ADDR=0x0000000000400000
SCADEKOS_VERSION=$(sed -n '1p' "$ROOT/VERSION")
SCDK_VERSION=$(sed -n 's/^SCDK_VERSION=//p' "$ROOT/KERNEL_VERSION")
VERSION_HEADER="$BUILD_USER_DIR/generated_version.h"

cd "$ROOT/kernel/scdk"
. ../../tools/kernel-env.sh
cd "$ROOT"

TARGET=${TARGET:-x86_64-elf}
CC=${CC:-"$TARGET-gcc"}
LD=${LD:-"$TARGET-ld"}
AR=${AR:-"$TARGET-ar"}
OBJCOPY=${OBJCOPY:-"$TARGET-objcopy"}

USER_CFLAGS="-std=gnu11 -Os -ffreestanding -fno-builtin -fno-pic -fno-pie -fno-asynchronous-unwind-tables -fno-unwind-tables -Wa,--noexecstack -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -Wall -Wextra -Werror -I$ROOT/userspace/libscadek/include -include $VERSION_HEADER"

mkdir -p "$LIB_BUILD_DIR" "$INITRD_ROOT" "$BUILD_USER_DIR"

{
    printf '#ifndef SCADEK_GENERATED_VERSION_H\n'
    printf '#define SCADEK_GENERATED_VERSION_H\n'
    printf '#define SCADEKOS_VERSION "%s"\n' "$SCADEKOS_VERSION"
    printf '#define SCDK_VERSION "%s"\n' "$SCDK_VERSION"
    printf '#endif\n'
} > "$VERSION_HEADER"

"$CC" $USER_CFLAGS -c "$ROOT/userspace/libscadek/scadek.S" -o "$LIB_BUILD_DIR/scadek.o"
"$CC" $USER_CFLAGS -c "$ROOT/userspace/libscadek/scadek.c" -o "$LIB_BUILD_DIR/scadek_c.o"
"$AR" rcs "$LIBSCADEK_A" "$LIB_BUILD_DIR/scadek.o" "$LIB_BUILD_DIR/scadek_c.o"

build_user() {
    name=$1
    output=$2
    obj="$BUILD_USER_DIR/$name.o"
    elf="$BUILD_USER_DIR/$name.elf"
    bin="$INITRD_ROOT/$output"
    src_c="$PAYLOAD_USER_DIR/$name.c"
    src_s="$PAYLOAD_USER_DIR/$name.S"

    if [ -f "$src_c" ]; then
        "$CC" $USER_CFLAGS -c "$src_c" -o "$obj"
    else
        "$CC" $USER_CFLAGS -c "$src_s" -o "$obj"
    fi
    "$LD" -nostdlib -static -z noexecstack -Ttext="$USER_LOAD_ADDR" -o "$elf" "$obj" "$LIBSCADEK_A"
    mkdir -p "$(dirname "$bin")"
    "$OBJCOPY" -O binary "$elf" "$bin"
}

build_user init init
build_user hello hello
build_user hello bin/hello
build_user grant grant-test
build_user ring ring-test
build_user runner runner
build_user prompt prompt
