# ScadekOS

ScadekOS is an experimental operating-system project built around the SCDK
kernel core.

SCDK means **Sharded Capability Dataplane Kernel**. ScadekOS keeps SCDK as a
separate Git submodule so the kernel core and OS integration layer can evolve
with clear ownership.

## Layout

```text
ScadekOS/
  kernel/scdk/      SCDK submodule pinned to a reviewed kernel commit
  initrd/           ScadekOS-owned default initrd payloads
  userspace/        ScadekOS-owned user programs and libscadek work
  services/         ScadekOS service composition and policy stubs
  configs/          OS-level configuration
  docs/             architecture and roadmap notes
  tools/            build and run wrappers
```

## Current State

Current version: `0.1.0-dev.3`
Kernel version: `SCDK 0.3.0-alpha.2`
Kernel commit: `6d617e2bef829fc0e87d0b567c2081c21962e123`

This repository currently implements **ScadekOS dev.3: minimal libscadek over
the SCDK M24 grant/ring baseline**.

The top-level build uses the pinned SCDK submodule for the kernel core, then
packages ScadekOS-owned payloads into the boot initrd:

- `userspace/init/init.S` becomes `/init`
- `userspace/hello/hello.S` becomes `/hello`
- `userspace/grant/grant.S` becomes `/grant-test`
- `userspace/ring/ring.S` becomes `/ring-test`
- `userspace/libscadek/` provides the minimal SCDK-native user ABI library
- `initrd/etc/scdk.conf` remains for SCDK boot-configuration compatibility
- `initrd/etc/scadekos.conf` is the ScadekOS boot policy placeholder
- `VERSION` is packaged as `/etc/scadekos.version`
- `KERNEL_VERSION` is packaged as `/etc/scdk.version`
- `initrd/hello.txt` is the VFS/tmpfs smoke-test file

The M24 user ABI still passes one bootstrap endpoint capability to a flat user
program. ScadekOS `/init` receives the proc endpoint and spawns `/hello` and
`/ring-test`. The kernel M24 self-tests load `/grant-test` through the
grant-test endpoint and `/ring-test` through the console endpoint.

The grant/ring demos cover:

- read-only user grant creation
- service-side grant reads without raw user pointers
- write denial, bounds rejection, and revoke validation
- ring creation and endpoint binding
- 16 descriptor submissions
- 16 completion polls

`libscadek` currently wraps endpoint calls, grant create/revoke, ring
create/bind/submit/poll, yield, and exit for flat user programs. It is not a
libc, POSIX layer, shell runtime, allocator, or package manager.

## Build

Initialize submodules:

```sh
git submodule update --init --recursive
```

Check the SCDK toolchain environment:

```sh
make check-kernel-env
```

On a fresh checkout, build the SCDK development tools from the submodule if the
check reports missing `limine` or `x86_64-elf-*` tools:

```sh
cd kernel/scdk
.devtools/build-cross-toolchain.sh
cd ../..
```

To reuse an existing SCDK toolchain on the same machine:

```sh
SCADEKOS_SCDK_DEVTOOLS=/home/taosiyuan/dev/SCDK/.devtools make check-kernel-env
SCADEKOS_SCDK_DEVTOOLS=/home/taosiyuan/dev/SCDK/.devtools make iso
```

Build the ISO:

```sh
make iso
```

The top-level image is copied to:

```text
build/scadekos.iso
```

The build also prints the ScadekOS version from `VERSION` and the pinned SCDK
kernel identity from `KERNEL_VERSION`.

## Run

```sh
make run
```

This uses QEMU and COM1 serial output, matching the SCDK development loop.

## Smoke Test

```sh
SCADEKOS_SCDK_DEVTOOLS=/home/taosiyuan/dev/SCDK/.devtools make smoke
```

The smoke test boots QEMU briefly, captures serial output in
`build/scadekos-boot.log`, and verifies these dev.3 markers:

```text
[initrd] file: /etc/scadekos.conf
[initrd] file: /etc/scdk.version
[initrd] file: /grant-test
[initrd] file: /ring-test
[loader] loading /init
[proc] spawn /hello
[scadekos] version 0.1.0-dev.3
[scadekos] hello from /hello
[grant] user read grant pass
[grant] revoke pass
[scadekos] libscadek ring console write
[ring] submit batch 16
[ring] completion batch 16 pass
[boot] scadekos dev.3 complete
[boot] milestone 24 complete
```

## Kernel Submodule

The SCDK submodule is located at:

```text
kernel/scdk
```

Update it intentionally:

```sh
cd kernel/scdk
git fetch origin
git checkout <reviewed-commit-or-tag>
cd ../..
git add kernel/scdk
```

Avoid ad hoc kernel edits in this repository unless the work is explicitly a
coordinated SCDK submodule update.
