#!/usr/bin/env sh
# SPDX-License-Identifier: MPL-2.0
# Copyright (c) 2026 The Scadek OS Project contributors
set -eu

ROOT=$(CDPATH= cd -- "$(dirname "$0")/.." && pwd)
cd "$ROOT"

exec make run
