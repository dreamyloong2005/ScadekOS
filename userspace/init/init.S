/* SPDX-License-Identifier: MPL-2.0 */
/* Copyright (c) 2026 The Scadek OS Project contributors */

#include <scadek/scadek.inc>

.section .text
.global _start
.type _start, @function
_start:
    movq %rdi, %rbx

    subq $SCADEK_MSG_SIZE, %rsp
    movq %rsp, %rsi

    movq $0, SCADEK_MSG_SENDER(%rsi)
    movq $SCADEK_SERVICE_PROC, SCADEK_MSG_TARGET(%rsi)
    movq $SCADEK_MSG_PROCESS_SPAWN, SCADEK_MSG_TYPE(%rsi)
    leaq hello_path(%rip), %rax
    movq %rax, SCADEK_MSG_ARG0(%rsi)
    movq $(hello_path_end - hello_path), SCADEK_MSG_ARG1(%rsi)
    movq $0, SCADEK_MSG_ARG2(%rsi)
    movq $0, SCADEK_MSG_ARG3(%rsi)
    movq $0, SCADEK_MSG_STATUS(%rsi)
    movq %rbx, %rdi
    call scadek_endpoint_call

    movq %rsp, %rsi
    movq $0, SCADEK_MSG_SENDER(%rsi)
    movq $SCADEK_SERVICE_PROC, SCADEK_MSG_TARGET(%rsi)
    movq $SCADEK_MSG_PROCESS_SPAWN, SCADEK_MSG_TYPE(%rsi)
    leaq ring_path(%rip), %rax
    movq %rax, SCADEK_MSG_ARG0(%rsi)
    movq $(ring_path_end - ring_path), SCADEK_MSG_ARG1(%rsi)
    movq $0, SCADEK_MSG_ARG2(%rsi)
    movq $0, SCADEK_MSG_ARG3(%rsi)
    movq $0, SCADEK_MSG_STATUS(%rsi)
    movq %rbx, %rdi
    call scadek_endpoint_call

    movq $0, %rdi
    call scadek_exit

.Lhalt:
    pause
    jmp .Lhalt

hello_path:
    .ascii "/hello"
hello_path_end:
ring_path:
    .ascii "/ring-test"
ring_path_end:
.size _start, . - _start

.section .note.GNU-stack,"",@progbits
