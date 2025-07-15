format ELF64 executable

; playing around jump and call instruction

; addressing mode:
; https://web.stanford.edu/class/archive/cs/cs107/cs107.1258/guide/x86-64.html

SYS_EXIT	= 60
SYS_WRITE	= 1
STDOUT_FILENO	= 1

write_hemlo:
	mov	eax, SYS_WRITE
	mov	edi, STDOUT_FILENO
	lea	rsi, [str_buff]
	mov	edx, str_len
	syscall

_start:
	mov	eax, SYS_EXIT
	xor	edi, edi
	syscall

str_buff db "hemlo", 0xa
str_len = $ - str_buff
