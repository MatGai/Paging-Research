%define MAGIC_HIDWORD       0xbaad0000
%define MAGIC_LODWORD       0xdeadc0de  
	
	BITS 64
    DEFAULT REL

	section .text
	align 16

; -----------------------------------------------------------------------------------------------
; uint64_t 
; _scouse_readmsr(
;   _In_  uint32_t,    // rax
;   _Out_ uint32_t*    // rdx
; );
;
; -----------------------------------------------------------------------------------------------


	global _scouse_readmsr
	_scouse_readmsr:

	xor r10d, r10d

	rdmsr

	mov r8, cr2
	cmp r8d, MAGIC_LODWORD
	jne readmsr_success

	xor r8, r8
	mov cr2, r8
	mov r10d, 1

readmsr_success:
 
	shl rdx, 32
	or  rax, rdx

	test edx, edx
	jz readmsr_ret

	mov dword [edx], r10d

readmsr_ret:

	ret

; -----------------------------------------------------------------------------------------------
; 
; void 
; _scouse_cpuid( 
;    _In_  unsigned __int32,    // rcx 
;    _Out_ unsigned __int32*    // rdx , pointer to buffer { eax, ebx, ecx, edx }
; );                     
;
; -----------------------------------------------------------------------------------------------

	global _scouse_cpuid
	_scouse_cpuid:

    push rbx			 ; save rbx as cpuid over writes it 

    mov  r10, rdx        ; save paramter into r10 as cpuid overwrite rdx
    mov  eax, ecx        ; cpuid takes function from eax
    xor  ecx, ecx        ; zero subleaf

    cpuid

    mov  dword [r10 + 0],  eax
    mov  dword [r10 + 4],  ebx
    mov  dword [r10 + 8],  ecx
    mov  dword [r10 + 12], edx

    pop  rbx
    ret

; -----------------------------------------------------------------------------------------------
; 
; void 
; _scouse_cpuidex( 
;    _In_  unsigned __int32,    // rcx 
;	 _In_  unsigned __int32,    // rdx 
;    _Out_ unsigned __int32*    // r8, pointer to buffer { eax, ebx, ecx, edx }
; );                     
;
; -----------------------------------------------------------------------------------------------

	global _scouse_cpuidex
	_scouse_cpuidex:

    push rbx			 ; save rbx as cpuid over writes it 

    mov  r10, r8         ; save paramter into r10 as cpuid overwrite rdx
    mov  eax, ecx        ; cpuid takes function from eax
    mov  ecx, edx        ; cpuid takes subleaf index from ecx

    cpuid

    mov  dword [r10 + 0],  eax
    mov  dword [r10 + 4],  ebx
    mov  dword [r10 + 8],  ecx
    mov  dword [r10 + 12], edx

    pop  rbx
    ret

; -----------------------------------------------------------------------------------------------
;
; unsigned __int64	// rax
; _scouse_readcr0(
;	
; );
;
; -----------------------------------------------------------------------------------------------

	global _scouse_readcr0
	_scouse_readcr0:

	mov rax, cr0
	ret

; -----------------------------------------------------------------------------------------------
;
; unsigned __int64	// rax
; _scouse_writecr0(
;	_In_ unsigned __int64	// rcx
; );
;
; -----------------------------------------------------------------------------------------------

	global _scouse_writecr0
	_scouse_writecr0:

	mov rax, rcx
	mov cr0, rax
	ret

; -----------------------------------------------------------------------------------------------
;
; unsigned __int64	// rax
; _scouse_readcr2(
;	
; );
;
; -----------------------------------------------------------------------------------------------

	global _scouse_readcr2
	_scouse_readcr2:

	mov rax, cr2
	ret

; -----------------------------------------------------------------------------------------------
;
; unsigned __int64	// rax
; _scouse_writecr2(
;	_In_ unsigned __int64	// rcx
; );
;
; -----------------------------------------------------------------------------------------------

	global _scouse_writecr2
	_scouse_writecr2:

	mov rax, rcx
	mov cr2, rax
	ret


; -----------------------------------------------------------------------------------------------
;
; unsigned __int64	// rax
; _scouse_readcr3(
;
; );
;
; -----------------------------------------------------------------------------------------------

	global _scouse_readcr3
	_scouse_readcr3:

	mov rax, cr3
	ret

; -----------------------------------------------------------------------------------------------
;
; unsigned __int64	// rax
; _scouse_writecr3(
;	_In_ unsigned __int64	// rcx
; );
;
; -----------------------------------------------------------------------------------------------

	global _scouse_writecr3
	_scouse_writecr3:

	mov rax, rcx
	mov cr3, rax
	ret

; -----------------------------------------------------------------------------------------------
;
; unsigned __int64	// rax
; _scouse_readcr4(
;
; );
;
; -----------------------------------------------------------------------------------------------

	global _scouse_readcr4
	_scouse_readcr4:

	mov rax, cr4
	ret

; -----------------------------------------------------------------------------------------------
;
; unsigned __int64	// rax
; _scouse_writecr4(
;	_In_ unsigned __int64	// rcx
; );
;
; -----------------------------------------------------------------------------------------------

	global _scouse_writecr4
	_scouse_writecr4:

	mov rax, rcx
	mov cr4, rax
	ret

; -----------------------------------------------------------------------------------------------
;
; unsigned __int64	// rax	
; _scouse_readeflags(
;
; );
;
; -----------------------------------------------------------------------------------------------

	global _scouse_readeflags
	_scouse_readeflags:

	     pushfq		 ; put RFLAGS onto the stack
		 pop     rax ; Pop RFLAGS from stack into RAX
		 ret

; -----------------------------------------------------------------------------------------------
;
; void	
; _scouse_writeeflags(
;	_In_ unsigned __int64 // rcx
; );
;
; -----------------------------------------------------------------------------------------------

	global _scouse_writeeflags
	_scouse_writeeflags:

	    push    rcx ; Save RCX to stack
		popfq		; Push RCX value into RFLAGS
		ret

; -----------------------------------------------------------------------------------------------
;
; void	
; _scouse_debugbreak(

; );
;
; -----------------------------------------------------------------------------------------------

	global _scouse_debugbreak
	_scouse_debugbreak:

	int3
	ret


	global _scouse_mfence
	_scouse_mfence:
		mfence
		ret

	global _scouse_lfence
	_scouse_lfence:
		lfence
		ret

	global _scouse_rdtsc
	_scouse_rdtsc:
		rdtsc
		shl rdx, 32
		or rax, rdx
		ret

	global _scouse_rdtscp
	_scouse_rdtscp:
		rdtscp
		shl rdx, 32
		or rax, rdx 
		ret

	global _scouse_readrsp
	_scouse_readrsp
		mov rax, rsp
		ret

