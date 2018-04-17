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
int mon_continue(int argc, char **argv, struct Trapframe *tf);
int mon_stepi(int argc, char **argv, struct Trapframe *tf);

#define CMD_HELP_HELP_STR       "\
-SYNOPSIS:\n\
    help {list | command name}\n\
-DESCRIPTION:\n\
    list: display all help information of all commands.\n\
    command name: display help information of given name\n"

#define CMD_KERNINFO_HELP_STR   "    Display information about the kernel\n"

#define CMD_BACKTRACE_HELP_STR  "    Display the current call stack\n"

#define CMD_CONTINUE_HELP_STR   "\
    Continue executing the program that caused a break point or debug\n\
    interrupt.\n"

#define CMD_STEPI_HELP_STR      "\
    Execute next intruction of the program that caused a break point or\n\
    debug interrupt. Then a debug interrupt is raised and execution is\n\
    trapped into kernel monitor again.\n"

#endif	// !JOS_KERN_MONITOR_H
