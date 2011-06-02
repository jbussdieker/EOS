;///////////////////////////////////////////////////////////////////////////////
;/ $Id: isr.asm,v 1.3 2010/12/20 04:45:31 Ecco Exp $
;/
;/ Author: Joshua Bussdieker
;/ E-Mail: jbussdieker@gmail.com
;/ Description: 
;///////////////////////////////////////////////////////////////////////////////

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Imports
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Peer
; Superior
extern isr
extern curtask

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Exports
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Peer
global isr_init
global gdtr
global code_segment
global data_segment
; Superior
global syscall
global tss

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Code
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
isr_init:
	pusha
	
	; Patch the tss address
	mov eax, gdt_tss
	mov ebx, tss
	add eax, 2
	mov word [eax], bx
	mov ax, tss_segment
	ltr ax
	
	; Patch the idt
	xor ecx, ecx
	mov cx, word [idtr]
	inc ecx
	shr ecx, 3
	
	mov esi, isr_primary
	mov edi, idt	
	
	.loop:
		mov word [edi], si
		add esi, 10
		add edi, 8
		dec ecx
	jnz .loop
		
	lidt [idtr]
	popa
	ret

global isr_handler
isr_handler:
	; Kernel stack loaded by ISR
	pusha
	push ds
	push es
	push fs
	push gs
	mov eax, [curtask]
	mov [eax], esp
	mov ecx, cr3
	mov [eax+8], ecx
	mov ax, data_segment
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov eax, isr
	push esp
	call eax
	pop esp
	mov eax, [curtask]
	mov esp, [eax]
	mov ebx, [eax+4]
	mov [tss+4],ebx	
	;mov ebx, [eax+8]
	;mov [_tss+28],ebx
	mov ecx, [eax+8]
global blah
blah:
	mov cr3, ecx
    invlpg [0]
    invlpg [768]
	pop gs
	pop fs
	pop es
	pop ds
	popa
	add esp, 8
	iret

syscall:
	pusha
	mov ebp, esp
	add ebp, 36
	mov eax, [ebp];
	mov ebx, [ebp+4];
	mov ecx, [ebp+8];
	mov edx, [ebp+12];
	mov edi, [ebp+16];
	int 48
	mov [ebp-8], eax
	popa
	ret
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Primary Handlers
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
%macro ISR_STUB 2
  ;global isr%1
  ;isr%1:
    cli
    %if %2 = 0
    	push byte 0
    %else
		nop
		nop
    %endif
	push byte %1
    jmp long isr_handler
%endmacro

isr_primary:
ISR_STUB 0, 0
ISR_STUB 1, 0
ISR_STUB 2, 0
ISR_STUB 3, 0
ISR_STUB 4, 0
ISR_STUB 5, 0
ISR_STUB 6, 0
ISR_STUB 7, 0
ISR_STUB 8, 1
ISR_STUB 9, 0
ISR_STUB 10, 0
ISR_STUB 11, 0
ISR_STUB 12, 0
ISR_STUB 13, 0
ISR_STUB 14, 0
ISR_STUB 15, 0
ISR_STUB 16, 0
ISR_STUB 17, 0
ISR_STUB 18, 0
ISR_STUB 19, 0
ISR_STUB 20, 0
ISR_STUB 21, 0
ISR_STUB 22, 0
ISR_STUB 23, 0
ISR_STUB 24, 0
ISR_STUB 25, 0
ISR_STUB 26, 0
ISR_STUB 27, 0
ISR_STUB 28, 0
ISR_STUB 29, 0
ISR_STUB 30, 0
ISR_STUB 31, 0
ISR_STUB 32, 0
ISR_STUB 33, 0
ISR_STUB 34, 0
ISR_STUB 35, 0
ISR_STUB 36, 0
ISR_STUB 37, 0
ISR_STUB 38, 0
ISR_STUB 39, 0
ISR_STUB 40, 0
ISR_STUB 41, 0
ISR_STUB 42, 0
ISR_STUB 43, 0
ISR_STUB 44, 0
ISR_STUB 45, 0
ISR_STUB 46, 0
ISR_STUB 47, 0
ISR_STUB 48, 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; IDT Table
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
%macro IDT_STUB 2
	;global idt%1
	;idt%1:
		dw 0x0000
		dw 0x0008
		%if %2 == 0
			dd 0xC0108E00
		%else
			dd 0xC010EE00
		%endif
