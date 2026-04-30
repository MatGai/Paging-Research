	BITS 64
    DEFAULT REL

	section .text
	align 16

    global scouse_transition_address_space
    global scouse_transition_address_space_start
    global scouse_transition_address_space_end

    ;
	; rcx = Address space switiching to
    ; rdx = Top of new stack  ; must be 16-byte aligned
    ; r8  = entrypoint        ; void entrypoint(BOOT_INFO*) 
    ; r9  = bootinfo*
    ;
    scouse_transition_address_space:
    scouse_transition_address_space_start:
    
        ;
        ; Save MS-x64 non-volatile GPRs on the old stack.
        ;
        push    rbp
        push    rbx
        push    rsi
        push    rdi
        push    r12
        push    r13
        push    r14
        push    r15

        ;
        ; Save original RFLAGS on the old stack too.
        ;
        pushfq

        ;
        ; Save old CR3 and old stack pointer.
        ;
        mov     r10, cr3
        mov     r11, rsp

        cli
        mov     cr3, rcx
        mov     rsp, rdx

        ;
        ; New stack is expected to be 16-byte aligned.
        ; Save old CR3 and old old-stack pointer on the new stack.
        ;
        push    r10
        push    r11

        ;
        ; MS-x64 shadow space.
        ;
        sub     rsp, 0x20

        ;
        ; Call entrypoint(bootinfo).
        ;
        mov     rcx, r9
        call    r8

        add     rsp, 0x20

        ;
        ; Restore old state.
        ;
        pop     r11                 ; old RSP
        pop     r10                 ; old CR3

        mov     cr3, r10
        mov     rsp, r11

        ;
        ; Restore original flags after old CR3 and old stack are back.
        ;
        popfq

        pop     r15
        pop     r14
        pop     r13
        pop     r12
        pop     rdi
        pop     rsi
        pop     rbx
        pop     rbp

        ret

    scouse_transition_address_space_end: