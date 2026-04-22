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
    
        push rbp
        push rbx
        push rsi
        push rdi
        push r12
    
        pushfq
        pop  r12
    
        mov  r10, cr3
        mov  r11, rsp
    
        cli
        mov  cr3, rcx
        mov  rsp, rdx             
    
        push r10
        push r11
        sub  rsp, 0x20          
    
        mov  rcx, r9          
        call r8
    
        add  rsp, 0x20
        pop  r11
        pop  r10
    
        mov  cr3, r10
        mov  rsp, r11
    
        push r12
        popfq
    
        pop  r12
        pop  rdi
        pop  rsi
        pop  rbx
        pop  rbp

        ret

    scouse_transition_address_space_end: