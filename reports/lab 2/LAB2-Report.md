---
# export_on_save:
#  ebook: "html"

ebook:
  title: "JOS Lab Report"
  author: "Shuyang Cao"
  language: English
  include_toc: true
  html:
    cdn: true
---

```python {cmd=true hide=true run_on_save=true output="html" id="global"}
import time
from functools import reduce

# global variables
labnumber = 2
author = "Shuyang Cao"
createDate = "Mar 24 2018"
createTime = "10:34:00"

KEYS = locals().keys()
var_tobe_check = ['labnumber', 'createDate', 'createTime']
not_in_KEYS = [c not in KEYS for c in var_tobe_check]
if reduce(lambda x,y: x or y, not_in_KEYS) :
    for (varTmp, not_in_key) in zip(var_tobe_check, not_in_KEYS) :
        print('<p style="color:red;font-size:4em">{_var_} is not defined</p>'.format(_var_=varTmp))
    raise NameError('At least ones of labnumber, createDate and createTime is not defined')

title = "Lab {_number} Report".format(_number=labnumber)
currentTime = time.localtime()
updateDate = time.strftime("%b %d %Y", currentTime)
updateTime = time.strftime("%H:%M:%S", currentTime)
```

```python {cmd=true hide=true run_on_save=true output="html" continue="global"}

ctStyle = 'display:inline-block;width:50%;text-align:left'
ctSpan = '<span style="{_style}">Created: {_date} {_time}</span>'
ctSpan = ctSpan.format(_style=ctStyle, _date=createDate, _time=createTime)

mtStyle = 'display:inline-block;width:50%;text-align:right'
mtSpan = '<span style="{_style}">Last Updated: {_date} {_time}</span>'
mtSpan = mtSpan.format(_style=mtStyle, _date=updateDate, _time=updateTime)

header = '<p>{_ct}{_mt}</p>'.format(_ct=ctSpan, _mt=mtSpan)


titleStyle = "font-style:italic; font-family:Times; font-size:4em"
titleH1 = '<h1 style="{_style}"><center>{_title}</center></h1>'
titleH1 = titleH1.format(_style=titleStyle, _title=title)

authorStyle = "font-style:italic; font-family:Times; font-size:1.2em"

authorDiv = '<div style="{_style}"><center>{_author}</center><center>{_updateDate}</center></div>'
authorDiv = authorDiv.format(_style=authorStyle, _author=author, _updateDate=updateDate)

print(header)
print(titleH1)
print(authorDiv)
```

[TOC]

## Physical Page Managements

JOS maps the last 256MB of 4GB linear address space to its physcial address using a simple formula `(uintptr_t)la - KERNBASE`. In `mem_init()`, JOS allocates a `npages` long array to store some information about each page. Macros `PADDR(kva)` and `KADDR(pa)` as well as functions `page2pa(struct PageInfo *)`, `pa2page(physaddr_t)` and `page2kva(struct PageInfo *)` are used to transform addresses among physical address, linear address and pointer to PageInfo.

Through `PageInfo.pp_link`, JOS establishes a linked list to trace free pages in the PageInfo array, as shown below.

<center>

```ditaa {cmd=true args=["-E"]}
page N    +--------------------+
          |      pp_link       | -----\
          +--------------------+      |
          |      pp_ref        |      |
          +--------------------+      |
                                      |
                    /-----------------/
                    |
                    v
page N-1  +--------------------+
          |      pp_link       | -----\
          +--------------------+      |
          |      pp_ref        |      |
          +--------------------+      |
                                      |
                    .
                    .
                    .

                                      |
                    /-----------------/
                    |
                    v
page 1    +--------------------+
          |      pp_link       | -----\
          +--------------------+      |
          |      pp_ref        |      |
          +--------------------+      |
                                      |
                    /-----------------/
                    |
                    v
page 0    +--------------------+
          |   pp_link(NULL)    |
          +--------------------+
          |      pp_ref        |
          +--------------------+
```
</center>

In `page_init()`, JOS alos initializes the PageInfo array, rules out those pages whose corresponding physical memory are used from JOS's free page list. The physical memory after `page_init()` is done is shown below.

