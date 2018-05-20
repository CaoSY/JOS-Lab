// implement fork from user space

#include <inc/string.h>
#include <inc/lib.h>

// PTE_COW marks copy-on-write page table entries.
// It is one of the bits explicitly allocated to user processes (PTE_AVAIL).
#define PTE_COW		0x800

//
// Custom page fault handler - if faulting page is copy-on-write,
// map in our own private writable copy.
//
static void
pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t err = utf->utf_err;
	int r;

	// Check that the faulting access was (1) a write, and (2) to a
	// copy-on-write page.  If not, panic.
	// Hint:
	//   Use the read-only page table mappings at uvpt
	//   (see <inc/memlayout.h>).

	// LAB 4: Your code here.
	if (!(err & FEC_WR && uvpt[PGNUM(addr)] & PTE_COW))
		panic("pgfault failed.\n");

	// Allocate a new page, map it at a temporary location (PFTEMP),
	// copy the data from the old page to the new page, then move the new
	// page to the old page's address.
	// Hint:
	//   You should make three system calls.

	// LAB 4: Your code here.
	if ((r = sys_page_alloc(0,(void *)PFTEMP, PTE_U | PTE_W | PTE_P)) < 0)
		panic("Failed to allocate a new page: %e\n", r);

	memcpy((void *)PFTEMP, ROUNDDOWN(addr, PGSIZE), PGSIZE);

	if ((r = sys_page_map(0, (void *)PFTEMP, 0, ROUNDDOWN(addr, PGSIZE), PTE_U | PTE_W | PTE_P)) < 0)
		panic("Failed to map new page: %e\n", r);
	
	if ((r = sys_page_unmap(0, (void *)PFTEMP)) < 0)
		panic("Faile to unmap PFTEMP: %e\n", r);

	// panic("pgfault not implemented");
}

//
// Map our virtual page pn (address pn*PGSIZE) into the target envid
// at the same virtual address.  If the page is writable or copy-on-write,
// the new mapping must be created copy-on-write, and then our mapping must be
// marked copy-on-write as well.  (Exercise: Why do we need to mark ours
// copy-on-write again if it was already copy-on-write at the beginning of
// this function?)
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int
duppage(envid_t envid, unsigned pn)
{
	int r;

	// LAB 4: Your code here.
	// panic("duppage not implemented");
	void *addr = (void *)(pn*PGSIZE);
	int perm = PGOFF(uvpt[pn]) & PTE_SYSCALL;

	if (perm & (PTE_W | PTE_COW)) {
		perm |= PTE_COW;
		perm &= ~PTE_W;

		if ((r = sys_page_map(0, addr, envid, addr, perm)) < 0)
			panic("Failed to duppage on child enviroment: %e\n", r);
		
		if ((r = sys_page_map(0, addr, 0, addr, perm)) < 0)
			panic("Failed to duppage on self enviroment: %e\n", r);
	} else {
		if ((r = sys_page_map(0, addr, envid, addr, perm)) < 0)
			panic("Failed to duppage on child enviroment: %e\n", r);
	}
	
	return 0;
}

//
// duppage for share-fork
//
// Returns: 0 on success, < 0 on error.
// It is also OK to panic on error.
//
static int 
sduppage(envid_t envid, unsigned pn, int cow_enabled)
{

	int r;

	void *addr = (void *)(pn * PGSIZE);
	int perm = PGOFF(uvpt[pn]) & PTE_SYSCALL;

	if (cow_enabled && (perm & PTE_W)) {
		perm |= PTE_COW;
		perm &= ~PTE_W;

		if ((r = sys_page_map(0, addr, envid, addr, perm)) < 0)
			panic("sduppage: Failed to duppage on child enviroment: %e\n", r);

		if ((r = sys_page_map(0, addr, 0, addr, perm)) < 0)
			panic("sduppage: Failed to duppage on self enviroment: %e\n", r);
	} else {
		if ((r = sys_page_map(0, addr, envid, addr, perm)) < 0)
			panic("sduppage: Failed to duppage on child enviroment: %e\n", r);
	}

	return 0;
}

//
// User-level fork with copy-on-write.
// Set up our page fault handler appropriately.
// Create a child.
// Copy our address space and page fault handler setup to the child.
// Then mark the child as runnable and return.
//
// Returns: child's envid to the parent, 0 to the child, < 0 on error.
// It is also OK to panic on error.
//
// Hint:
//   Use uvpd, uvpt, and duppage.
//   Remember to fix "thisenv" in the child process.
//   Neither user exception stack should ever be marked copy-on-write,
//   so you must allocate a new page for the child's user exception stack.
//
envid_t
fork(void)
{
	int r;
	// LAB 4: Your code here.
	// panic("fork not implemented");
	set_pgfault_handler(pgfault);
	
	// envid is different in parent and child process
	envid_t envid = sys_exofork();
	if (envid == 0)	{	// if the process is the child process
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}

	// copy address map
	for (size_t i = 0; i < NPDENTRIES; ++i) {
		if (uvpd[i] & PTE_P) {
			for (size_t j = 0; j < NPTENTRIES; ++j) {
				size_t pn = i * NPTENTRIES + j;
				
				if (pn == PGNUM(USTACKTOP))
					goto COPY_END;

				if (uvpt[pn] & PTE_P)
					duppage(envid, pn);
			}
		}
	}
COPY_END:

	if ((r = sys_page_alloc(envid, (void *)(UXSTACKTOP-PGSIZE), PTE_U | PTE_W | PTE_P)) < 0)
		panic("Failed to allocate page for child's exception stack: %e", r);

	extern void _pgfault_upcall(void);
	if ((r = sys_env_set_pgfault_upcall(envid, _pgfault_upcall)) < 0)
		panic("Failed to set page fault hanlder for child: %e\n", r);

	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
		panic("Failed to set child's status as ENV_RUNNABLE", r);

	return envid;
}

// Challenge!
int
sfork(void)
{
	//panic("sfork not implemented");
	int r;

	//LAB 4: Your code here.
	set_pgfault_handler(pgfault);
	
	envid_t envid = sys_exofork();
	if (envid < 0)
		panic("sys_exofork: %e", envid);

	if (envid == 0) {
		thisenv = &envs[ENVX(sys_getenvid())];
		return 0;
	}

	bool stackarea = true;
	for (uint32_t addr = USTACKTOP - PGSIZE; addr >= UTEXT; addr -= PGSIZE) {
		if ((uvpd[PDX(addr)] & PTE_P) && (uvpt[PGNUM(addr)] & PTE_P))
			sduppage(envid, PGNUM(addr), stackarea);
		else
			stackarea = false;
	}

	if ((r = sys_page_alloc(envid, (void *)(UXSTACKTOP - PGSIZE), PTE_U | PTE_W | PTE_P)) < 0)
		panic("sfork: Failed to allocate page for child's exception stack: %e", r);

	extern void _pgfault_upcall(void);
	if ((r = sys_env_set_pgfault_upcall(envid, _pgfault_upcall)) < 0)
		panic("sfork: Failed to set page fault hanlder for child: %e\n", r);

	if ((r = sys_env_set_status(envid, ENV_RUNNABLE)) < 0)
		panic("sfork: Failed to set child's status as ENV_RUNNABLE", r);

	return envid;
	//return -E_INVAL;
}