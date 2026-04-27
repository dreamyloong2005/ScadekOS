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

    scadek_proc_spawn %rsi, hello_path, (hello_path_end - hello_path)
    scadek_endpoint_call %rbx, %rsi

    movq %rsp, %rsi
    scadek_proc_spawn %rsi, ring_path, (ring_path_end - ring_path)
    scadek_endpoint_call %rbx, %rsi

    scadek_exit

.Lhalt:
    hlt
    jmp .Lhalt

hello_path:
    .ascii "/hello"
hello_path_end:
ring_path:
    .ascii "/ring-test"
ring_path_end:
.size _start, . - _start

.section .note.GNU-stack,"",@progbits
