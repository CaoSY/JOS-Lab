// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>
#include <inc/color.h>
#include <inc/types.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/pmap.h>

#define CMDBUF_SIZE	80	// enough for one VGA text line


struct Command {
	const char *name;
	const char *desc;
	// return -1 to force monitor to exit
	int (*func)(int argc, char** argv, struct Trapframe* tf);
};

static struct Command commands[] = {
	{ "help", CMD_HELP_HELP_STR, mon_help },
	{ "kerninfo", CMD_KERNINFO_HELP_STR, mon_kerninfo },
	{ "backtrace", CMD_BACKTRACE_HELP_STR, mon_backtrace},
	{ "mappings", CMD_MAPPINGS_HELP_STR, mon_mappings},
};

/***** helper functions *****/

#define CMD_ERR_ARG		0		// wrong arguments
#define CMD_ERR_NUM		1		// invalid number format
#define CMD_ERR_OPE		2		// invalid operation
#define CMD_ERR_STR		3		// not specific error
inline int
cmd_error(int err_type, char *str)
{
	switch(err_type) {
		case CMD_ERR_ARG: 
			cprintf("E: Wrong arguments! type 'help %s' for usage.\n", str);
			break;
		case CMD_ERR_NUM:
			cprintf("E: Wrong Number format!\n");
			break;
		case CMD_ERR_OPE:
			cprintf("E: Invalid operation %s\n", str);
			break;
		case CMD_ERR_STR:
			cprintf("E: %s\n", str);
			break;
		default:
			cprintf("E: Wrong error type!\n");
			break;
	}
	
	return 0;
}

#define parse_number(_num_str, _num_ptr)			\
({													\
	typeof(_num_str) __num_str = (_num_str);		\
	typeof(_num_ptr) __num_ptr = (_num_ptr);	\
	char *end_char;									\
	*__num_ptr = strtol(__num_str, &end_char, 0);	\
	*end_char != '\0';								\
})

#define MAX(_a, _b)						\
({								\
	typeof(_a) __a = (_a);					\
	typeof(_b) __b = (_b);					\
	__a >= __b ? __a : __b;					\
})

/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	char *operation;
	if ((operation = argv[1]) == 0) {
		cprintf(CMD_HELP_HELP_STR);
		return 0;
	}

	if (argc > 2)
		return cmd_error(CMD_ERR_ARG, "help");

	if (strcmp(operation, "list") == 0) {
		for (int i = 0; i < ARRAY_SIZE(commands); i++)
			cprintf("%s\n   -%s\n", commands[i].name, commands[i].desc);
		return 0;
	}


	bool cmd_found = false;
	for (int i = 0; i < ARRAY_SIZE(commands); i++) {
		if (strcmp(operation, commands[i].name) == 0) {
			cprintf("%s\n   -%s\n", commands[i].name, commands[i].desc);
			cmd_found = true;
		}
	}
	
	if (!cmd_found)
		cprintf("command not found!\n");

	return 0;
}

int
mon_kerninfo(int argc, char **argv, struct Trapframe *tf)
{
	extern char _start[], entry[], etext[], edata[], end[];

	cprintf("Special kernel symbols:\n");
	cprintf("  _start                  %08x (phys)\n", _start);
	cprintf("  entry  %08x (virt)  %08x (phys)\n", entry, entry - KERNBASE);
	cprintf("  etext  %08x (virt)  %08x (phys)\n", etext, etext - KERNBASE);
	cprintf("  edata  %08x (virt)  %08x (phys)\n", edata, edata - KERNBASE);
	cprintf("  end    %08x (virt)  %08x (phys)\n", end, end - KERNBASE);
	cprintf("Kernel executable memory footprint: %dKB\n",
		ROUNDUP(end - entry, 1024) / 1024);
	return 0;
}