<center>

```ditaa {cmd=true args=["-E"]}
128MB -------------> +----------=---------+
                     |                    |
                     :                    :
                     |                    |
                     |                    |
boot_alloc(0) -----> +--------------------+
                     |                    |
                     |  used by kernel    |
                     |                    |
0x00100000(1MB) ---> +--------------------+
                     |     BIOS ROM       |
0x000F0000(960KB) -> +--------------------+
                     |   16-bit devices,  |
                     |   expansions ROMs  |
0x000C0000(768KB) -> +--------------------+
                     |    VGA Display     |
0x000A0000(640KB) -> +--------------------+
                     |                    |
                     |     Low Memory     |
                     |                    |
0x00001000(4KB) ---> | ------=----------- |
                     |   page 0 is used   |
                     |      by BIOS       |
0x00000000 --------> +--------------------+
```
</center>

### Exercise 1

> Q : In the file kern/pmap.c, you must implement code for the following functions (probably in the order given).
> `boot_alloc()`
> `mem_init()` (only up to the call to check_page_free_list(1))
> `page_init()`
> `page_alloc()`
> page_free() 
> `check_page_free_list()` and `check_page_alloc()` test your physical page allocator. You should boot JOS and see whether check_page_alloc() reports success. Fix your code so that it passes. You may find it helpful to add your own assert()s to verify that your assumptions are correct.

The code modified are show in the following `git diff` log. Since `mem_init()` is not done after **Exercise 1**, the result is not shown here.

