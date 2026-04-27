/* SPDX-License-Identifier: MPL-2.0 */
/* Copyright (c) 2026 The Scadek OS Project contributors */

#include "@SCADEK_RUNTIME_INC@"

.section .text
.global _start
.type _start, @function
_start:
    movq %rdi, %rbx

    subq $SCADEK_MSG_SIZE, %rsp
    movq %rsp, %rsi

    scadek_console_write %rsi, hello_message, (hello_message_end - hello_message)
    scadek_endpoint_call %rbx, %rsi

    scadek_exit

.Lhalt:
    hlt
    jmp .Lhalt

hello_message:
    .ascii "[scadekos] version @SCADEKOS_VERSION@\n"
    .ascii "[scadekos] hello from /hello\n"
    .ascii "[boot] scadekos dev.2 complete\n"
hello_message_end:
.size _start, . - _start

.section .note.GNU-stack,"",@progbits
