#ifndef JOS_INC_SPINLOCK_H
#define JOS_INC_SPINLOCK_H

#include <inc/types.h>

// Comment this to disable spinlock debugging
#define DEBUG_SPINLOCK

// Mutual exclusion lock.
struct spinlock {
	unsigned locked;       // Is the lock held?

#ifdef DEBUG_SPINLOCK
	// For debugging:
	char *name;            // Name of lock.
	struct CpuInfo *cpu;   // The CPU holding the lock.
	uintptr_t pcs[10];     // The call stack (an array of program counters)
	                       // that locked the lock.
#endif
};

void __spin_initlock(struct spinlock *lk, char *name);
void spin_lock(struct spinlock *lk);
void spin_unlock(struct spinlock *lk);

#define spin_initlock(lock)   __spin_initlock(lock, #lock)

extern struct spinlock kernel_lock;
extern struct spinlock page_allocator_lock;
extern struct spinlock console_lock;
extern struct spinlock scheduler_lock;
extern struct spinlock IPC_lock;

static inline void
lock_kernel(void)
{
	spin_lock(&kernel_lock);
}

static inline void
unlock_kernel(void)
{
	spin_unlock(&kernel_lock);

	// Normally we wouldn't need to do this, but QEMU only runs
	// one CPU at a time and has a long time-slice.  Without the
	// pause, this CPU is likely to reacquire the lock before
	// another CPU has even been given a chance to acquire it.
	asm volatile("pause");
}

static inline void
lock_page_allocator(void)
{
	spin_lock(&page_allocator_lock);
}

static inline void
unlock_page_allocator(void)
{
	spin_unlock(&page_allocator_lock);

	// refer to unlock_kernel to figure out why we need this instruction.
	asm volatile("pause");
}

static inline void
lock_console(void)
{
	spin_lock(&console_lock);
}

static inline void
unlock_console(void)
{
	spin_unlock(&console_lock);

	// refer to unlock_kernel to figure out why we need this instruction.
	asm volatile("pause");
}

static inline void
lock_scheduler(void)
{
	spin_lock(&scheduler_lock);
}

static inline void
unlock_scheduler(void)
{
	spin_unlock(&scheduler_lock);

	// refer to unlock_kernel to figure out why we need this instruction.
	asm volatile("pause");
}

static inline void
lock_IPC(void)
{
	spin_lock(&IPC_lock);
}

static inline void
unlock_IPC(void)
{
	spin_unlock(&IPC_lock);

	// refer to unlock_kernel to figure out why we need this instruction.
	asm volatile("pause");
}

#endif
