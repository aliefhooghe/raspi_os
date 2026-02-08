
.section ".text.boot"

.globl _start
_start:
	mov sp, #0x7000   // before where the loader is copied
	bl loader_main
halt:
	wfe
	b halt
