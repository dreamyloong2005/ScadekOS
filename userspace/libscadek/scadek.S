/* SPDX-License-Identifier: MPL-2.0 */
/* Copyright (c) 2026 The Scadek OS Project contributors */

.set SCDK_SYS_ENDPOINT_CALL, 1
.set SCDK_SYS_YIELD, 2
.set SCDK_SYS_EXIT, 3
.set SCDK_SYS_GRANT_CREATE, 4
.set SCDK_SYS_GRANT_REVOKE, 5
.set SCDK_SYS_RING_CREATE, 6
.set SCDK_SYS_RING_BIND, 7
.set SCDK_SYS_RING_SUBMIT, 8
.set SCDK_SYS_RING_POLL, 9

.section .text

.global scadek_endpoint_call
.type scadek_endpoint_call, @function
scadek_endpoint_call:
    movq $SCDK_SYS_ENDPOINT_CALL, %rax
    syscall
    ret
.size scadek_endpoint_call, . - scadek_endpoint_call

.global scadek_grant_create
.type scadek_grant_create, @function
scadek_grant_create:
    movq %rcx, %r10
    movq $SCDK_SYS_GRANT_CREATE, %rax
    syscall
    ret
.size scadek_grant_create, . - scadek_grant_create

.global scadek_grant_revoke
.type scadek_grant_revoke, @function
scadek_grant_revoke:
    movq $SCDK_SYS_GRANT_REVOKE, %rax
    syscall
    ret
.size scadek_grant_revoke, . - scadek_grant_revoke

.global scadek_ring_create
.type scadek_ring_create, @function
scadek_ring_create:
    movq $SCDK_SYS_RING_CREATE, %rax
    syscall
    ret
.size scadek_ring_create, . - scadek_ring_create

.global scadek_ring_bind
.type scadek_ring_bind, @function
scadek_ring_bind:
    movq $SCDK_SYS_RING_BIND, %rax
    syscall
    ret
.size scadek_ring_bind, . - scadek_ring_bind

.global scadek_ring_submit
.type scadek_ring_submit, @function
scadek_ring_submit:
    movq $SCDK_SYS_RING_SUBMIT, %rax
    syscall
    ret
.size scadek_ring_submit, . - scadek_ring_submit

.global scadek_ring_poll
.type scadek_ring_poll, @function
scadek_ring_poll:
    movq $SCDK_SYS_RING_POLL, %rax
    syscall
    ret
.size scadek_ring_poll, . - scadek_ring_poll

.global scadek_yield
.type scadek_yield, @function
scadek_yield:
    movq $SCDK_SYS_YIELD, %rax
    syscall
    ret
.size scadek_yield, . - scadek_yield

.global scadek_exit
.type scadek_exit, @function
scadek_exit:
    movq $SCDK_SYS_EXIT, %rax
    syscall
.Lscadek_exit_wait:
    pause
    jmp .Lscadek_exit_wait
.size scadek_exit, . - scadek_exit

.section .note.GNU-stack,"",@progbits
