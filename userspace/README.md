# ScadekOS Userspace

This directory is for ScadekOS-owned user programs.

The current flat binaries are:

- `/init`: starts ScadekOS user programs through the SCDK proc service.
- `/hello`: basic console endpoint/message demo.
- `/grant-test`: user-visible grant demo used by SCDK M24 self-tests.
- `/ring-test`: user-visible ring + grant data-plane demo.

The current flat binaries link against `libscadek`, a tiny SCDK-native user ABI
library. It is not a libc or POSIX layer.
