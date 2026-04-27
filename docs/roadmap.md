# ScadekOS Roadmap

## Stage 0: Repository Split

- Create ScadekOS as a separate repository.
- Pin SCDK as a Git submodule at `kernel/scdk`.
- Add top-level build and run wrappers.
- Document ownership boundaries.
- Add boot-log smoke testing.
- Maintain ScadekOS version in the top-level `VERSION` file.
- Maintain pinned SCDK identity in the top-level `KERNEL_VERSION` file.

## Stage 1: External Payload Build

- Let ScadekOS provide initrd files outside the SCDK submodule.
- Let ScadekOS provide user programs outside the SCDK submodule.
- Keep SCDK responsible for the kernel and image construction primitives.
- Produce `build/scadekos.iso` without editing kernel-owned payload stubs.
- Status: implemented for M0 using top-level initrd and ISO packaging.

## Stage 2: SCDK-Native System Startup

- Replace demo `/init` with ScadekOS-owned `/init`.
- Define a ScadekOS boot configuration format.
- Start services through SCDK-native proc, VFS, endpoint, and capability paths.
- Status: initial `/init` spawns `/hello` through proc service on SCDK M22.

## Stage 3: Service Policy

- Define default capability distribution.
- Define service launch order.
- Keep tmpfs, VFS, proc, console, and future devmgr interactions message-based.

## Stage 4: Release Discipline

- Depend on tagged SCDK kernel releases.
- Keep ScadekOS releases tied to explicit SCDK commits or tags.
- Add boot-log based smoke tests for supported milestones.
