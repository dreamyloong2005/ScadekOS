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

cd "$ROOT/kernel/scdk"
. ../../tools/kernel-env.sh
cd "$ROOT"

TARGET=${TARGET:-x86_64-elf}
CC=${CC:-"$TARGET-gcc"}
LD=${LD:-"$TARGET-ld"}
AR=${AR:-"$TARGET-ar"}
OBJCOPY=${OBJCOPY:-"$TARGET-objcopy"}

USER_CFLAGS="-ffreestanding -fno-builtin -fno-pic -fno-pie -m64 -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -Wall -Wextra -Werror -I$ROOT/userspace/libscadek/include"

mkdir -p "$LIB_BUILD_DIR" "$INITRD_ROOT" "$BUILD_USER_DIR"

"$CC" $USER_CFLAGS -c "$ROOT/userspace/libscadek/scadek.S" -o "$LIB_BUILD_DIR/scadek.o"
"$AR" rcs "$LIBSCADEK_A" "$LIB_BUILD_DIR/scadek.o"

build_user() {
    name=$1
    output=$2
    src="$PAYLOAD_USER_DIR/$name.S"
    obj="$BUILD_USER_DIR/$name.o"
    elf="$BUILD_USER_DIR/$name.elf"
    bin="$INITRD_ROOT/$output"

    "$CC" $USER_CFLAGS -c "$src" -o "$obj"
    "$LD" -nostdlib -static -Ttext="$USER_LOAD_ADDR" -o "$elf" "$obj" "$LIBSCADEK_A"
    "$OBJCOPY" -O binary -j .text "$elf" "$bin"
}

build_user init init
build_user hello hello
build_user grant grant-test
build_user ring ring-test
