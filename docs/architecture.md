# ScadekOS Architecture

ScadekOS is the operating-system layer built around SCDK.

## Project Split

SCDK owns the kernel core:

- boot and hardware entry
- memory management
- scheduling
- object and capability management
- endpoints and messages
- rings and grants
- core kernel services
- syscall and IPC boundary

ScadekOS owns the OS integration layer:

- default initrd contents
- user-space startup policy
- SCDK-native user programs
- service composition
- image packaging
- release documentation

## Interface Contract

ScadekOS must interact with SCDK through explicit SCDK-native interfaces:

- object IDs instead of raw kernel pointers
- capabilities instead of ambient authority
- endpoints and messages for control-plane operations
- rings and grants for data-plane operations

POSIX compatibility is not a native contract. If it appears later, it should be
implemented as a compatibility layer above SCDK-native interfaces.

## Current Integration Model

ScadekOS pins SCDK as `kernel/scdk`, compiles the SCDK kernel core from the
submodule, links ScadekOS-owned flat user programs against `libscadek` from the
top-level build, and then packages the final initrd and ISO at the ScadekOS top
level.

Current packaging flow:

```text
ScadekOS initrd/userspace/configs
  -> SCDK kernel build rules + ScadekOS libscadek user build
  -> ScadekOS initrd/ISO packager
    -> build/scadekos.iso
```

This keeps the submodule source tree clean while letting ScadekOS own `/init`,
`/hello`, `/grant-test`, `/ring-test`, the minimal `libscadek` user ABI
library, and OS-level configuration.

## M24 libscadek Shape

SCDK M24 flat user programs receive a single bootstrap endpoint capability.
ScadekOS keeps `libscadek` deliberately small and SCDK-native:

- endpoint calls are explicit message sends through `scadek_endpoint_call()`
- grants are created and revoked through `scadek_grant_create()` and
  `scadek_grant_revoke()`
- rings are created, bound, submitted, and polled through the
  `scadek_ring_*()` calls
- payload access for ring console writes goes through grants
- `scadek_yield()` and `scadek_exit()` wrap the minimal task-control syscalls

This is a helper library for current demos, not a libc or POSIX ABI.
