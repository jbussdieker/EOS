OUTPUT_FORMAT("elf32-i386")
ENTRY(start)
phys = 0x00100000;
virt = 0xC0100000;
SECTIONS
{
	.text virt : AT(phys) 
	{
		code = .;
		_ld_kernel_start = .;
		*(.mboot)
		*(.text)
	}
	.data ALIGN(4096): AT(phys + (data - code))
	{
		data = .;
		*(.kpde)
		*(.data)
	}
	.bss : AT(phys + (bss - code))
	{
		bss = .;
		*(.bss)
		_ld_kernel_end = .;
	}
}