int
mon_backtrace(int argc, char **argv, struct Trapframe *tf)
{
/*
 * Stack Structure
 *
 *    High Address ->  +------------------------------+
 *                     :              .               :
 *                     :              .               :
 *                     :              .               :
 *                     |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
 *                     |            arg 5             |
 *                     |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
 *                     :              .               |
 *                     :              .               |
 *                     :              .               |
 *                     |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
 *                     |            arg 1             |
 *                     |        return address        |
 *    callee ebp --->  |          caller ebp          |
 *                     |~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~|
 *                     :              .               :
 *                     :              .               :
 *                     :              .               :
 *    0 ------------>  +------------------------------+
 *
 * (*) Note: ebp, addresses, args are all 4-bytes in 32-bit system.
 */

	struct Eipdebuginfo info;
	uint32_t *ebp = (uint32_t *) read_ebp();
	cprintf("Stack backtrace:\n");

	while(ebp) {
		cprintf("  ebp %08x  eip %08x  args", ebp, ebp[1]);
		for(int i = 2; i < 7; ++i) {
			cprintf(" %08x", ebp[i]);
		}
		debuginfo_eip(ebp[1], &info);
		cprintf("\n       %s:%d: %.*s+%d\n", info.eip_file,\
			info.eip_line, info.eip_fn_namelen, info.eip_fn_name, ebp[1]-info.eip_fn_addr);
		ebp = (uint32_t *) (*ebp);
	}

	return 0;
}

int
show_mappings(uintptr_t lower_addr, uintptr_t upper_addr)
{
	uintptr_t laddr = ROUNDDOWN(lower_addr, PGSIZE);
	uintptr_t uaddr = ROUNDDOWN(upper_addr, PGSIZE);

	if (laddr != lower_addr)
		cprintf("lower address:%p -> %p\n", lower_addr, laddr);
	if (uaddr != upper_addr)
		cprintf("upper address:%p -> %p\n", upper_addr, uaddr);

	cprintf("         vaddr                       paddr            privilege\n");
	while (laddr < uaddr) {
		cprintf("%08p - %08p:    ", laddr, laddr + PGSIZE);
		
		pte_t *pt_entry = pgdir_walk(kern_pgdir, (void *)laddr, false);
		if (pt_entry == NULL || !(*pt_entry & PTE_P)) {
			cprintf("not mapped\n");
		} else {
			physaddr_t pte_paddr = PTE_ADDR(*pt_entry);
			char privilege[] = {
				(*pt_entry & PTE_U) ? 'u' : '-',
				(*pt_entry & PTE_W) ? 'w' : '-',
				'\0'
			};

			cprintf("%08p - %08p      %s\n", pte_paddr, pte_paddr + PGSIZE, privilege);
		}

		laddr += PGSIZE;
	}

	return 0;	
}

int
set_mappings(uintptr_t virtual_addr, size_t size, int perm)
{
	if (PGNUM(virtual_addr) + size > npages_in_4GB)
		return cmd_error(CMD_ERR_STR, "Addresses exceed 4G memory!");
	
	uintptr_t vaddr = ROUNDDOWN(virtual_addr, PGSIZE);
	if (vaddr != virtual_addr)
		cprintf("virtual address:%p -> %p\n", virtual_addr, vaddr);
	
	pte_t *pt_entry;
	while (--size >= 0) {
		pt_entry = pgdir_walk(kern_pgdir, (void *)vaddr, false);
		if (*pt_entry & PTE_P) {
			// turn off PTE_U and PTE_W
			*pt_entry &= ~(PTE_U | PTE_W);
			
			// mask off other bits in perm, and turn on bits
			// in *pt_entry based on perm
			*pt_entry |= (perm & (PTE_U | PTE_W));
		} else
			cprintf("W: %d not mapped!\n", vaddr);

		vaddr += PGSIZE;
	}

	return 0;
}

int
clear_mappings(uintptr_t virtual_addr, size_t size)
{
	// turn off PTE_U and PTE_W
	return set_mappings(virtual_addr, size, 0);
}

