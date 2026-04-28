/* SPDX-License-Identifier: MPL-2.0 */
/* Copyright (c) 2026 The Scadek OS Project contributors */

#include <scadek/scadek.inc>

.section .text
.global _start
.type _start, @function
_start:
    movq %rdi, %rbx

    leaq grant_payload(%rip), %rdi
    movq $(grant_payload_end - grant_payload), %rsi
    movq $SCADEK_RIGHT_READ, %rdx
    movq %rbx, %rcx
    call scadek_grant_create
    movq %rax, %r12

    subq $SCADEK_MSG_SIZE, %rsp
    movq %rsp, %rsi

    movq $0, SCADEK_MSG_SENDER(%rsi)
    movq $SCADEK_SERVICE_GRANT_TEST, SCADEK_MSG_TARGET(%rsi)
    movq $SCADEK_MSG_READ, SCADEK_MSG_TYPE(%rsi)
    movq %r12, SCADEK_MSG_ARG0(%rsi)
    movq $0, SCADEK_MSG_ARG1(%rsi)
    movq $(grant_payload_end - grant_payload), SCADEK_MSG_ARG2(%rsi)
    movq $1, SCADEK_MSG_ARG3(%rsi)
    movq $0, SCADEK_MSG_STATUS(%rsi)
    movq %rbx, %rdi
    call scadek_endpoint_call

    movq %r12, %rdi
    call scadek_grant_revoke

    movq %rsp, %rsi
    movq $0, SCADEK_MSG_SENDER(%rsi)
    movq $SCADEK_SERVICE_GRANT_TEST, SCADEK_MSG_TARGET(%rsi)
    movq $SCADEK_MSG_READ, SCADEK_MSG_TYPE(%rsi)
    movq %r12, SCADEK_MSG_ARG0(%rsi)
    movq $0, SCADEK_MSG_ARG1(%rsi)
    movq $(grant_payload_end - grant_payload), SCADEK_MSG_ARG2(%rsi)
    movq $2, SCADEK_MSG_ARG3(%rsi)
    movq $0, SCADEK_MSG_STATUS(%rsi)
    movq %rbx, %rdi
    call scadek_endpoint_call

    movq $0, %rdi
    call scadek_exit

.Lhalt:
    pause
    jmp .Lhalt

grant_payload:
    .ascii "grant payload"
grant_payload_end:
.size _start, . - _start

.section .note.GNU-stack,"",@progbits
