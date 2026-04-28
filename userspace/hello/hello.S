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
    movq $SCADEK_SERVICE_CONSOLE, SCADEK_MSG_TARGET(%rsi)
    movq $SCADEK_MSG_WRITE, SCADEK_MSG_TYPE(%rsi)
    leaq hello_message(%rip), %rax
    movq %rax, SCADEK_MSG_ARG0(%rsi)
    movq $(hello_message_end - hello_message), SCADEK_MSG_ARG1(%rsi)
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

hello_message:
    .ascii "[scadekos] version @SCADEKOS_VERSION@\n"
    .ascii "[scadekos] hello from /hello\n"
    .ascii "[boot] scadekos dev.3 complete\n"
hello_message_end:
.size _start, . - _start

.section .note.GNU-stack,"",@progbits
