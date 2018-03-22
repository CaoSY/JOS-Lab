/* See COPYRIGHT for copyright information. */

#ifndef JOS_KERN_PMAP_H
#define JOS_KERN_PMAP_H
#ifndef JOS_KERNEL
# error "This is a JOS kernel header; user programs should not #include it"
#endif

#include <inc/memlayout.h>
#include <inc/assert.h>

#define npages_in_4GB	(1<<(32-PGSHIFT))
#define DWORD_SIZE		4		// four bytes per dword
#define DOWRD_SHIFT		2		// log2(DWORD_SIZE)
#define ndwords_in_4GB	(1<<(32-DOWRD_SHIFT))
#define DOWRD_NUM(la)	(((uintptr_t) (la)) >> DOWRD_SHIFT)

extern char bootstacktop[], bootstack[];

extern struct PageInfo *pages;
extern size_t npages;

extern pde_t *kern_pgdir;


/* This macro takes a kernel virtual address -- an address that points above
 * KERNBASE, where the machine's maximum 256MB of physical memory is mapped --
 * and returns the corresponding physical address.  It panics if you pass it a
 * non-kernel virtual address.
 */
#define PADDR(kva) _paddr(__FILE__, __LINE__, kva)

static inline physaddr_t
_paddr(const char *file, int line, void *kva)
{
	if ((uint32_t)kva < KERNBASE)
		_panic(file, line, "PADDR called with invalid kva %08lx", kva);
	return (physaddr_t)kva - KERNBASE;
}

/* This macro takes a physical address and returns the corresponding kernel
 * virtual address.  It panics if you pass an invalid physical address. */
#define KADDR(pa) _kaddr(__FILE__, __LINE__, pa)

static inline void*
_kaddr(const char *file, int line, physaddr_t pa)
{
	if (PGNUM(pa) >= npages)
		_panic(file, line, "KADDR called with invalid pa %08lx", pa);
	return (void *)(pa + KERNBASE);
}


enum {
	// For page_alloc, zero the returned physical page.
	ALLOC_ZERO = 1<<0,
};

void	mem_init(void);

void	page_init(void);
struct PageInfo *page_alloc(int alloc_flags);
void	page_free(struct PageInfo *pp);
int	page_insert(pde_t *pgdir, struct PageInfo *pp, void *va, int perm);
void	page_remove(pde_t *pgdir, void *va);
struct PageInfo *page_lookup(pde_t *pgdir, void *va, pte_t **pte_store);
void	page_decref(struct PageInfo *pp);

void	tlb_invalidate(pde_t *pgdir, void *va);

static inline physaddr_t
page2pa(struct PageInfo *pp)
{
	return (pp - pages) << PGSHIFT;
}

static inline struct PageInfo*
pa2page(physaddr_t pa)
{
	if (PGNUM(pa) >= npages)
		panic("pa2page called with invalid pa");
	return &pages[PGNUM(pa)];
}

static inline void*
page2kva(struct PageInfo *pp)
{
	return KADDR(page2pa(pp));
}

pte_t *pgdir_walk(pde_t *pgdir, const void *va, int create);


/*
 * Some concept in buddy system
 * The minimal memory unit is a page(4KB).
 * 2^order pages is allocated each time.
 * The N-th order buddy page is the buddy of a page which is the
 * first page of 2^order contiguous pages. Thus the buddy's order
 * of a page can not be arbitrarily large. For eaxmple, (110)_2 page
 * has 2-th order buddy page at most.
 * Another trick we should point out is that if bit-N(counting from
 * left and N starts at zero) is the first non-zero bit of index.
 * Then this page has N-th buddy page at most. And by toggling bit-N,
 * we get the index of its buddy page.
 * If a page is the first page of 2^N contiguous pages, we call
 * the page is N-th order free.
 */

#define MAX_BUDDY_ORDER	10	// the biggest guaranteed memory is 2^10 pages, i.e. 4MB

struct PageInfo * buddy_is_free(struct PageInfo *pp, int order);
void buddy_tree_init(struct PageInfo *pp_free);
struct PageInfo *buddy_alloc_page(int alloc_flags, int order);
void buddy_free_page(struct PageInfo *pp, int order);

/*
 * This macro takes a PageInfo pointer and returns its index in the pages array.
 */
#define page2index(pp)	((pp) - pages)

#endif /* !JOS_KERN_PMAP_H */
