/* SPDX-License-Identifier: MPL-2.0 */
/* Copyright (c) 2026 The Scadek OS Project contributors */

#include "@SCADEK_RUNTIME_INC@"

.section .text
.global _start
.type _start, @function
_start:
    movq %rdi, %rbx

    scadek_grant_create_read grant_payload, (grant_payload_end - grant_payload), %rbx
    movq %rax, %r12

    subq $SCADEK_MSG_SIZE, %rsp
    movq %rsp, %rsi

    scadek_message_clear %rsi
    movq $SCDK_SERVICE_GRANT_TEST, SCADEK_MSG_TARGET(%rsi)
    movq $SCDK_MSG_READ, SCADEK_MSG_TYPE(%rsi)
    movq %r12, SCADEK_MSG_ARG0(%rsi)
    movq $0, SCADEK_MSG_ARG1(%rsi)
    movq $(grant_payload_end - grant_payload), SCADEK_MSG_ARG2(%rsi)
    movq $1, SCADEK_MSG_ARG3(%rsi)
    scadek_endpoint_call %rbx, %rsi

    scadek_grant_revoke %r12

    movq %rsp, %rsi
    scadek_message_clear %rsi
    movq $SCDK_SERVICE_GRANT_TEST, SCADEK_MSG_TARGET(%rsi)
    movq $SCDK_MSG_READ, SCADEK_MSG_TYPE(%rsi)
    movq %r12, SCADEK_MSG_ARG0(%rsi)
    movq $0, SCADEK_MSG_ARG1(%rsi)
    movq $(grant_payload_end - grant_payload), SCADEK_MSG_ARG2(%rsi)
    movq $2, SCADEK_MSG_ARG3(%rsi)
    scadek_endpoint_call %rbx, %rsi

    scadek_exit

.Lhalt:
    hlt
    jmp .Lhalt

grant_payload:
    .ascii "grant payload"
grant_payload_end:
.size _start, . - _start

.section .note.GNU-stack,"",@progbits
