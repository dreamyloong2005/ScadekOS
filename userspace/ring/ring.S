/* SPDX-License-Identifier: MPL-2.0 */
/* Copyright (c) 2026 The Scadek OS Project contributors */

#include "@SCADEK_RUNTIME_INC@"

.section .text
.global _start
.type _start, @function
_start:
    movq %rdi, %rbx

    scadek_grant_create_read ring_payload, (ring_payload_end - ring_payload), %rbx
    movq %rax, %r12

    scadek_ring_create SCDK_RING_BATCH
    movq %rax, %r13

    scadek_ring_bind %r13, %rbx
    testq %rax, %rax
    jne .Lbad

    subq $2048, %rsp
    movq %rsp, %r14
    movq $0, %r15

.Lfill_desc:
    cmpq $SCDK_RING_BATCH, %r15
    je .Lsubmit

    movq %r15, %rax
    imulq $SCDK_RING_DESC_SIZE, %rax, %rax
    leaq (%r14,%rax), %rsi

    movq $SCDK_RING_OP_CONSOLE_WRITE, 0(%rsi)
    movq %rbx, 8(%rsi)
    movq %r12, 16(%rsi)
    movq $0, 24(%rsi)
    movq $(ring_payload_end - ring_payload), 32(%rsi)
    movq %r15, 40(%rsi)

    incq %r15
    jmp .Lfill_desc

.Lsubmit:
    scadek_ring_submit %r13, %r14, SCDK_RING_BATCH
    testq %rax, %rax
    jne .Lbad

    leaq 1024(%r14), %r15
    scadek_ring_poll %r13, %r15, SCDK_RING_BATCH
    cmpq $SCDK_RING_BATCH, %rax
    jne .Lbad

    scadek_exit

.Lhalt:
    pause
    jmp .Lhalt

.Lbad:
    pause
    jmp .Lbad

ring_payload:
    .ascii "[scadekos] ring runtime console write"
ring_payload_end:
.size _start, . - _start

.section .note.GNU-stack,"",@progbits
