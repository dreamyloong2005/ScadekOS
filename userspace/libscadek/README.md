# libscadek

`libscadek` is the small ScadekOS-owned user ABI library for current SCDK flat
binary programs.

It wraps the M30 SCDK-native syscall surface used by the demos:

- endpoint calls
- user grants
- user rings
- grant-backed console writes
- TTY polling helpers for programs that receive a TTY endpoint
- proc spawn requests
- yield
- exit

This is not a libc, POSIX layer, shell runtime, allocator, or package-management
interface.