int
mon_mappings(int argc, char **argv, struct Trapframe *tf)
{
	/*
	 * SYNOPSIS:
	 * 		mappings {show laddr uaddr} |
	 * 				 {clear vaddr [size]} |
	 * 				 {set perm vaddr [size]}
	 * 		laddr: lower address
	 * 		uaddr: upper address
	 * 		vaddr: virtual address
	 * 		paddr: physical address
	 * 		size: memory size in pages
	 * 		perm: page table entry permission
	 */

	char *operation;
	if ((operation = argv[1]) == 0) {
		cprintf(CMD_MAPPINGS_HELP_STR);
		return 0;
	}

	if (strcmp(operation, "show") == 0) {
		// mappings {show laddr uaddr}

		if (argc != 4)
			return cmd_error(CMD_ERR_ARG, "mappings");
		
		uintptr_t laddr, uaddr;
		if (parse_number(argv[2], &laddr) ||
			parse_number(argv[3], &uaddr))
			return cmd_error(CMD_ERR_NUM,"");
		
		return show_mappings(laddr, uaddr);
	}
	
	if (strcmp(operation, "clear") == 0) {
		// mappings {clear vaddr [size]}
		
		if (argc < 3 || argc > 4)
			return cmd_error(CMD_ERR_ARG, "mappings");
		
		uintptr_t vaddr;
		if (parse_number(argv[2], &vaddr))
			return cmd_error(CMD_ERR_NUM, NULL);
		
		size_t size;
		if (argv[3]) {		// if size is provided explicitly
			if (parse_number(argv[3], &size))
				return cmd_error(CMD_ERR_NUM, NULL);
		} else
			size = 1;
		
		return clear_mappings(vaddr, size);
	}
	
	if (strcmp(operation, "set") == 0) {
		// mappings {set perm vaddr [size]}
		if (argc < 4 || argc > 5)
			return cmd_error(CMD_ERR_ARG, "mappings");
		
		int perm;
		char *perm_str = argv[2];
		if (parse_number(perm_str, &perm)) {
			if (strlen(perm_str) != 2)
				return cmd_error(CMD_ERR_STR, "Wrong privilege format!");
			
			perm = 0;
			for (int i = 0; i < 2; ++i) {
				if (perm_str[i] == 'u')
					perm |= PTE_U;
				else if (perm_str[i] == 'w')
					perm |= PTE_W;
				else if (perm_str[i] == '-')
					continue;
				else
					return cmd_error(CMD_ERR_STR, "Wrong privilege format!");
			}
		}

		uintptr_t vaddr;
		if (parse_number(argv[3], &vaddr))
			return cmd_error(CMD_ERR_NUM, NULL);
		
		size_t size;
		if (argv[4]) {		// if size is provided explicitly
			if (parse_number(argv[4], &size))
				return cmd_error(CMD_ERR_NUM, NULL);
		} else
			size = 1;

		return set_mappings(vaddr, size, perm);
	}

	return cmd_error(CMD_ERR_OPE, operation);
}


/***** Kernel monitor command interpreter *****/

#define WHITESPACE "\t\r\n "		// horizontal tab, carriage return, new line, space
									// four characters in total
#define MAXARGS 16					// a program recieves 16 command line arguments at most.

static int
runcmd(char *buf, struct Trapframe *tf)
{
	int argc;
	char *argv[MAXARGS];
	int i;

	// Parse the command buffer into whitespace-separated arguments.
	// This part of code doesn't cerate any new string. It set all
	// whitespaces to '\0' and store pointers to each word of the buf
	// string in argv, which is really tricky but efficient.
	argc = 0;
	argv[argc] = 0;
	while (1) {
		// gobble whitespace
		while (*buf && strchr(WHITESPACE, *buf))	// set all whitespaces to zero until encounting a non-whitespace character
			*buf++ = 0;
		if (*buf == 0)		// end of string in buf
			break;

		// save and scan past next arg
		if (argc == MAXARGS-1) {
			cprintf("Too many arguments (max %d)\n", MAXARGS);
			return 0;
		}
		argv[argc++] = buf;
		while (*buf && !strchr(WHITESPACE, *buf))	// increase buf until the end of string or a whitespace
			buf++;
	}
	argv[argc] = 0;

	// Lookup and invoke the command
	if (argc == 0)
		return 0;
	for (i = 0; i < ARRAY_SIZE(commands); i++) {
		if (strcmp(argv[0], commands[i].name) == 0)
			return commands[i].func(argc, argv, tf);
	}
	cprintf("Unknown command '%s'\n", argv[0]);
	return 0;
}

void
monitor(struct Trapframe *tf)
{
	char *buf;

	cprintf("Welcome to the JOS kernel monitor!\n");
	cprintf("Type 'help' for a list of commands.\n");

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
