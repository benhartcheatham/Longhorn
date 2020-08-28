OUTPUT_FORMAT("binary")
OUTPUT_ARCH("i386")
ENTRY(_start)

SECTIONS {
    . = 0x20000

    .text BLOCK(4K) : ALIGN(4K) {
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K) {
		*(.rodata)
	}
 
	.data BLOCK(4K) : ALIGN(4K) {
		*(.data)
	}

	.bss BLOCK(4K) : ALIGN(4K) {
		*(COMMON)
		*(.bss)
	}
}