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

Current version: `0.1.0-devpreview.4`
Kernel version: `SCDK 0.4.0-alpha.4`
Kernel commit: `2f59d1f32b4aca1dec24612783b537915e60375b`

This repository currently implements **ScadekOS devpreview.3 over the SCDK M31
kernel/OS parallel baseline**.

The top-level build uses the pinned SCDK submodule for the kernel core, then
packages ScadekOS-owned payloads into the boot initrd:

- `userspace/init/init.c` becomes `/init`
- `userspace/hello/hello.c` becomes `/hello` and `/bin/hello`
- `userspace/grant/grant.S` becomes `/grant-test`
- `userspace/ring/ring.S` becomes `/ring-test`
- `userspace/runner/runner.c` becomes `/runner`
- `userspace/prompt/prompt.c` becomes `/prompt`
- `userspace/libscadek/` provides the minimal SCDK-native user ABI library
- `initrd/etc/scdk.conf` remains for SCDK boot-configuration compatibility
- `initrd/etc/scadekos.conf` is the ScadekOS boot policy manifest
- `initrd/etc/scadek.rc` is the deterministic devpreview boot script
- `VERSION` is packaged as `/etc/scadekos.version`
- `KERNEL_VERSION` is packaged as `/etc/scdk.version`
- `initrd/hello.txt` is the VFS/tmpfs smoke-test file

The M30 flat-user ABI still passes one bootstrap endpoint capability to a flat
user program. ScadekOS `/init` receives the proc endpoint and spawns `/runner`,
`/bin/hello`, `/ring-test`, and `/prompt`. Programs spawned through proc receive
the console endpoint and use grant-backed console messages rather than direct
serial or framebuffer access.

The devpreview.3 boot path makes these SCDK properties visible:

- local framebuffer text output before ScadekOS `/init`
- ScadekOS command-runner output and prompt through the console endpoint
- user grant creation/revoke and grant-backed console writes
- ring creation, endpoint binding, 16 submissions, and 16 completions
- capability revoke rejection markers
- user fault isolation without taking down the kernel
- timer/preemption markers
- device-manager fake device and queue authorization markers
- M30 architecture-review completion markers retained under the M31 path

`libscadek` currently wraps endpoint calls, grant create/revoke, ring
create/bind/submit/poll, console writes, TTY polling helpers, proc spawn,
yield, and exit for flat user programs. It is not a libc, POSIX layer, shell
runtime, allocator, or package manager.

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

To boot without a configured serial port:

```sh
make run-framebuffer
```

Serial is an optional debug sink. Normal ScadekOS console output goes through
the SCDK console endpoint and appears on the framebuffer text console.

## Smoke Test

```sh
SCADEKOS_SCDK_DEVTOOLS=/home/taosiyuan/dev/SCDK/.devtools make smoke
```

The smoke test boots QEMU briefly, captures serial output in
`build/scadekos-boot.log`, and verifies devpreview.3 plus M31 markers:

```text
[console] framebuffer text backend ok
[tty] input event path pass
[loader] loading /init
[proc] spawn /runner
[scadekos] command runner started
scadek:/> version
[proc] spawn /bin/hello
[hello] hello from ScadekOS
[grant] user read grant pass
[revoke] cap revoke pass
[fault] task killed
[timer] tick ok
[devmgr] fake device registered
[m30] architecture review complete
[scadekos] libscadek ring console write
[ring] completion batch 16 pass
[boot] milestone 30 complete
```

The no-serial framebuffer smoke test boots QEMU with `-serial none`, captures a
framebuffer screenshot, and checks that the screen is nonblank:

```sh
SCADEKOS_SCDK_DEVTOOLS=/home/taosiyuan/dev/SCDK/.devtools make smoke-no-serial
```

The screenshot is written to:

```text
build/scadekos-no-serial.ppm
```

It is captured during the prompt window and should visibly include:

```text
scadek:/>
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
