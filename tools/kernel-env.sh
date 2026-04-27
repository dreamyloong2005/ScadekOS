#!/usr/bin/env sh
# SPDX-License-Identifier: MPL-2.0
# Copyright (c) 2026 The Scadek OS Project contributors

if [ -n "${SCADEKOS_SCDK_DEVTOOLS:-}" ]; then
    SCDK_DEVTOOLS=$SCADEKOS_SCDK_DEVTOOLS
    export SCDK_DEVTOOLS
    export PATH="$SCDK_DEVTOOLS/cross/bin:$SCDK_DEVTOOLS/limine:$PATH"
else
    . .devtools/env.sh
fi
