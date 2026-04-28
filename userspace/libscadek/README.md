# libscadek

`libscadek` is the small ScadekOS-owned user ABI library for current SCDK flat
binary programs.

It wraps the M24 SCDK-native syscall surface used by the demos:

- endpoint calls
- user grants
- user rings
- yield
- exit

This is not a libc, POSIX layer, shell runtime, allocator, or package-management
interface.
