	BITS 64
    DEFAULT REL

	section .text
	align 16



	global _scouse_check_cpuid_support
	_scouse_check_cpuid_support:

		pushfq
		pop     rax                 ; rax = original RFLAGS
		mov     rcx, rax             ; rcx = saved original RFLAGS, volatile register

		xor     rax, 1 << 21         ; toggle ID bit
		push    rax
		popfq

		pushfq
		pop     rax                 ; rax = modified RFLAGS

		xor     rax, rcx             ; changed bits
		and     rax, 1 << 21         ; isolate ID bit change

		push    rcx
		popfq                       ; restore original RFLAGS

		shr     rax, 21
		and     rax, 1
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

		rdrand  rax
		jc      .success

		xor     eax, eax
		ret

	.success:
		mov     [rcx], rax
		mov     eax, 1
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
		
		;
		; Call RDRAND instruction, which will store a random value in rbx.
		; If the RDRAND instruction fails, the carry flag will be 0.
		;

		test    rcx, rcx
		jz      .fail

		inc     rdx                 ; attempts = retries + 1
		jz      .fail               ; avoid max unsigned qword wrap to zero

	.retry:
		rdrand  rax
		jc      .success

		dec     rdx
		jnz     .retry

	.fail:
		xor     eax, eax
		ret

	.success:
		mov     [rcx], rax
		mov     eax, 1
		ret