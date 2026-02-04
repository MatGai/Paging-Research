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


	;
	; _rdrandy64_step - Trys to retrieve a single 64-bit random value. ** RDRAND can fail ** 
	;
	; Input(s):		RCX - 64-bit pointer to store the random value
	;
	; Output(s):	1 on success, 0 on failure
	;
	
	global _scouse_rdrand64_step
	_scouse_rdrand64_step:

		push rbx

		rdrand rbx
		jc .end 

		xor rax, rax
		pop rbx
		ret

	.end: 

		mov [rcx], rbx
		mov rax, 1
		pop rbx
		ret

	; 
	; _rdrandy64_try - Attemps to retrieve a 64-bit random value, with specified retries. (0 retries will try once)
	;
	;  Input(s):	RCX - 64-bit pointer to store the random value
	;				RDX - Signed 32-bit number of retries
	;
	;  Output(s):	1 on success, 0 on failure
	;

	global _scouse_rdrand64_retry
	_scouse_rdrand64_retry:
		
		push rbx  
		test rdx, rdx
		inc rdx

	.retry:
		
		;
		; Call RDRAND instruction, which will store a random value in rbx.
		; If the RDRAND instruction fails, the carry flag will be 0.
		;

		rdrand rbx
		jc .end			; jumps to .end if carry flag is set   

		dec rdx
		jnz .retry		; rdx should be signed otherwise this is weird
		
		;
		; We have run out of tries, set return value to 0 and return 
		;

		xor rax, rax
		pop rbx
		ret

	.end:
		
		mov [rcx], rbx
		mov rax, 1
		
		pop rbx
		ret