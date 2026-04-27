# ScadekOS Userspace

This directory is for ScadekOS-owned user programs.

The current flat binaries are:

- `/init`: starts ScadekOS user programs through the SCDK proc service.
- `/hello`: basic console endpoint/message demo.
- `/grant-test`: user-visible grant demo used by SCDK M24 self-tests.
- `/ring-test`: user-visible ring + grant data-plane demo.

Shared assembly helpers live in `runtime/`. This runtime is SCDK-native and is
not a libc or POSIX layer.