%endmacro

idt:
IDT_STUB 0, 0
IDT_STUB 1, 1
IDT_STUB 2, 0
IDT_STUB 3, 0
IDT_STUB 4, 0
IDT_STUB 5, 0
IDT_STUB 6, 0
IDT_STUB 7, 0
IDT_STUB 8, 0
IDT_STUB 9, 0
IDT_STUB 10, 0
IDT_STUB 11, 0
IDT_STUB 12, 0
IDT_STUB 13, 0
IDT_STUB 14, 0
IDT_STUB 15, 0
IDT_STUB 16, 0
IDT_STUB 17, 0
IDT_STUB 18, 0
IDT_STUB 19, 0
IDT_STUB 20, 0
IDT_STUB 21, 0
IDT_STUB 22, 0
IDT_STUB 23, 0
IDT_STUB 24, 0
IDT_STUB 25, 0
IDT_STUB 26, 0
IDT_STUB 27, 0
IDT_STUB 28, 0
IDT_STUB 29, 0
IDT_STUB 30, 0
IDT_STUB 31, 0
IDT_STUB 32, 0
IDT_STUB 33, 0
IDT_STUB 34, 0
IDT_STUB 35, 0
IDT_STUB 36, 0
IDT_STUB 37, 0
IDT_STUB 38, 0
IDT_STUB 39, 0
IDT_STUB 40, 0
IDT_STUB 41, 0
IDT_STUB 42, 0
IDT_STUB 43, 0
IDT_STUB 44, 0
IDT_STUB 45, 0
IDT_STUB 46, 0
IDT_STUB 47, 0
IDT_STUB 48, 1
idt_end:

; Interrupt Descriptor Table Register
idtr:
	dw idt_end-idt-1
	dd idt

tss:
	tss_reserved	dd 0xFFFFFFFF
	tss_esp0 		dd 0xFFFFFFFF
	tss_ss0			dd 0x00000010 ; 16 bit
	tss_esp1 		dd 0xFFFFFFFF
	tss_ss1			dd 0xFFFFFFFF ; 16 bit
	tss_esp2 		dd 0xFFFFFFFF
	tss_ss2			dd 0xFFFFFFFF ; 16 bit
	tss_cr3 		dd 0xFFFFFFFF
	tss_eip 		dd 0xFFFFFFFF
	tss_eflags 		dd 0xFFFFFFFF
	tss_eax 		dd 0xFFFFFFFF
	tss_ecx 		dd 0xFFFFFFFF
	tss_edx 		dd 0xFFFFFFFF
	tss_ebx 		dd 0xFFFFFFFF
	tss_esp 		dd 0xFFFFFFFF
	tss_ebp 		dd 0xFFFFFFFF
	tss_esi 		dd 0xFFFFFFFF
	tss_edi 		dd 0xFFFFFFFF
	tss_es			dd 0xFFFFFFFF ; 16 bit
	tss_cs			dd 0xFFFFFFFF ; 16 bit
	tss_ss			dd 0xFFFFFFFF ; 16 bit
	tss_ds			dd 0xFFFFFFFF ; 16 bit
	tss_fs			dd 0xFFFFFFFF ; 16 bit
	tss_gs			dd 0xFFFFFFFF ; 16 bit
	tss_ldtr		dd 0xFFFFFFFF ; 16 bit
	tss_iopb		dd 0x00680000 ; 16 bit, Upper

; Global Descriptor Table
gdt:
	; null descriptor
	gdt_null:
		dd 0x00000000
		dd 0x00000000
	; code                                             
	gdt_cs:
		code_segment equ $-gdt
		dd 0x0000FFFF
		dd 0x00CF9A00
	; data
	gdt_ds:
		data_segment equ $-gdt
		dd 0x0000FFFF
		dd 0x00CF9200
	; code                                             
	gdt_ucs:
		ucode_segment equ $-gdt
		dd 0x0000FFFF
		dd 0x00CFFA00
	; data
	gdt_uds:
		udata_segment equ $-gdt
		dd 0x0000FFFF
		dd 0x00CFF200
	; tss
	gdt_tss:
		tss_segment equ $-gdt
		dw 0x0068
		dw 0x0000
		dd 0xC0408910
gdt_end:

; Global Descriptor Table Register Pointer
gdtr:
	dw gdt_end-gdt-1
	dd gdt
