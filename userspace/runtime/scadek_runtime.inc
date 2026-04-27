/* SPDX-License-Identifier: MPL-2.0 */
/* Copyright (c) 2026 The Scadek OS Project contributors */

.set SCDK_SYS_ENDPOINT_CALL, 1
.set SCDK_SYS_EXIT, 3
.set SCDK_SYS_GRANT_CREATE, 4
.set SCDK_SYS_GRANT_REVOKE, 5
.set SCDK_SYS_RING_CREATE, 6
.set SCDK_SYS_RING_BIND, 7
.set SCDK_SYS_RING_SUBMIT, 8
.set SCDK_SYS_RING_POLL, 9

.set SCDK_SERVICE_CONSOLE, 1
.set SCDK_SERVICE_PROC, 4
.set SCDK_SERVICE_GRANT_TEST, 5

.set SCDK_MSG_READ, 3
.set SCDK_MSG_WRITE, 4
.set SCDK_MSG_PROCESS_SPAWN, 9

.set SCDK_RIGHT_READ, 1

.set SCDK_RING_OP_CONSOLE_WRITE, 1
.set SCDK_RING_BATCH, 16
.set SCDK_RING_DESC_SIZE, 48
.set SCDK_RING_COMPLETION_SIZE, 32

.set SCADEK_MSG_SENDER, 0
.set SCADEK_MSG_TARGET, 8
.set SCADEK_MSG_TYPE, 16
.set SCADEK_MSG_ARG0, 24
.set SCADEK_MSG_ARG1, 32
.set SCADEK_MSG_ARG2, 40
.set SCADEK_MSG_ARG3, 48
.set SCADEK_MSG_STATUS, 56
.set SCADEK_MSG_SIZE, 64

.macro scadek_exit
    movq $SCDK_SYS_EXIT, %rax
    syscall
.endm

.macro scadek_endpoint_call endpoint, message
    movq $SCDK_SYS_ENDPOINT_CALL, %rax
    movq \endpoint, %rdi
    movq \message, %rsi
    syscall
.endm

.macro scadek_message_clear message
    movq $0, SCADEK_MSG_SENDER(\message)
    movq $0, SCADEK_MSG_TARGET(\message)
    movq $0, SCADEK_MSG_TYPE(\message)
    movq $0, SCADEK_MSG_ARG0(\message)
    movq $0, SCADEK_MSG_ARG1(\message)
    movq $0, SCADEK_MSG_ARG2(\message)
    movq $0, SCADEK_MSG_ARG3(\message)
    movq $0, SCADEK_MSG_STATUS(\message)
.endm

.macro scadek_proc_spawn message, path, path_len
    scadek_message_clear \message
    movq $SCDK_SERVICE_PROC, SCADEK_MSG_TARGET(\message)
    movq $SCDK_MSG_PROCESS_SPAWN, SCADEK_MSG_TYPE(\message)
    leaq \path(%rip), %rax
    movq %rax, SCADEK_MSG_ARG0(\message)
    movq $\path_len, SCADEK_MSG_ARG1(\message)
.endm

.macro scadek_console_write message, text, text_len
    scadek_message_clear \message
    movq $SCDK_SERVICE_CONSOLE, SCADEK_MSG_TARGET(\message)
    movq $SCDK_MSG_WRITE, SCADEK_MSG_TYPE(\message)
    leaq \text(%rip), %rax
    movq %rax, SCADEK_MSG_ARG0(\message)
    movq $\text_len, SCADEK_MSG_ARG1(\message)
.endm

.macro scadek_grant_create_read payload, payload_len, target
    movq $SCDK_SYS_GRANT_CREATE, %rax
    leaq \payload(%rip), %rdi
    movq $\payload_len, %rsi
    movq $SCDK_RIGHT_READ, %rdx
    movq \target, %r10
    syscall
.endm

.macro scadek_grant_revoke grant
    movq $SCDK_SYS_GRANT_REVOKE, %rax
    movq \grant, %rdi
    syscall
.endm

.macro scadek_ring_create entries
    movq $SCDK_SYS_RING_CREATE, %rax
    movq $\entries, %rdi
    syscall
.endm

.macro scadek_ring_bind ring, endpoint
    movq $SCDK_SYS_RING_BIND, %rax
    movq \ring, %rdi
    movq \endpoint, %rsi
    syscall
.endm

.macro scadek_ring_submit ring, descs, count
    movq $SCDK_SYS_RING_SUBMIT, %rax
    movq \ring, %rdi
    movq \descs, %rsi
    movq $\count, %rdx
    syscall
.endm
.macro scadek_ring_poll ring, completions, count
    movq $SCDK_SYS_RING_POLL, %rax
    movq \ring, %rdi
    movq \completions, %rsi
    movq $\count, %rdx
    syscall
.endm
