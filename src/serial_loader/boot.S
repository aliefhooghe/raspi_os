
.section ".text"

.globl _start
_start:
	mov sp, #0x8000
	bl loader_main
halt:
	wfe
	b halt
