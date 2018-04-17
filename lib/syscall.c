// System call stubs.

#include <inc/syscall.h>
#include <inc/lib.h>

/*
 * new syscall using sysenter/sysexit
 * 
 * We use a label (_sysexit) in inline asm that C doesn't know.
 * So we can't declare syscall() with 'inline'. Otherwise C
 * preprocessor will copy those asm codes to multiple places then
 * the compiler will complain that it found the same label
 * declaration in multiple places.
 */
static int32_t
syscall(int num, int check, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5)
{
	int32_t ret;

	asm volatile(
			"pushl %%ebp\n\t"
			"movl $_sysexit, %%esi\n\t"
			"movl %%esp, %%ebp\n\t"
			"sysenter\n\t"
			"_sysexit:\n\t"
			"popl %%ebp"
			: "=a" (ret)
			: "a" (num),
			  "d" (a1),
			  "c" (a2),
			  "b" (a3),
			  "D" (&a4)		/* store pointer to a4 in edi to pass a4 and a5 */
			: "cc", "memory", "esi");

	if(check && ret > 0)
		panic("syscall %d returned %d (> 0)", num, ret);

	return ret;
}

void
sys_cputs(const char *s, size_t len)
{
	syscall(SYS_cputs, 0, (uint32_t)s, len, 0, 0, 0);
}

int
sys_cgetc(void)
{
	return syscall(SYS_cgetc, 0, 0, 0, 0, 0, 0);
}

int
sys_env_destroy(envid_t envid)
{
	return syscall(SYS_env_destroy, 1, envid, 0, 0, 0, 0);
}

envid_t
sys_getenvid(void)
{
	 return syscall(SYS_getenvid, 0, 0, 0, 0, 0, 0);
}

