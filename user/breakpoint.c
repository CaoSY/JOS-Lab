// program to cause a breakpoint trap

#include <inc/lib.h>

void
umain(int argc, char **argv)
{
	asm volatile("int $3");
	asm volatile("nop");
	asm volatile("int $3");
}