```git
diff --git a/kern/pmap.c b/kern/pmap.c
index 727ea68..88356e4 100644
--- a/kern/pmap.c
+++ b/kern/pmap.c
@@ -5,6 +5,7 @@
 #include <inc/error.h>
 #include <inc/string.h>
 #include <inc/assert.h>
+#include <inc/memlayout.h>
 
 #include <kern/pmap.h>
 #include <kern/kclock.h>
@@ -102,8 +103,15 @@ boot_alloc(uint32_t n)
 	// to a multiple of PGSIZE.
 	//
 	// LAB 2: Your code here.
+	result = nextfree;
+	nextfree = ROUNDUP(nextfree + n, PGSIZE);
+	if ((uint32_t)nextfree - KERNBASE > npages*PGSIZE) {
+		panic("Out of Memory!\n");
+		nextfree = result;
+		result = NULL;
+	}
 
-	return NULL;
+	return result;
 }
 
 // Set up a two-level page table:
@@ -148,6 +156,8 @@ mem_init(void)
 	// array.  'npages' is the number of physical pages in memory.  Use memset
 	// to initialize all fields of each struct PageInfo to 0.
 	// Your code goes here:
+	pages = (struct PageInfo *) boot_alloc(sizeof(struct PageInfo) * npages);
+	memset(pages, 0, sizeof(struct PageInfo)*npages);
 
 
 	//////////////////////////////////////////////////////////////////////
@@ -251,8 +261,110 @@ page_init(void)
 	// Change the code to reflect this.
 	// NB: DO NOT actually touch the physical memory corresponding to
 	// free pages!
+
+	// Keep in mind memset(pages, 0, sizeof(struct PageInfo)*npages); is 
+	// called before this function, so we can simply assume whichever
+	// field not set is zero.
+
+	/*
+	 * When page_init() is called, the usage of our physical memory is
+	 * shown in Fig.1. After our page table is initialized, our free
+	 * pages are stringed by a linked list, which looks like a stack,
+	 * as shown in Fig.2.
+	 * 
+	 * Figure 1:
+	 * 
+	 *       256MB -------------> +--------------------+
+	 *                            |                    |
+	 *                            |                    |
+	 *                            /\/\/\/\/\/\/\/\/\/\/\
+	 * 
+	 *       boot_alloc(0) -----> /\/\/\/\/\/\/\/\/\/\/\
+	 *                            |                    |
+	 *                            |  used by kernel    |
+	 *                            |                    |
+	 *       0x00100000(1MB) ---> +--------------------+
+	 *                            |     BIOS ROM       |
+	 *       0x000F0000(960KB) -> +--------------------+
+	 *                            |   16-bit devices,  |
+	 *                            |   expansions ROMs  |
+	 *       0x000C0000(768KB) -> +--------------------+
+	 *                            |    VGA Display     |
+	 *       0x000A0000(640KB) -> +--------------------+
+	 *                            |                    |
+	 *                            |     Low Memory     |
+	 *                            |                    |
+	 *       0x00001000(4KB) ---> |  ~~~~~~~~~~~~~~~~~ |
+	 *                            |   page 0 is used   |
+	 *                            |      by BIOS       |
+	 *       0x00000000 --------> +--------------------+
+	 * 
+	 * 
+	 * Figure 2:
+	 * 
+	 *       page N    +--------------------+
+	 *                 |      pp_link       | -----\
+	 *                 +--------------------+      |
+	 *                 |      pp_ref        |      |
+	 *                 +--------------------+      |
+	 *                                             |
+	 *                           /-----------------/
+	 *                           |
+	 *                           v
+	 *       page N-1  +--------------------+
+	 *                 |      pp_link       | -----\
+	 *                 +--------------------+      |
+	 *                 |      pp_ref        |      |
+	 *                 +--------------------+      |
+	 *                                             |
+	 *                           .
+	 *                           .
+	 *                           .
+	 * 
+	 *                           /-----------------/
+	 *                           |
+	 *                           v
+	 *       page 1    +--------------------+
+	 *                 |      pp_link       | -----\
+	 *                 +--------------------+      |
+	 *                 |      pp_ref        |      |
+	 *                 +--------------------+      |
+	 *                                             |
+	 *                           /-----------------/
+	 *                           |
+	 *                           v
+	 *       page 0    +--------------------+
+	 *                 |    pp_link=NULL    |
+	 *                 +--------------------+
+	 *                 |      pp_ref        |
+	 *                 +--------------------+
+	 * 
+	 */
+
+	page_free_list = NULL;
+
+	pages[0].pp_ref = 1;		// mark physical page 0 as in use
 	size_t i;
-	for (i = 0; i < npages; i++) {
+	
+	for (i = 1; i < npages_basemem; ++i) {
+		// The rest of base memory, [PGSIZE, npages_basemem * PGSIZE)
+		// is free.
+		pages[i].pp_ref = 0;
+		pages[i].pp_link = page_free_list;
+		page_free_list = &pages[i];
+	}
+
+	size_t kernel_end_page = ((uint32_t)(boot_alloc(0)) - KERNBASE)/PGSIZE;
+	for (; i < kernel_end_page; ++i) {
+		// IO hole and kernel physical memory reside consecutively
+		// in physcial memory. So initialize their corresponding 
+		// page entries in one loop
+		pages[i].pp_ref = 1;
+		pages[i].pp_link = NULL;
+	}
+	
+	for (; i < npages; ++i) {
+		// The rest of extended memory are not used
 		pages[i].pp_ref = 0;
 		pages[i].pp_link = page_free_list;
 		page_free_list = &pages[i];
@@ -274,8 +386,17 @@ page_init(void)
 struct PageInfo *
 page_alloc(int alloc_flags)
 {
-	// Fill this function in
-	return 0;
+	if (!page_free_list)	// if page_free_list == NULL
+		return NULL;
+	
+	struct PageInfo *page_allocated = page_free_list;
+	page_free_list = page_free_list->pp_link;
+	page_allocated->pp_link = NULL;
+
+	if (alloc_flags & ALLOC_ZERO)
+		memset(page2kva(page_allocated), 0, PGSIZE);
+	
+	return page_allocated;
 }
 
 //
@@ -288,6 +409,18 @@ page_free(struct PageInfo *pp)
 	// Fill this function in
 	// Hint: You may want to panic if pp->pp_ref is nonzero or
 	// pp->pp_link is not NULL.
+
+	if (pp->pp_link)		// pp->pp_link is not NULL, double free
+		panic("Double free page %u, pvaddr:%x, ppaddr:%x\n",\
+			(uint32_t)(pp-pages), page2kva(pp), page2pa(pp));
+	
+	if (pp->pp_ref)			// pp->pp_ref is nonzero
+		panic("Free a page in use, pnum:%u, pvaddr:%x, ppaddr:%x\n",\
+			(uint32_t)(pp-pages), page2kva(pp), page2pa(pp));
+	
+	pp->pp_link = page_free_list;
+	page_free_list = pp;
+	return;
 }
 
 //

```

