# ScadekOS User Runtime

This directory contains the small SCDK-native assembly runtime used by the
current flat-binary userspace demos.

It is intentionally minimal: constants, message layout offsets, and syscall
macros for endpoint calls, grants, and rings. It is not a libc or POSIX layer.
