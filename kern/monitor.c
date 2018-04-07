// Simple command-line kernel monitor useful for
// controlling the kernel and exploring the system interactively.

#include <inc/stdio.h>
#include <inc/string.h>
#include <inc/memlayout.h>
#include <inc/assert.h>
#include <inc/x86.h>

#include <kern/console.h>
#include <kern/monitor.h>
#include <kern/kdebug.h>
#include <kern/trap.h>

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
};


/***** Implementations of basic kernel monitor commands *****/

int
mon_help(int argc, char **argv, struct Trapframe *tf)
{
	// help {list | command name}
	char *operation;
	if ((operation = argv[1]) == 0) {
		cprintf(CMD_HELP_HELP_STR);
		return 0;
	}

	if (argc > 2) {
		cprintf("E: Wrong arguments! type 'help help' for usage.\n");
		return 0;
	}

	if (strcmp(operation, "list") == 0) {
		for (int i = 0; i < ARRAY_SIZE(commands); i++)
			cprintf("%s\n%s\n", commands[i].name, commands[i].desc);
		return 0;
	}


	bool cmd_found = false;
	for (int i = 0; i < ARRAY_SIZE(commands); i++) {
		if (strcmp(operation, commands[i].name) == 0) {
			cprintf("%s\n%s\n", commands[i].name, commands[i].desc);
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

	if (tf != NULL)
		print_trapframe(tf);

	while (1) {
		buf = readline("K> ");
		if (buf != NULL)
			if (runcmd(buf, tf) < 0)
				break;
	}
}
