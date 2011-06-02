;///////////////////////////////////////////////////////////////////////////////
;/ $Id: bs.asm,v 1.6 2010/12/20 09:47:14 Ecco Exp $
;/
;/ Author: Joshua Bussdieker
;/ E-Mail: jbussdieker@gmail.com
;/ Description: 
;///////////////////////////////////////////////////////////////////////////////

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Imports
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Peer
extern isr_init
extern data_segment
extern code_segment
extern gdtr
; Superior
extern arch_init

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Exports
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Peer
global start
; Superior
global i386_spinlock
global i386_unlock
global i386_start_trace

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; KLUDGE Definitions
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
STACKSIZE 				equ 0x8000
MULTIBOOT_PAGE_ALIGN	equ 1<<0
MULTIBOOT_MEMORY_INFO	equ 1<<1
MULTIBOOT_HEADER_MAGIC	equ 0x1BADB002
MULTIBOOT_HEADER_FLAGS	equ MULTIBOOT_PAGE_ALIGN | MULTIBOOT_MEMORY_INFO
CHECKSUM				equ -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)
KERNEL_VIRTUAL_BASE equ 0xC0000000
KERNEL_PAGE_NUMBER equ (KERNEL_VIRTUAL_BASE >> 22)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Data
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
section .kpde

align 4096
global BootPageTable
BootPageTable:
    times (1024) dd 0
.end:
align 4096
global BootPageDirectory
BootPageDirectory:
    dd (BootPageTable - KERNEL_VIRTUAL_BASE) + 3
    times (KERNEL_PAGE_NUMBER - 1) dd 0
    dd (BootPageTable - KERNEL_VIRTUAL_BASE) + 3
    times (1024 - KERNEL_PAGE_NUMBER - 1) dd 0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; Code
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
section .mboot
align 4
	dd MULTIBOOT_HEADER_MAGIC
	dd MULTIBOOT_HEADER_FLAGS
	dd CHECKSUM

start:
	cli

	; very dirty
	pusha
	
	mov eax, 0x0
	mov ebx, 0
	.fill_table:
		mov ecx, ebx
		or ecx, 3
		mov [(BootPageTable - KERNEL_VIRTUAL_BASE)+eax*4], ecx
		add ebx, 4096
		inc eax
		cmp eax, 1024
		je .end
	jmp .fill_table
	.end:
	
	; very dirty
	popa
	
    ; NOTE: Until paging is set up, the code must be position-independent and use physical
    ; addresses, not virtual ones!
    mov ecx, (BootPageDirectory - KERNEL_VIRTUAL_BASE)
    mov cr3, ecx                                        ; Load Page Directory Base Register.
 
    mov ecx, cr0
    or ecx, 0x80000000                          ; Set PG bit in CR0 to enable paging.
	
    mov cr0, ecx

    ; Start fetching instructions in kernel space.
    lea ecx, [starthigh]
    jmp ecx                                                     ; NOTE: Must be absolute jump!
 
starthigh:
    ; Unmap the identity-mapped first 4MB of physical address space. It should not be needed
    ; anymore.
    mov dword [BootPageDirectory], 0
    invlpg [0]

	lgdt [gdtr]				; Load our gdt
	jmp code_segment:flush	; Switch to our code descriptor
flush:
	mov cx, data_segment	; Load the data descriptor
	mov ds, cx				; Use the data descriptor for all other segments
	mov es, cx
	mov fs, cx
	mov gs, cx
	mov ss, cx
	mov esp, stackend		; Use our stack
	mov ebp, esp			; Set the base pointer
	call isr_init
	add ebx, KERNEL_VIRTUAL_BASE
	push eax				; Pass the GRUB parameters
	push ebx				; Pass the GRUB parameters
	call arch_init			; Call our initialization code
idle_loop:
	hlt
	jmp idle_loop
	;jmp $					; Just in case

i386_start_trace:
	pushf
	pop     ax
	or      ah, 1
	push    ax
	popf
	ret

i386_spinlock:
	mov eax, 1
	mov edx, [esp+4]			; get the pointer we passed it
	loop:
		xchg eax, [edx]
		test eax, eax
	jnz loop
	ret

i386_unlock:
	mov eax, 0
	mov edx, [esp+4]
	xchg eax, [edx]
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; BSS
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
section .bss
align 4						; DWORD Align our stack
stack:
   resb STACKSIZE			; Reserve stack space that is STACKSIZE bytes
stackend:
