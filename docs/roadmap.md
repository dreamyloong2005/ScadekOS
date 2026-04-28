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
- Status: implemented since the initial repository split using top-level initrd
  and ISO packaging.

## Stage 2: SCDK-Native System Startup

- Replace demo `/init` with ScadekOS-owned `/init`.
- Define a ScadekOS boot configuration format.
- Start services through SCDK-native proc, VFS, endpoint, and capability paths.
- Status: `/init` spawns `/runner`, `/bin/hello`, `/ring-test`, and `/prompt`
  through proc service on SCDK M30.

## Stage 3: Grant/Ring libscadek Demos

- Add a small SCDK-native `libscadek` user ABI library.
- Add `/grant-test` for user-visible grant create/read/revoke validation.
- Add `/ring-test` for grant-backed ring descriptor submission and completion polling.
- Validate grant/ring logs in the smoke test.
- Status: implemented for the current SCDK M30 devpreview path.

## Stage 4: Service Policy

- Define default capability distribution.
- Define service launch order.
- Keep tmpfs, VFS, proc, console, and future devmgr interactions message-based.
- Status: devpreview.3 uses a deterministic `/etc/scadek.rc` preview runner and
  validates no-serial framebuffer boot with `make smoke-no-serial`.

## Stage 5: Release Discipline

- Depend on tagged SCDK kernel releases.
- Keep ScadekOS releases tied to explicit SCDK commits or tags.
- Add boot-log based smoke tests for supported milestones.
