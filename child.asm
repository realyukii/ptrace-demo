use64

BASE=0x400000
OFFSETOF equ -BASE+
ELF64_PHEADER_ENTRY_SZ = 56

org BASE
Elf64_Ehdr:
	db 0x7F, "ELF", 2, 1, 1				; e_ident[...]		= 7 bytes
	times 9 db 0					; e_ident[EI_PAD]	= 9 bytes
	dw 2						; e_type ET_EXEC	= 2 bytes
	dw 0x3e						; e_machine AMD x86-64	= 2 bytes
	dd 1						; e_version		= 4 bytes
	dq _start					; e_entry entry point	= 8 bytes
	dq OFFSETOF Elf64_Phdr				; e_phoff		= 8 bytes
	dq 0						; e_shoff		= 8 bytes
	dd 0						; e_flags		= 4 bytes
	dw Elf64_Ehdr.SIZE				; e_ehsize		= 2 bytes
	dw ELF64_PHEADER_ENTRY_SZ			; e_phentsize		= 2 bytes
	dw Elf64_Phdr.SIZE/ELF64_PHEADER_ENTRY_SZ	; e_phnum		= 2 bytes
	dw 0						; e_shentsize		= 2 bytes
	dw 0						; e_shnum		= 2 bytes
	dw 0						; e_shstrndx		= 2 bytes
							; total			= 64 bytes
.SIZE = $-Elf64_Ehdr

Elf64_Phdr:
	dd 1						; p_type PT_LOAD	= 4 bytes
	dd 7						; p_flags RWX		= 4 bytes
	dq OFFSETOF _start				; p_offset		= 8 bytes
	dq _start					; p_vaddr		= 8 bytes
	dq 0						; p_paddr		= 8 bytes
	dq _start.SIZE					; p_filesz		= 8 bytes
	dq _start.SIZE					; p_memsz		= 8 bytes
	dq 0						; p_align		= 8 bytes
							; total			= 56 bytes
.SIZE = $-Elf64_Phdr

SYS_write		= 1
SYS_exit		= 60
STDIN_FILENO		= 1

_start:
	mov		eax, SYS_write			; 4 + 1			= 5 bytes
	mov		edi, STDIN_FILENO		; 4 + 1			= 5 bytes
	mov		rsi, buff_ptr			;			= 7 bytes
	mov		edx, buff_len			; 4 + 1			= 5 bytes
	syscall						;			= 2 bytes

	mov		eax, SYS_exit			; 4 + 1			= 5 bytes
	xor		edi, edi			;			= 2 bytes
	syscall						;			= 2 bytes
.SIZE = $-_start					; the size of instructions is 33 bytes

buff_ptr:
	db "assemble an executable file (a program) from scratch!!!", 0xa
buff_len = $-buff_ptr					; length of the string is 56 bytes
