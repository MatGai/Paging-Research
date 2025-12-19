	BITS 64
    DEFAULT REL

	section .text
	align 16



	global _scouse_check_cpuid_support
	_scouse_check_cpuid_support:

		   pushfq    
		   pop     rax     
		   mov     rbx, rax		   ; store RFLAGS into RBX
		   
		   xor     rax, 1 << 21    ; toggle ID flag 
		   push    rax     
		   popfq                   ; set RFLAGS with new ID flag

		   pushfq         
		   pop     rax             ; get new RFLAGS into RAX
		   xor     rax, rbx        
		   and     rax, 1 << 21    ; check for any change in ID flag
		   push    rbx
		   popfq                   ; restore RFLAGS left from RBX
		   
		   shr	   rax, 21         ; move ID flag to bit 0
		   and	   rax, 1          ; mask all other bits

		   ret
