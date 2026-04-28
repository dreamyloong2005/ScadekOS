/* SPDX-License-Identifier: MPL-2.0 */
/* Copyright (c) 2026 The Scadek OS Project contributors */

#include <scadek/scadek.inc>

.section .text
.global _start
.type _start, @function
_start:
    movq %rdi, %rbx

    leaq ring_payload(%rip), %rdi
    movq $(ring_payload_end - ring_payload), %rsi
    movq $SCADEK_RIGHT_READ, %rdx
    movq %rbx, %rcx
    call scadek_grant_create
    movq %rax, %r12

    movq $SCADEK_RING_BATCH, %rdi
    call scadek_ring_create
    movq %rax, %r13

    movq %r13, %rdi
    movq %rbx, %rsi
    call scadek_ring_bind
    testq %rax, %rax
    jne .Lbad

    subq $2048, %rsp
    movq %rsp, %r14
    movq $0, %r15

.Lfill_desc:
    cmpq $SCADEK_RING_BATCH, %r15
    je .Lsubmit

    movq %r15, %rax
    imulq $SCADEK_RING_DESC_SIZE, %rax, %rax
    leaq (%r14,%rax), %rsi

    movq $SCADEK_RING_OP_CONSOLE_WRITE, 0(%rsi)
    movq %rbx, 8(%rsi)
    movq %r12, 16(%rsi)
    movq $0, 24(%rsi)
    movq $(ring_payload_end - ring_payload), 32(%rsi)
    movq %r15, 40(%rsi)

    incq %r15
    jmp .Lfill_desc

.Lsubmit:
    movq %r13, %rdi
    movq %r14, %rsi
    movq $SCADEK_RING_BATCH, %rdx
    call scadek_ring_submit
    testq %rax, %rax
    jne .Lbad

    leaq 1024(%r14), %r15
    movq %r13, %rdi
    movq %r15, %rsi
    movq $SCADEK_RING_BATCH, %rdx
    call scadek_ring_poll
    cmpq $SCADEK_RING_BATCH, %rax
    jne .Lbad

    movq $0, %rdi
    call scadek_exit

.Lhalt:
    pause
    jmp .Lhalt

.Lbad:
    pause
    jmp .Lbad

ring_payload:
    .ascii "[scadekos] libscadek ring console write"
ring_payload_end:
.size _start, . - _start

.section .note.GNU-stack,"",@progbits
