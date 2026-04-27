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
  userspace/        ScadekOS-owned user programs and runtime work
  services/         ScadekOS service composition and policy stubs
  configs/          OS-level configuration
  docs/             architecture and roadmap notes
  tools/            build and run wrappers
```

## Current State

Current version: `0.1.0-dev.1`
Kernel version: `SCDK 0.3.0-alpha.1`
Kernel commit: `c6b018b2962714cac9f400fd6a690db032e70735`

This repository currently implements **ScadekOS M0: SCDK M22 integration
baseline**.

The top-level build uses the pinned SCDK submodule for the kernel core, then
packages ScadekOS-owned payloads into the boot initrd:

- `userspace/init/init.S` becomes `/init`
- `userspace/hello/hello.S` becomes `/hello`
- `initrd/etc/scdk.conf` remains for SCDK M22 compatibility
- `initrd/etc/scadekos.conf` is the ScadekOS boot policy placeholder
- `VERSION` is packaged as `/etc/scadekos.version`
- `KERNEL_VERSION` is packaged as `/etc/scdk.version`
- `initrd/hello.txt` is the VFS/tmpfs smoke-test file

The M22 user ABI passes one bootstrap endpoint capability to a flat user
program. ScadekOS `/init` receives the proc endpoint and spawns `/hello`.
`/hello` receives the console endpoint through the proc/loader path and prints
the ScadekOS boot marker.

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
`build/scadekos-boot.log`, and verifies these M0 markers:

```text
[initrd] file: /etc/scadekos.conf
[initrd] file: /etc/scdk.version
[loader] loading /init
[proc] spawn /hello
[scadekos] version 0.1.0-dev.1
[scadekos] hello from /hello
[boot] scadekos m0 complete
[boot] milestone 22 complete
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
