#ifndef JOS_KERN_MONITOR_H
#define JOS_KERN_MONITOR_H
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif

struct Trapframe;

// Activate the kernel monitor,
// optionally providing a trap frame indicating the current state
// (NULL if none).
void monitor(struct Trapframe *tf);

// Functions implementing monitor commands.
int mon_help(int argc, char **argv, struct Trapframe *tf);
int mon_kerninfo(int argc, char **argv, struct Trapframe *tf);
int mon_backtrace(int argc, char **argv, struct Trapframe *tf);
int mon_showmappings(int argc, char **argv, struct Trapframe *tf);

#define CMD_HELP_HELP_STR       "SYNOPSIS: help {list | command name}\n\
    list: display all help information of all commands.\n\
    command name: display help information of given name\n"
#define CMD_KERNINFO_HELP_STR   "Display information about the kernel\n"
#define CMD_BACKTRACE_HELP_STR  "Display the current call stack\n"
#define CMD_SHOWMAPPINGS_HELP_STR   "SYNOPSIS: showmappings LOWER_ADDR UPPER_ADDR\n\
    Display pages mappings in [LOWER_ADDR, UPPER_ADDR).\n\
    LOWER_ADDR/UPPER_ADDR will be rounded down to be aligned in 4KB(i.e. page size).\n\
    Addresses should be 32-bit unsigned integers.\n"

#endif	// !JOS_KERN_MONITOR_H