## Virtual Memory

In x86 terminology, a ***virtual address*** consists of a ***segment selector*** and an ***offset*** within the segment. A ***linear address*** is what you get after segment translation but before page translation. A ***physical address*** is what you finally get after both segment and page translation and what ultimately goes out on the hardware bus to your RAM. ( [Ref](https://pdos.csail.mit.edu/6.828/2017/labs/lab2/) )

A C pointer is the "offset" component of the virtual address. In boot/boot.S, we installed a Global Descriptor Table (GDT) that effectively disabled segment translation by setting all segment base addresses to 0 and limits to 0xffffffff. Hence the "selector" has no effect and the linear address always equals the offset of the virtual address. ( [Ref](https://pdos.csail.mit.edu/6.828/2017/labs/lab2/) )

The figure below gives a more straightforward explanation.

<center>

```ditaa {cmd=true args=["-E"]}
           Selector  +--------------+         +-----------+
          ---------->|              |         |           |
                     | Segmentation |         |  Paging   |
Software             |              |-------->|           |---------->  RAM
            Offset   |  Mechanism   |         | Mechanism |
          ---------->|              |         |           |
                     +--------------+         +-----------+
            Virtual                   Linear                Physical
```
</center>

### Exercise 3

> Q : Use the xp command in the QEMU monitor and the x command in GDB to inspect memory at corresponding physical and virtual addresses and make sure you see the same data.

In `entry_pgdir`, the linear address in the range 0xF0000000 through 0xF0400000 is mapped to physical addresses 0x00000000 through 0x00400000. So we check ten DWORDss beginning from 0x100000 in physical address or 0xf0100000 in linear address, i.e., the first ten DWORDs in kernel. As shown below, they are the same.

```bash
QEMU 2.5.0 monitor - type 'help' for more information
(qemu) xp/10x 0x100000
0000000000100000: 0x1badb002 0x00000000 0xe4524ffe 0x7205c766
0000000000100010: 0x34000004 0x6000b812 0x220f0011 0xc0200fd8
0000000000100020: 0x0100010d 0xc0220f80
(qemu) x/10x 0xf0100000
f0100000: 0x1badb002 0x00000000 0xe4524ffe 0x7205c766
f0100010: 0x34000004 0x6000b812 0x220f0011 0xc0200fd8
f0100020: 0x0100010d 0xc0220f80
(qemu) 
```

### Question 1

> Q : Assuming that the following JOS kernel code is correct, what type should variable x have, uintptr_t or physaddr_t? 
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`mystery_t x;`
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`char* value = return_a_pointer();`
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`*value = 10;`
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`x = (mystery_t) value;`

`x` is a pointer in C language, which should be a virtual address, namely, `uintptr_t`.

### Exercise 4

> Q : In the file kern/pmap.c, you must implement code for the following functions.
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`pgdir_walk()`
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`boot_map_region()`
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`page_lookup()`
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`page_remove()`
> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;`page_insert()`
> check_page(), called from mem_init(), tests your page table management routines. You should make sure it reports success before proceeding. 

* **What is [*Two-level Page Table*](https://en.wikipedia.org/wiki/Page_table#Multilevel_page_table) ?**

To finish this exercise, we need to how the page system works in JOS. To mantain the map between linear address and physical address while avoiding using too much memory on page tables, JOS adopts a two-level page system.

In a two-level page system, a linear address is decomposed as below.

<center>

```ditaa {cmd=true args=["-E"]}
/--=----31 - 22---=---\ /---=---21 - 12--=---\ /--=----- 11 - 0 --=------\
|                     | |                    | |                         |
+----------------------+----------------------+--------------------------+
| Page Directory Index |   Page Table Index   |    Offset within Page    |
+----------------------+----------------------+--------------------------+
|                     | |                    | |                         |
+--=----PDX(la)---=---/ \---=---PTX(la)--=---/ \--=-----PGOFF(la)-=------/

|                                            |
\----=------------PGNUM(la)-----------=------/
```
</center>

To be transformed to a physical address, a linear address goes through the following procedure.

<center>

```ditaa {cmd=true args=["-E"]}
/--=----31 - 22---=---\ /---=---21 - 12--=---\ /--=----- 11 - 0 --=------\
|                     | |                    | |                         |
+----------------------+----------------------+--------------------------+
| Page Directory Index |   Page Table Index   |    Offset within Page    |
+----------------------+----------------------+--------------------------+
    |                              |                          | 
    |                              |                          |            4KB Page
    |                              |                          |      +------------------+
    |                              |                          |      |                  |
    |                              |                          |      |                  |
    |                              |        Page Table        |      |--------=---------|
    |                              |   +------------------+   \----->| Physcial Address |
    |                              |   |                  |          |--------=---------|
    |        Page Directory        |   |                  |          |                  |
    |     +------------------+     |   |                  |          |                  |
    |     |                  |     |   |                  |          |                  |
    |     |                  |     |   |                  |          |                  |
    |     |                  |     |   |                  |          |                  |
    |     |                  |     |   |                  |          |                  |
    |     |                  |     |   |                  |          |                  |
    |     |                  |     |   |-------=----------|          |                  |
    |     |                  |     \-->|    Table Entry   |--------->+------------------+
    |     |                  |         |--------=---------|
    |     |                  |         |                  |
    |     |--------=---------|         |                  |
    \---->| Directory Entry  |-------->+------------------+
          |--------=---------|
          |                  |
    /---->+------------------+
    |
    |
    |     +------------------+
    \-----|    CR3(PDBR)     |
          +------------------+
```
</center>

When referring to a place in memory, the hardware does this translation for us. What we need to do is to maintain this map properly, i.e., setting or resetting the entry correctly. In `pgdir_walk(pde_t *, const void *, int)`, we always give the page where a page table resides a loose security restriction. Because a page table may contain entries used by kernel and entries used by user programs at the same time. Thus we can't prevent user programs from accessing those entries.

The code modified is show in the following `git diff` log.

```git
diff --git a/kern/pmap.c b/kern/pmap.c
index 88356e4..34589aa 100644
--- a/kern/pmap.c
+++ b/kern/pmap.c
@@ -133,7 +133,7 @@ mem_init(void)
 	i386_detect_memory();
 
 	// Remove this line when you're ready to test this function.
-	panic("mem_init: This function is not finished\n");
+	//panic("mem_init: This function is not finished\n");
 
 	//////////////////////////////////////////////////////////////////////
 	// create initial page directory.
@@ -456,11 +456,38 @@ page_decref(struct PageInfo* pp)
 // Hint 3: look at inc/mmu.h for useful macros that mainipulate page
 // table and page directory entries.
 //
+// Hint 4: all pointers in C are virtual addresses but all addresses in
+// page dir and page table are physical address
+//
 pte_t *
 pgdir_walk(pde_t *pgdir, const void *va, int create)
 {
 	// Fill this function in
-	return NULL;
+	pde_t *pd_entry = &pgdir[PDX(va)];
+	pte_t *pgtable = NULL;
+
+	// if the relevant page table page exists
+	if ((physaddr_t)*pd_entry & PTE_P) {
+		pgtable = (pte_t *)(KADDR(PTE_ADDR(*pd_entry)));
+		return &pgtable[PTX(va)];
+	}
+	
+	// if the relevant page table doesn't exist and create == false
+	if (create == false)
+		return NULL;
+	
+	// allocate a new page table, use ALLOC_ZERO to clear the page
+	// page_alloc guarantees NULL is returned on allocation failure
+	struct PageInfo *new_page_info = page_alloc(ALLOC_ZERO);
+	if (new_page_info == NULL)
+		return NULL;
+	++(new_page_info->pp_ref);
+	
+	// pages of kernel and users may be allocated on one page table
+	// so it's convenient to give page table a user level privilige
+	*pd_entry = (pde_t)((uintptr_t)page2pa(new_page_info) | PTE_P | PTE_W | PTE_U);
+	pgtable = (pte_t *)page2kva(new_page_info);
+	return &pgtable[PTX(va)];
 }
 
 //
@@ -477,7 +504,16 @@ pgdir_walk(pde_t *pgdir, const void *va, int create)
 static void
 boot_map_region(pde_t *pgdir, uintptr_t va, size_t size, physaddr_t pa, int perm)
 {
-	// Fill this function in
+	for (size_t alloc_size = 0; alloc_size < size; alloc_size += PGSIZE) {
+		pte_t *pt_entry = pgdir_walk(pgdir, (void *)va, true);
+		
+		if (pt_entry == NULL)
+			panic("Page allocation failed!");
+		
+		*pt_entry = pa | perm | PTE_P;
+		va += PGSIZE;
+		pa += PGSIZE;
+	}
 }
 
 //
@@ -498,6 +534,13 @@ boot_map_region(pde_t *pgdir, uintptr_t va, size_t size, physaddr_t pa, int perm
 // frequently leads to subtle bugs; there's an elegant way to handle
 // everything in one code path.
 //
+// Corner-case answer: The hint actually remind us to consider such a condition
+// that the same pp is re-inserted at the same virtual address "va" and the "pp"
+// is only referred to by the "va" previously, namely, pp->pp_ref == 0. Under
+// such a condition, if we call page_remove() before we increment pp->ref,
+// page_remove() will free pp back to free_page_list so that our pte will point
+// to a free page. if we increment pp->pp_ref first, we can avoid this mistake.
+//
 // RETURNS:
 //   0 on success
 //   -E_NO_MEM, if page table couldn't be allocated
@@ -509,6 +552,16 @@ int
 page_insert(pde_t *pgdir, struct PageInfo *pp, void *va, int perm)
 {
 	// Fill this function in
+	pte_t *pt_entry = pgdir_walk(pgdir, va, true);
+	
+	if (pt_entry == NULL)
+		return -E_NO_MEM;
+
+	++(pp->pp_ref);
+	if (*pt_entry & PTE_P)
+		page_remove(pgdir, va);		// page_remove() will call tlb_invalidate()
+
+	*pt_entry = page2pa(pp) | perm | PTE_P;
 	return 0;
 }
 
@@ -527,7 +580,16 @@ struct PageInfo *
 page_lookup(pde_t *pgdir, void *va, pte_t **pte_store)
 {
 	// Fill this function in
-	return NULL;
+
+	pte_t *pt_entry = pgdir_walk(pgdir, va, false);
+
+	if (pt_entry == NULL)
+		return NULL;
+	
+	if (pte_store)	// if pte_store is not zero
+		*pte_store = pt_entry;
+
+	return pa2page(PTE_ADDR(*pt_entry));
 }
 
 //
@@ -549,6 +611,14 @@ void
 page_remove(pde_t *pgdir, void *va)
 {
 	// Fill this function in
+	pte_t *pt_entry;
+	struct PageInfo *mapped_page = page_lookup(pgdir, va, &pt_entry);
+	if (mapped_page == NULL)	// no physical page mapped, do nothing
+		return;
+	
+	page_decref(mapped_page);
+	*pt_entry = 0;
+	tlb_invalidate(pgdir, va);
 }
 
 //

```

## Kernel Address Space

In x86 page tables, we use permission bits to allow user programs only to access their own part of the address space. Otherwiese, bugs in user programs might overwrite kernel data, causing a crash or more subtle malfunction; user programs might also be able to steal other environments' private data.

Here we use `PTE_U` and `PTE_W` to set permission bits. Since the name of `PTE_W` may be a little confusing, we should keep in mind that when the processor is executing at supervisor level, all pages are addressable, i.e., readable and writable. `PTE_U` means user programs can access the corresponding memory. It effectively means the corresponding memory is user-readable. `PTE_W` means user program can write data into the corresponding memroy.

In lab 2, we are ask to set our permission bits as the following.

<center>

```ditaa {cmd=true args=["-E"]}
Virtual memory map:                                Permissions
                                                   kernel/user
4 Gig ---------->+--------------=---------------+
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 :   Remapped Physical Memory   :
                 |                              | RW/--
                 |                              |
                 |                              |
                 |                              |
KERNBASE, ------>+------------------------------+ 0xF0000000      --\
KSTACKTOP        |     CPU0's Kernel Stack      | RW/--  KSTKSIZE   |
                 |-------------=----------------|                   |
                 |      Invalid Memory (*)      | --/--  KSTKGAP    |
                 +------------------------------+                   :
                 |                              |
                 |                              |                 PTSIZE
                 |                              |
                 |                              |                   :
                 |                              |                   |
                 |                              |                   |
MMIOLIM -------->+------------------------------+ 0xEFC00000      --/
                 |                              |
                 :                              :
                 |                              |
UVPT       ----->+------------------------------+ 0xEF400000
                 |          RO PAGES            | R-/R-  PTSIZE
UPAGES     ----->+------------------------------+ 0xEF000000
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 |                              |
                 |                              |
0 ------------>  +---------------=--------------+
```
</center>

### Exercise 5

> Q : Fill in the missing code in mem_init() after the call to check_page().
> Your code should now pass the check_kern_pgdir() and check_page_installed_pgdir() checks.

In this exercise, we use `boot_map_region(pde_t *, uintptr_t, size_t, physaddr_t, int)` to mapp some linear addresses as required and then load `kern_pgdir` to `CR3` to replace `entry_pgdir`.

In this exercise we finish `mem_init()`, the test result is shown below.

```bash
***
*** Use Ctrl-a x to exit qemu
***
qemu-system-i386 -nographic -drive file=obj/kern/kernel.img,index=0,media=disk,format=raw -serial mon:stdio -gdb tcp::26000 -D qemu.log
6828 decimal is 15254 octal!
Physical memory: 131072K available, base = 640K, extended = 130432K
check_page_free_list() succeeded!
check_page_alloc() succeeded!
check_page() succeeded!
check_kern_pgdir() succeeded!
check_page_free_list() succeeded!
check_page_installed_pgdir() succeeded!
Welcome to the JOS kernel monitor!
Type 'help' for a list of commands.
K>
```

The code modified is shown in the following `git diff` log.

```git
diff --git a/kern/pmap.c b/kern/pmap.c
index 34589aa..f22a5e5 100644
--- a/kern/pmap.c
+++ b/kern/pmap.c
@@ -182,6 +182,7 @@ mem_init(void)
 	//      (ie. perm = PTE_U | PTE_P)
 	//    - pages itself -- kernel RW, user NONE
 	// Your code goes here:
+	boot_map_region(kern_pgdir, UPAGES, PTSIZE, PADDR((void *)pages), PTE_U);
 
 	//////////////////////////////////////////////////////////////////////
 	// Use the physical memory that 'bootstack' refers to as the kernel
@@ -194,6 +195,7 @@ mem_init(void)
 	//       overwrite memory.  Known as a "guard page".
 	//     Permissions: kernel RW, user NONE
 	// Your code goes here:
+	boot_map_region(kern_pgdir, KSTACKTOP-KSTKSIZE, KSTKSIZE, PADDR((void *)bootstack), PTE_W);
 
 	//////////////////////////////////////////////////////////////////////
 	// Map all of physical memory at KERNBASE.
@@ -203,6 +205,12 @@ mem_init(void)
 	// we just set up the mapping anyway.
 	// Permissions: kernel RW, user NONE
 	// Your code goes here:
+	// Actually 0xFFFFFFFF is 2^32-1. But you aren't able to represent 2^32
+	// in an integer on 32-bit system. So 0xFFFFFFFF-KERNBASE is one less
+	// than the size we want. Luckily, boot_map_region() will round up the
+	// for us and everything goes fine since 0xFFFFFFFF-KERNBASE is 
+	// representable on 32-bit system.
+	boot_map_region(kern_pgdir, KERNBASE, 0xFFFFFFFF-KERNBASE, (physaddr_t)0x0, PTE_W);
 
 	// Check that the initial page directory has been set up correctly.
 	check_kern_pgdir();
```

### Question 2

> Q : What entries (rows) in the page directory have been filled in at this point? What addresses do they map and where do they point?

| Entry | Base Virtual Address | Points to (logically) |
|:-------:|:----------------------:|:-----------------------:|
| [0x3BC, 0x3BD) | [0xEF000000, 0xEF400000) | `pages` array |
| [0x3BF, 0x3C0) | [0xEFFF8000, 0xF0000000) | bootstack |
| [0x3C0, 0x3FF) | [0xF0000000, 0x100000000) | kernel |

### Question 3

> Q : We have placed the kernel and user environment in the same address space. Why will user programs not be able to read or write the kernel's memory? What specific mechanisms protect the kernel memory?

We didn't toggle on the `PTE_U` bit in those page table entry that point to kernel's memroy. So once a user program wants to access memory that belongs to kernel, CPU will detect it and notify JOS.

### Question 4

> Q : What is the maximum amount of physical memory that this operating system can support? Why?

JOS can support 2GB memory at most. JOS use 4MB memory to store a PageInfo array. Each PageInfo struct consumes 8 Bytes, containing information of one 4KB page. So 2^19^ PageInfo structs can be stored at most, which record information of 2G memory.

### Question 5

> Q : How much space overhead is there for managing memory, if we actually had the maximum amount of physical memory? How is this overhead broken down?

If we actually had the maximum amout of physical memory, we need 4MB for the PageInfo array, 4KB for page directory and 4BM at most for page tables. The memory for the PageInfo array is hardcoded in JOS so its size doesn't change even though it's not fully used in lab 2 JOS, which only has 128M memory. The memroy for the page directory can't not be ruled out, either. But we can save our memory by allocating a page for a page table only when we really need it. It's the reason why we use a two-level paging system.

### Question 6

> Q : Revisit the page table setup in kern/entry.S and kern/entrypgdir.c. Immediately after we turn on paging, EIP is still a low number (a little over 1MB). At what point do we transition to running at an EIP above KERNBASE? What makes it possible for us to continue executing at a low EIP between when we enable paging and when we begin running at an EIP above KERNBASE? Why is this transition necessary?

After `jmp *%eax` in `entry.S`, we transition to running at an EIP above KERNBASE. In manully established page tables that reside in `entrypgdir.c`, we map 4MB from 0x00000000 and 4MB from KERNBASE in linear address space to 4MB from 0x000000 in physical memory address. Therefore, after enabling paging, kernel can still run at a low EIP.

We hope kernel to run at the top of 4GB linear address space. So we want to kernel transitions to running at an EIP above KERNBASE. Furthermore, we will use`kern_pgdir` and discar `entry_pgdir` after `mem_init()`. In `mem_init()/kern_pgdir`, we didn't map 4MB memory from 0x00000000 in linear address space to any physcial memory. So it's also necessary for JOS to run at an EIP above KERNBASE before we switch our page directory.

## Challenges

### Challenge 1

> We consumed many physical pages to hold the page tables for the KERNBASE mapping. Do a more space-efficient job using the PTE_PS ("Page Size") bit in the page directory entries.

For details about *4MB page* mechanism, you can refer to section 3.6, 3.7 and 10.9 in **INTEL 80386 PROGRAMMER'S REFERENCE MANUAL 1986**.

## Grade

Finally, we got our grade.

```bash
```

```python {cmd=true hide=true run_on_save=true output="html" continue="global"}

footerStyle = "width:100%;text-align:center;font-family:Times"
footerTemplate = '<footer style="{_style}">End of {_title}<br/>Email: <a mailto="caoshuyang1996@pku.edu.cn">caoshuyang@pku.edu.cn</a> GitHub: <a href="https://github.com/CaoSY/JOS-Lab" title="JOS Lab">JOS-Lab</a></footer>'

# print footer
print(footerTemplate.format(_style=footerStyle, _title=title))
```