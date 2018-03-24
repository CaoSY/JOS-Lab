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

<!-- @import "[TOC]" {cmd="toc" depthFrom=1 depthTo=6 orderedList=false} -->

<!-- code_chunk_output -->

* [Physical Page Managements](#physical-page-managements)
	* [Exercise 1](#exercise-1)
* [Virtual Memory](#virtual-memory)
	* [Exercise 3](#exercise-3)
	* [Question 1](#question-1)
	* [Exercise 4](#exercise-4)
* [Kernel Address Space](#kernel-address-space)
	* [Exercise 5](#exercise-5)
	* [Question 2](#question-2)
	* [Question 3](#question-3)
	* [Question 4](#question-4)
	* [Question 5](#question-5)
	* [Question 6](#question-6)
* [Challenges](#challenges)
	* [Challenge 1](#challenge-1)
	* [Challenge 2](#challenge-2)
	* [Challenge 3](#challenge-3)
	* [Challenge 4](#challenge-4)
* [Grade](#grade)

<!-- /code_chunk_output -->



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
    |                              |                          |         4KB Page Frame
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

For details about *4MB page* mechanism, you can refer to section 3.6, 3.7 and 10.9 in **INTEL 80386 PROGRAMMER'S REFERENCE MANUAL 1986**. Below is a brief explanation of it.

Since one entry corresponds to 4MB memory, we only need one-level page table, which means we discard page table and retain the page directory. Now we decompose linear addresses in a new and simpler way as show below.

<center>

```ditaa {cmd=true args=["-E"]}
/--=----31 - 22---=---\ /---=----------------21 - 0------------=---------\
|                     | |                                                |
+----------------------+-------------------------------------------------+
| Page Directory Index |              Offset within Page                 |
+----------------------+-------------------------------------------------+
|                     | |                                                |
+--=----PDX(la)---=---/ \---=--------------PGOFF(la)-=-------------------/

|                     |
\-=---PGNUM(la)-=-----/
```
</center>

The translation between linear addresses and physical addresses is simpler, too, as shown below.

<center>

```ditaa {cmd=true args=["-E"]}
/--=----31 - 22---=---\ /---=---------------21 - 0-----------------=-----\
|                     | |                                                |
+----------------------+-------------------------------------------------+
| Page Directory Index |             Offset within Page                  |
+----------------------+-------------------------------------------------+
    |                              |
    |                              |
    |                              |
    |                              |
    |                              |
    |                              |      4MB Page Frame
    |                              |   +------------------+
    |                              |   |                  |
    |        Page Directory        |   |                  |
    |     +------------------+     |   |                  |
    |     |                  |     |   |                  |
    |     |                  |     |   |                  |
    |     |                  |     |   |                  |
    |     |                  |     |   |                  |
    |     |                  |     |   |                  |
    |     |                  |     |   |-------=----------|
    |     |                  |     \-->| Physical Address |
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

To inform CPU that you are using big page, you need to toggle bits on in two places. The first one is the **PSE (page size extensions) flag**, which is bit 4 of CR4 (introduced in Pentium^®^ processor). The other is the **PS (page size)** in page directory entries. Again, the entries become simpler, too, as shown below.

<center>

```ditaa {cmd=true args=["-E"]}
              Page Directory Entry (4MByte Page)

31                    22 21                13 12  11    9  8   7   6   5   4   3   2   1   0
+-------------------------+--------------------+---+-------+---+---+---+---+---+---+---+---+---+
|                         |                    | P |       |   | P |   |   | P | P | U | R |   |
|    Page Base Address    |      Reserved      | A | Avail | G |   | D | A | C | W | / | / | P |
|                         |                    | T |       |   | S |   |   | D | T | S | W |   |
+-------------------------+--------------------+---+-------+---+---+---+---+---+---+---+---+---+
                                                 ^    ^      ^   ^   ^   ^   ^   ^   ^   ^   ^
                                                 |    |      |   |   |   |   |   |   |   |   |
Page Table Attribute Index-----------------------/    |      |   |   |   |   |   |   |   |   |
Available for system programmer's use-----------------/      |   |   |   |   |   |   |   |   |
Global page--------------------------------------------------/   |   |   |   |   |   |   |   |
Page size (1 indicates 4MBytes)----------------------------------/   |   |   |   |   |   |   |
Dirty----------------------------------------------------------------/   |   |   |   |   |   |
Accessed-----------------------------------------------------------------/   |   |   |   |   |
Cache disables---------------------------------------------------------------/   |   |   |   |
Write-through--------------------------------------------------------------------/   |   |   |
User/Supervisor----------------------------------------------------------------------/   |   |
Read/Write-------------------------------------------------------------------------------/   |
Present--------------------------------------------------------------------------------------/
```
</center>

Because when enabling or diabling large page sizes, the TLBs must be invalidated (flushed) after the **PSE flag** in control register **CR4** has been set or cleared. Otherwise, incorrect page translation might occur due to the processor using outdated page translation information stored in the TLBs. Further more, loading a new value into **CR3** is one method to flush all TLBs. Thus, We need to set **PSE flag** before we load new value into **CR3**.

Since tests in `mem_init()` are created with small page size concept in mind that are not suitable for testing 4MB page system, so I comment most of this tests to avoid JOS `panic()` and I did write new tests for 4MB page system. When switch JOS to big page mode, I didn't change the value of `PGSIZE` directly. Instead, I define a new macro named `BIG_PGSIZE` whose value is 4MB and modified other codes base on `BIG_PGSIZE`. By doing this, I didn't need to change macros for JOS memory layout, whose value are derived using `PGSIZE`. One thing we should keep in mind is that although I succeed in switching JOS to big page mode and all functions implemented in lab 1 and lab 2 work fine, we haven't finished the transition to big page mode yet. It's a coincidence that our memory layout design using small pages gets along well with big pages. If we want to implemente more complicated functions related to memory management, we need to re-design JOS's memory layout.

The result is shown below, JOS executes properly.

```bash
***
*** Use Ctrl-a x to exit qemu
***
qemu-system-i386 -nographic -drive file=obj/kern/kernel.img,index=0,media=disk,format=raw -serial mon:stdio -gdb tcp::26000 -D qemu.log
6828 decimal is 15254 octal!
Physical memory: 131072K available, base = 640K, extended = 130432K
check_page_free_list() succeeded!
check_page_free_list() succeeded!
Welcome to the JOS kernel monitor!
Type 'help' for a list of commands.
K> help list
help
-SYNOPSIS:
    help {list | command name}
-DESCRIPTION:
    list: display all help information of all commands.
    command name: display help information of given name

kerninfo
Display information about the kernel

backtrace
Display the current call stack

mappings
-SYNOPSIS:
    mappings {show laddr uaddr} | {clear vaddr [size==1]}
             | {set perm vaddr [size==1]}
-DESCRIPTION:
    show: display page mappings in [laddr, uaddr)
    clear: clear privilege of size(in page) pages from vaddr.
           The privilege is set to PTE_U == 0 and PTE_W == 0.
    set: set privilege of size(in page) pages from vaddr.
         The privilege 'perm' can be specified by a number or a
         two-character string containing 'u'(PTE_U), 'w'(PTE_W) or
         '-'(none)
    All addresses will be rounded down to page alignment.

dump
-SYNOPSIS:
    dump addr_type addr [size==1]
-DESCRIPTION:
    addr_type: '-v' for virtual address, '-p for physical address
    addr: beginning address
    size: memory size in dwords(4 bytes)

K>
```

The code modified for 4MB paging is shown in the following `git diff` log.

```git
diff --git a/inc/mmu.h b/inc/mmu.h
index 093c8a6..7f6df3f 100644
--- a/inc/mmu.h
+++ b/inc/mmu.h
@@ -26,25 +26,55 @@
 // To construct a linear address la from PDX(la), PTX(la), and PGOFF(la),
 // use PGADDR(PDX(la), PTX(la), PGOFF(la)).
 
+/*
+ * In big page table,
+ * a linear address 'la' has a two-part structure as follows"
+ * +-------10-------+--------------------22----------------+
+ * | Page Directory |          Offset within Page          |
+ * |      Index     |                                      |
+ * +----------------+--------------------------------------+
+ * \--BIG_PDX(la)--/ \-------------BIG_PGOFF(la)-----------/
+ * \-BIG_PGNUM(la)-/
+ * 
+ * The BIG_PDX, BIG_PGOFF, and BIG_PGNUM macors decompose linear address as
+ * shown. To construct a linear address la from BIG_PDX(la), BIG_PGOFF(la),
+ * use BIG_PGADDR(BIG_PDX(la), BIG_PGOFF(la)).
+ */
+
 // page number field of address
 #define PGNUM(la)	(((uintptr_t) (la)) >> PTXSHIFT)
 
+// big page number field of address
+#define BIG_PGNUM(la) (((uintptr_t) (la)) >> BIG_PGSHIFT)
+
 // page directory index
 #define PDX(la)		((((uintptr_t) (la)) >> PDXSHIFT) & 0x3FF)
 
+// big page directory index
+#define BIG_PDX(la) ((((uintptr_t) (la)) >> BIG_PGSHIFT) & 0x3FF)
+
 // page table index
 #define PTX(la)		((((uintptr_t) (la)) >> PTXSHIFT) & 0x3FF)
 
 // offset in page
 #define PGOFF(la)	(((uintptr_t) (la)) & 0xFFF)
 
+// offset in big page
+#define BIG_PGOFF(la) (((uintptr_t) (la)) &0x3FFFFF)
+
 // construct linear address from indexes and offset
 #define PGADDR(d, t, o)	((void*) ((d) << PDXSHIFT | (t) << PTXSHIFT | (o)))
 
+// construct linear address for big page indexes and offset
+#define BIG_PGADDR(d, o) ((void *) ((d) << BIG_PGSHIFT | (o)))
+
 // Page directory and page table constants.
 #define NPDENTRIES	1024		// page directory entries per page directory
-#define NPTENTRIES	1024		// page table entries per page table
+#define NPTENTRIES	1024		// page table entries per page table, in big page
+                                // there is only page directory, no page table.
 
+#define BIG_PGSHIFT 22                  // log(BIG_PGSIZE)
+#define BIG_PGSIZE  (1 << BIG_PGSHIFT)  // bytes mapped by a page
 #define PGSIZE		4096		// bytes mapped by a page
 #define PGSHIFT		12		// log2(PGSIZE)
 
@@ -75,6 +105,9 @@
 // Address in page table or page directory entry
 #define PTE_ADDR(pte)	((physaddr_t) (pte) & ~0xFFF)
 
+// Address in page directory entry
+#define PDE_ADDR(pde)	((physaddr_t) (pde) & ~0x3FFFFF)
+
 // Control Register flags
 #define CR0_PE		0x00000001	// Protection Enable
 #define CR0_MP		0x00000002	// Monitor coProcessor
diff --git a/kern/pmap.c b/kern/pmap.c
index f22a5e5..498330f 100644
--- a/kern/pmap.c
+++ b/kern/pmap.c
@@ -50,8 +50,11 @@ i386_detect_memory(void)
 	else
 		totalmem = basemem;
 
-	npages = totalmem / (PGSIZE / 1024);
-	npages_basemem = basemem / (PGSIZE / 1024);
+	// in big page system, a page is 4MB, which can cover memory much
+	// more than basemem. So npages_basemem should be zero and can't
+	// be relied on.
+	npages = totalmem / (BIG_PGSIZE / 1024);
+	npages_basemem = basemem / (BIG_PGSIZE / 1024);
 
 	cprintf("Physical memory: %uK available, base = %uK, extended = %uK\n",
 		totalmem, basemem, totalmem - basemem);
@@ -105,7 +108,7 @@ boot_alloc(uint32_t n)
 	// LAB 2: Your code here.
 	result = nextfree;
 	nextfree = ROUNDUP(nextfree + n, PGSIZE);
-	if ((uint32_t)nextfree - KERNBASE > npages*PGSIZE) {
+	if ((uint32_t)nextfree - KERNBASE > npages*BIG_PGSIZE) {
 		panic("Out of Memory!\n");
 		nextfree = result;
 		result = NULL;
@@ -131,12 +134,15 @@ mem_init(void)
 
 	// Find out how much memory the machine has (npages & npages_basemem).
 	i386_detect_memory();
-
+	
 	// Remove this line when you're ready to test this function.
 	//panic("mem_init: This function is not finished\n");
 
 	//////////////////////////////////////////////////////////////////////
 	// create initial page directory.
+	// in boot_alloc, we still use small page alignment, since now free page
+	// list has not been established and big page has not been enabled yet.
+	// By doing so we can save memory and possibly gain better performance.
 	kern_pgdir = (pde_t *) boot_alloc(PGSIZE);
 	memset(kern_pgdir, 0, PGSIZE);
 
@@ -147,7 +153,7 @@ mem_init(void)
 	// following line.)
 
 	// Permissions: kernel R, user R
-	kern_pgdir[PDX(UVPT)] = PADDR(kern_pgdir) | PTE_U | PTE_P;
+	kern_pgdir[BIG_PDX(UVPT)] = PADDR(kern_pgdir) | PTE_U | PTE_P;
 
 	//////////////////////////////////////////////////////////////////////
 	// Allocate an array of npages 'struct PageInfo's and store it in 'pages'.
@@ -169,8 +175,16 @@ mem_init(void)
 	page_init();
 
 	check_page_free_list(1);
-	check_page_alloc();
-	check_page();
+	// comment this test. because now we still use the manully established
+	// map and check_page_alloc() will alloc and free pages which are 4MB
+	// per page. This will surely casue a page fault and JOS to reboot. in
+	// addition, functions tested by check_page_alloc() are not changed and
+	// have been tested in lab2 branch. so it's OK to comment this test.
+	// check_page_alloc();
+
+	// comment this test. most of this test is related to page table. since
+	// we don't have a page table in big page system, we don't need this test.
+	// check_page();
 
 	//////////////////////////////////////////////////////////////////////
 	// Now we set up virtual memory
@@ -213,8 +227,12 @@ mem_init(void)
 	boot_map_region(kern_pgdir, KERNBASE, 0xFFFFFFFF-KERNBASE, (physaddr_t)0x0, PTE_W);
 
 	// Check that the initial page directory has been set up correctly.
-	check_kern_pgdir();
-
+	// since one big page may cover many important address, it's not suitable
+	// to check our newly established map in check_kern_pgdir() way.
+	// check_kern_pgdir();
+ 
+	// enable big page
+	lcr4(rcr4() | CR4_PSE);
 	// Switch from the minimal entry page directory to the full kern_pgdir
 	// page table we just created.	Our instruction pointer should be
 	// somewhere between KERNBASE and KERNBASE+4MB right now, which is
@@ -234,7 +252,9 @@ mem_init(void)
 	lcr0(cr0);
 
 	// Some more checks, only possible after kern_pgdir is installed.
-	check_page_installed_pgdir();
+	// Some of tests in the following function use PGSIZE(4KB) concept,
+	// which is not suitable for big page system.
+	//check_page_installed_pgdir();
 }
 
 // --------------------------------------------------------------
@@ -349,30 +369,21 @@ page_init(void)
 	 * 
 	 */
 
+	// In big page system, it's much easier to initialize PageInfo array
+	// since one page will cover all base memory. Big pages where base
+	// memeory and kernel reside are used and reside contiguously.
 	page_free_list = NULL;
 
-	pages[0].pp_ref = 1;		// mark physical page 0 as in use
-	size_t i;
-	
-	for (i = 1; i < npages_basemem; ++i) {
-		// The rest of base memory, [PGSIZE, npages_basemem * PGSIZE)
-		// is free.
-		pages[i].pp_ref = 0;
-		pages[i].pp_link = page_free_list;
-		page_free_list = &pages[i];
-	}
+	size_t kernel_end_page = ROUNDUP(((uintptr_t)(boot_alloc(0)) - KERNBASE), BIG_PGSIZE)/BIG_PGSIZE;
+
+	size_t i = 0;
 
-	size_t kernel_end_page = ((uint32_t)(boot_alloc(0)) - KERNBASE)/PGSIZE;
 	for (; i < kernel_end_page; ++i) {
-		// IO hole and kernel physical memory reside consecutively
-		// in physcial memory. So initialize their corresponding 
-		// page entries in one loop
 		pages[i].pp_ref = 1;
 		pages[i].pp_link = NULL;
 	}
-	
+
 	for (; i < npages; ++i) {
-		// The rest of extended memory are not used
 		pages[i].pp_ref = 0;
 		pages[i].pp_link = page_free_list;
 		page_free_list = &pages[i];
@@ -467,35 +478,33 @@ page_decref(struct PageInfo* pp)
 // Hint 4: all pointers in C are virtual addresses but all addresses in
 // page dir and page table are physical address
 //
-pte_t *
+
+/*
+ * In big page system,
+ * given 'pgdir', a pointer to a page directory, pgdir_walk returns
+ * a pointer to the page directory entry (PDE) for linear address 'va'.
+ * 
+ * The relevantt page directory entry might not exist yet.
+ * If this is true, and create == false, then pgdir_walk returns NULL.
+ * Otherwise, pgdir_walk return the corresponding page directory entry.
+ * 
+ */
+
+pde_t *
 pgdir_walk(pde_t *pgdir, const void *va, int create)
 {
 	// Fill this function in
-	pde_t *pd_entry = &pgdir[PDX(va)];
-	pte_t *pgtable = NULL;
+	pde_t *pd_entry = &pgdir[BIG_PDX(va)];
 
-	// if the relevant page table page exists
-	if ((physaddr_t)*pd_entry & PTE_P) {
-		pgtable = (pte_t *)(KADDR(PTE_ADDR(*pd_entry)));
-		return &pgtable[PTX(va)];
-	}
-	
-	// if the relevant page table doesn't exist and create == false
+	if ((physaddr_t)*pd_entry & PTE_P)
+		return pd_entry;
+
+	// if the relevant page directory entry doesn't exist
+	// and create == false
 	if (create == false)
 		return NULL;
 	
-	// allocate a new page table, use ALLOC_ZERO to clear the page
-	// page_alloc guarantees NULL is returned on allocation failure
-	struct PageInfo *new_page_info = page_alloc(ALLOC_ZERO);
-	if (new_page_info == NULL)
-		return NULL;
-	++(new_page_info->pp_ref);
-	
-	// pages of kernel and users may be allocated on one page table
-	// so it's convenient to give page table a user level privilige
-	*pd_entry = (pde_t)((uintptr_t)page2pa(new_page_info) | PTE_P | PTE_W | PTE_U);
-	pgtable = (pte_t *)page2kva(new_page_info);
-	return &pgtable[PTX(va)];
+	return pd_entry;
 }
 
 //
@@ -512,15 +521,19 @@ pgdir_walk(pde_t *pgdir, const void *va, int create)
 static void
 boot_map_region(pde_t *pgdir, uintptr_t va, size_t size, physaddr_t pa, int perm)
 {
-	for (size_t alloc_size = 0; alloc_size < size; alloc_size += PGSIZE) {
-		pte_t *pt_entry = pgdir_walk(pgdir, (void *)va, true);
+	uintptr_t round_va = ROUNDDOWN(va, BIG_PGSIZE);
+	uintptr_t round_pa = ROUNDDOWN(pa, BIG_PGSIZE);
+	size_t round_size = ROUNDUP(size + va - round_va, BIG_PGSIZE);
+	for (size_t alloc_size = 0; alloc_size < round_size; alloc_size += BIG_PGSIZE) {
+		pde_t *pd_entry = pgdir_walk(pgdir, (void *)round_va, true);
+
+		if (pd_entry == NULL)
+			panic("Page alloction failed");
 		
-		if (pt_entry == NULL)
-			panic("Page allocation failed!");
-		
-		*pt_entry = pa | perm | PTE_P;
-		va += PGSIZE;
-		pa += PGSIZE;
+		*pd_entry = round_pa | perm | PTE_PS | PTE_P;
+
+		round_va += BIG_PGSIZE;
+		round_pa += BIG_PGSIZE;
 	}
 }
 
@@ -556,20 +569,22 @@ boot_map_region(pde_t *pgdir, uintptr_t va, size_t size, physaddr_t pa, int perm
 // Hint: The TA solution is implemented using pgdir_walk, page_remove,
 // and page2pa.
 //
+
 int
 page_insert(pde_t *pgdir, struct PageInfo *pp, void *va, int perm)
 {
 	// Fill this function in
-	pte_t *pt_entry = pgdir_walk(pgdir, va, true);
-	
-	if (pt_entry == NULL)
+	pde_t *pd_entry = pgdir_walk(pgdir, va, true);
+
+	if (pd_entry == NULL)
 		return -E_NO_MEM;
 
 	++(pp->pp_ref);
-	if (*pt_entry & PTE_P)
-		page_remove(pgdir, va);		// page_remove() will call tlb_invalidate()
+	if (*pd_entry & PTE_P)
+		page_remove(pgdir, va);
 
-	*pt_entry = page2pa(pp) | perm | PTE_P;
+	*pd_entry = page2pa(pp) | perm | PTE_P;
+	
 	return 0;
 }
 
@@ -584,20 +599,19 @@ page_insert(pde_t *pgdir, struct PageInfo *pp, void *va, int perm)
 //
 // Hint: the TA solution uses pgdir_walk and pa2page.
 //
+
 struct PageInfo *
-page_lookup(pde_t *pgdir, void *va, pte_t **pte_store)
+page_lookup(pde_t *pgdir, void *va, pde_t **pde_store)
 {
-	// Fill this function in
-
-	pte_t *pt_entry = pgdir_walk(pgdir, va, false);
+	pde_t *pd_entry = pgdir_walk(pgdir, va, false);
 
-	if (pt_entry == NULL)
+	if (pd_entry == NULL)
 		return NULL;
 	
-	if (pte_store)	// if pte_store is not zero
-		*pte_store = pt_entry;
-
-	return pa2page(PTE_ADDR(*pt_entry));
+	if (pde_store)
+		*pde_store = pd_entry;
+	
+	return pa2page(PDE_ADDR(*pd_entry));
 }
 
 //
@@ -615,17 +629,17 @@ page_lookup(pde_t *pgdir, void *va, pte_t **pte_store)
 // Hint: The TA solution is implemented using page_lookup,
 // 	tlb_invalidate, and page_decref.
 //
+
 void
 page_remove(pde_t *pgdir, void *va)
 {
-	// Fill this function in
-	pte_t *pt_entry;
-	struct PageInfo *mapped_page = page_lookup(pgdir, va, &pt_entry);
-	if (mapped_page == NULL)	// no physical page mapped, do nothing
+	pde_t *pd_entry;
+	struct PageInfo *mapped_page = page_lookup(pgdir, va, &pd_entry);
+	if (mapped_page == NULL)
 		return;
 	
 	page_decref(mapped_page);
-	*pt_entry = 0;
+	*pd_entry = 0;
 	tlb_invalidate(pgdir, va);
 }
 
@@ -701,7 +715,10 @@ check_page_free_list(bool only_low_memory)
 			++nfree_extmem;
 	}
 
-	assert(nfree_basemem > 0);
+	// comment this assert as the first 4MB page already cover base memory.
+	// furthermore, some of base memory is used. So no page in base memory
+	// is free.
+	//assert(nfree_basemem > 0);
 	assert(nfree_extmem > 0);
 
 	cprintf("check_page_free_list() succeeded!\n");
@@ -736,10 +753,10 @@ check_page_alloc(void)
 	assert(pp0);
 	assert(pp1 && pp1 != pp0);
 	assert(pp2 && pp2 != pp1 && pp2 != pp0);
-	assert(page2pa(pp0) < npages*PGSIZE);
-	assert(page2pa(pp1) < npages*PGSIZE);
-	assert(page2pa(pp2) < npages*PGSIZE);
-
+	assert(page2pa(pp0) < npages*BIG_PGSIZE);
+	assert(page2pa(pp1) < npages*BIG_PGSIZE);
+	assert(page2pa(pp2) < npages*BIG_PGSIZE);
+	
 	// temporarily steal the rest of the free pages
 	fl = page_free_list;
 	page_free_list = 0;
@@ -761,7 +778,7 @@ check_page_alloc(void)
 	assert(!page_alloc(0));
 
 	// test flags
-	memset(page2kva(pp0), 1, PGSIZE);
+	memset(page2kva(pp0), 1, BIG_PGSIZE);
 	page_free(pp0);
 	assert((pp = page_alloc(ALLOC_ZERO)));
 	assert(pp && pp0 == pp);
@@ -771,7 +788,7 @@ check_page_alloc(void)
 
 	// give free list back
 	page_free_list = fl;
-
+	
 	// free the pages we took
 	page_free(pp0);
 	page_free(pp1);
@@ -888,7 +905,9 @@ check_page(void)
 	assert(page_lookup(kern_pgdir, (void *) 0x0, &ptep) == NULL);
 
 	// there is no free memory, so we can't allocate a page table
-	assert(page_insert(kern_pgdir, pp1, 0x0, PTE_W) < 0);
+	// in big page system, there is no page table. so this test is
+	// unnecessary.
+	// assert(page_insert(kern_pgdir, pp1, 0x0, PTE_W) < 0);
 
 	// free pp0 and try again: pp0 should be used for page table
 	page_free(pp0);
diff --git a/kern/pmap.h b/kern/pmap.h
index 9d447fc..754c443 100644
--- a/kern/pmap.h
+++ b/kern/pmap.h
@@ -9,7 +9,8 @@
 #include <inc/memlayout.h>
 #include <inc/assert.h>
 
-#define npages_in_4GB	(1<<(32-PGSHIFT))
+#define npages_in_4GB	(1 << (32 - BIG_PGSHIFT))
+//#define npages_in_4GB	(1<<(32-PGSHIFT))
 #define DWORD_SIZE		4		// four bytes per dword
 #define DOWRD_SHIFT		2		// log2(DWORD_SIZE)
 #define ndwords_in_4GB	(1<<(32-DOWRD_SHIFT))
@@ -45,7 +46,7 @@ _paddr(const char *file, int line, void *kva)
 static inline void*
 _kaddr(const char *file, int line, physaddr_t pa)
 {
-	if (PGNUM(pa) >= npages)
+	if (BIG_PGNUM(pa) >= npages)
 		_panic(file, line, "KADDR called with invalid pa %08lx", pa);
 	return (void *)(pa + KERNBASE);
 }
@@ -71,15 +72,15 @@ void	tlb_invalidate(pde_t *pgdir, void *va);
 static inline physaddr_t
 page2pa(struct PageInfo *pp)
 {
-	return (pp - pages) << PGSHIFT;
+	return (pp - pages) << BIG_PGSHIFT;
 }
 
 static inline struct PageInfo*
 pa2page(physaddr_t pa)
 {
-	if (PGNUM(pa) >= npages)
+	if (BIG_PGNUM(pa) >= npages)
 		panic("pa2page called with invalid pa");
-	return &pages[PGNUM(pa)];
+	return &pages[BIG_PGNUM(pa)];
 }
 
 static inline void*
```

### Challenge 2

> Extend the JOS kernel monitor with commands to:
> * Display in a useful and easy-to-read format all of the physical page mappings (or lack thereof) that apply to a particular range of virtual/linear addresses in the currently active address space. For example, you might enter 'showmappings 0x3000 0x5000' to display the physical page mappings and corresponding permission bits that apply to the pages at virtual addresses 0x3000, 0x4000, and 0x5000.
> * Explicitly set, clear, or change the permissions of any mapping in the current address space.
> * Dump the contents of a range of memory given either a virtual or physical address range. Be sure the dump code behaves correctly when the range extends across page boundaries!

As long as you understand what we have talked about in this report, it's easy to implemente functions required.

The following is a exhibition of fuctions I implemented in this challenge.

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
K> help list
help
-SYNOPSIS:
    help {list | command name}
-DESCRIPTION:
    list: display all help information of all commands.
    command name: display help information of given name

kerninfo
Display information about the kernel

backtrace
Display the current call stack

mappings
-SYNOPSIS:
    mappings {show laddr uaddr} | {clear vaddr [size==1]}
             | {set perm vaddr [size==1]}
-DESCRIPTION:
    show: display page mappings in [laddr, uaddr)
    clear: clear privilege of size(in page) pages from vaddr.
           The privilege is set to PTE_U == 0 and PTE_W == 1.
    set: set privilege of size(in page) pages from vaddr.
         The privilege 'perm' can be specified by a number or a
         two-character string containing 'u'(PTE_U), 'w'(PTE_W) or
         '-'(none)
    All addresses will be rounded down to page alignment.

dump
-SYNOPSIS:
    dump addr_type addr [size==1]
-DESCRIPTION:
    addr_type: '-v' for virtual address, '-p for physical address
    addr: beginning address
    size: memory size in dwords(4 bytes)

K> mappings show 0xf0100000 0xf0104000
         vaddr                       paddr            privilege
0xf0100000 - 0xf0101000:    0x00100000 - 0x00101000      -w
0xf0101000 - 0xf0102000:    0x00101000 - 0x00102000      -w
0xf0102000 - 0xf0103000:    0x00102000 - 0x00103000      -w
0xf0103000 - 0xf0104000:    0x00103000 - 0x00104000      -w
K> mappings set uw 0xf0100000 4
K> mappings show 0xf0100000 0xf0104000
         vaddr                       paddr            privilege
0xf0100000 - 0xf0101000:    0x00100000 - 0x00101000      uw
0xf0101000 - 0xf0102000:    0x00101000 - 0x00102000      uw
0xf0102000 - 0xf0103000:    0x00102000 - 0x00103000      uw
0xf0103000 - 0xf0104000:    0x00103000 - 0x00104000      uw
K> mappings clear 0xf0100000 4
K> mappings show 0xf0100000 0xf0104000
         vaddr                       paddr            privilege
0xf0100000 - 0xf0101000:    0x00100000 - 0x00101000      -w
0xf0101000 - 0xf0102000:    0x00101000 - 0x00102000      -w
0xf0102000 - 0xf0103000:    0x00102000 - 0x00103000      -w
0xf0103000 - 0xf0104000:    0x00103000 - 0x00104000      -w
K> dump -v 0xf0100000 10
0xf0100000:  0x1badb002  0x00000000  0xe4524ffe  0x7205c766  
0xf0100010:  0x34000004  0x7000b812  0x220f0011  0xc0200fd8  
0xf0100020:  0x0100010d  0xc0220f80  
K> dump -p 0x00100000 10
0x100000:  0x1badb002  0x00000000  0xe4524ffe  0x7205c766  
0x100010:  0x34000004  0x7000b812  0x220f0011  0xc0200fd8  
0x100020:  0x0100010d  0xc0220f80  
K> QEMU 2.5.0 monitor - type 'help' for more information
(qemu) x/10x 0xf0100000
f0100000: 0x1badb002 0x00000000 0xe4524ffe 0x7205c766
f0100010: 0x34000004 0x7000b812 0x220f0011 0xc0200fd8
f0100020: 0x0100010d 0xc0220f80
(qemu) xp/10x 0x00100000
0000000000100000: 0x1badb002 0x00000000 0xe4524ffe 0x7205c766
0000000000100010: 0x34000004 0x7000b812 0x220f0011 0xc0200fd8
0000000000100020: 0x0100010d 0xc0220f80
(qemu) QEMU: Terminated
```

The code for implementing these functions is shown in the following `git diff` log.

```git
diff --git a/kern/monitor.c b/kern/monitor.c
index f77abb7..67ddd15 100644
--- a/kern/monitor.c
+++ b/kern/monitor.c
@@ -7,10 +7,12 @@
 #include <inc/assert.h>
 #include <inc/x86.h>
 #include <inc/color.h>
+#include <inc/types.h>
 
 #include <kern/console.h>
 #include <kern/monitor.h>
 #include <kern/kdebug.h>
+#include <kern/pmap.h>
 
 #define CMDBUF_SIZE	80	// enough for one VGA text line
 
@@ -23,20 +25,90 @@ struct Command {
 };
 
 static struct Command commands[] = {
-	{ "help", "Display this list of commands", mon_help },
-	{ "kerninfo", "Display information about the kernel", mon_kerninfo },
-	{ "backtrace", "Display the current call stack", mon_backtrace},
+	{ "help", CMD_HELP_HELP_STR, mon_help },
+	{ "kerninfo", CMD_KERNINFO_HELP_STR, mon_kerninfo },
+	{ "backtrace", CMD_BACKTRACE_HELP_STR, mon_backtrace},
+	{ "mappings", CMD_MAPPINGS_HELP_STR, mon_mappings},
+	{ "dump", CMD_DUMP_HELP_STR, mon_dump},
 };
 
+/***** helper functions *****/
+#define ADDR_TYPE_V		0		// virtual address type
+#define ADDR_TYPE_P		1		// physical address type
+
+#define CMD_ERR_ARG		0		// wrong arguments
+#define CMD_ERR_NUM		1		// invalid number format
+#define CMD_ERR_OPE		2		// invalid operation
+#define CMD_ERR_STR		3		// not specific error
+
+inline int
+cmd_error(int err_type, char *str)
+{
+	switch(err_type) {
+		case CMD_ERR_ARG: 
+			cprintf("E: Wrong arguments! type 'help %s' for usage.\n", str);
+			break;
+		case CMD_ERR_NUM:
+			cprintf("E: Wrong Number format!\n");
+			break;
+		case CMD_ERR_OPE:
+			cprintf("E: Invalid operation %s\n", str);
+			break;
+		case CMD_ERR_STR:
+			cprintf("E: %s\n", str);
+			break;
+		default:
+			cprintf("E: Wrong error type!\n");
+			break;
+	}
+	
+	return 0;
+}
+
+// using macro to mimic a template function to adapt
+// different types of _num_ptr
+#define parse_number(_num_str, _num_ptr)			\
+({													\
+	typeof(_num_str) __num_str = (_num_str);		\
+	typeof(_num_ptr) __num_ptr = (_num_ptr);	\
+	char *end_char;									\
+	*__num_ptr = strtol(__num_str, &end_char, 0);	\
+	*end_char != '\0';								\
+})
+
+
 /***** Implementations of basic kernel monitor commands *****/
 
 int
 mon_help(int argc, char **argv, struct Trapframe *tf)
 {
-	int i;
+	char *operation;
+	if ((operation = argv[1]) == 0) {
+		cprintf(CMD_HELP_HELP_STR);
+		return 0;
+	}
+
+	if (argc > 2)
+		return cmd_error(CMD_ERR_ARG, "help");
+
+	if (strcmp(operation, "list") == 0) {
+		for (int i = 0; i < ARRAY_SIZE(commands); i++)
+			cprintf("%s\n%s\n", commands[i].name, commands[i].desc);
+		return 0;
+	}
+
+
+	bool cmd_found = false;
+	for (int i = 0; i < ARRAY_SIZE(commands); i++) {
+		if (strcmp(operation, commands[i].name) == 0) {
+			cprintf("%s\n%s\n", commands[i].name, commands[i].desc);
+			cmd_found = true;
+		}
+	}
+	
+	if (!cmd_found)
+		cprintf("command not found!\n");
 
-	for (i = 0; i < ARRAY_SIZE(commands); i++)
-		cprintf("%s - %s\n", commands[i].name, commands[i].desc);
 	return 0;
 }
 
@@ -103,7 +175,283 @@ mon_backtrace(int argc, char **argv, struct Trapframe *tf)
 	return 0;
 }
 
+int
+show_mappings(uintptr_t lower_addr, uintptr_t upper_addr)
+{
+	uintptr_t laddr = ROUNDDOWN(lower_addr, PGSIZE);
+	uintptr_t uaddr = ROUNDDOWN(upper_addr, PGSIZE);
+
+	if (laddr != lower_addr)
+		cprintf("lower address:%p -> %p\n", lower_addr, laddr);
+	if (uaddr != upper_addr)
+		cprintf("upper address:%p -> %p\n", upper_addr, uaddr);
+
+	cprintf("         vaddr                       paddr            privilege\n");
+	while (laddr < uaddr) {
+		cprintf("%08p - %08p:    ", laddr, laddr + PGSIZE);
+		
+		pte_t *pt_entry = pgdir_walk(kern_pgdir, (void *)laddr, false);
+		if (pt_entry == NULL || !(*pt_entry & PTE_P)) {
+			cprintf("not mapped\n");
+		} else {
+			physaddr_t pte_paddr = PTE_ADDR(*pt_entry);
+			char privilege[] = {
+				(*pt_entry & PTE_U) ? 'u' : '-',
+				(*pt_entry & PTE_W) ? 'w' : '-',
+				'\0'
+			};
+
+			cprintf("%08p - %08p      %s\n", pte_paddr, pte_paddr + PGSIZE, privilege);
+		}
+
+		laddr += PGSIZE;
+	}
+
+	return 0;	
+}
+
+int
+set_mappings(uintptr_t virtual_addr, size_t size, int perm)
+{
+	if (PGNUM(virtual_addr) + size > npages_in_4GB)
+		return cmd_error(CMD_ERR_STR, "Addresses exceed 4G memory!");
+	
+	uintptr_t vaddr = ROUNDDOWN(virtual_addr, PGSIZE);
+	if (vaddr != virtual_addr)
+		cprintf("virtual address:%p -> %p\n", virtual_addr, vaddr);
+	
+	while (size-- > 0) {
+		pte_t *pte_ptr = pgdir_walk(kern_pgdir, (void *)vaddr, false);
+		pte_t pt_entry = *pte_ptr;
+		if (pt_entry & PTE_P) {
+			// turn off PTE_U and PTE_W
+			pt_entry &= ~(PTE_U | PTE_W);
+			
+			// mask off other bits in perm, and turn on bits
+			// in *pt_entry based on perm
+			pt_entry |= (perm & (PTE_U | PTE_W));
+			*pte_ptr = pt_entry;
+		} else
+			cprintf("W: %d not mapped!\n", vaddr);
+
+		vaddr += PGSIZE;
+	}
+
+	return 0;
+}
+
+int
+clear_mappings(uintptr_t virtual_addr, size_t size)
+{
+	// turn off PTE_U
+	return set_mappings(virtual_addr, size, PTE_W);
+}
+
+int
+mon_mappings(int argc, char **argv, struct Trapframe *tf)
+{
+	/*
+	 * SYNOPSIS:
+	 * 		mappings {show laddr uaddr} |
+	 * 				 {clear vaddr [size]} |
+	 * 				 {set perm vaddr [size]}
+	 * 		laddr: lower address
+	 * 		uaddr: upper address
+	 * 		vaddr: virtual address
+	 * 		paddr: physical address
+	 * 		size: memory size in pages
+	 * 		perm: page table entry permission
+	 */
+
+	char *operation;
+	if ((operation = argv[1]) == 0) {
+		cprintf(CMD_MAPPINGS_HELP_STR);
+		return 0;
+	}
+
+	if (strcmp(operation, "show") == 0) {
+		// mappings {show laddr uaddr}
+
+		if (argc != 4)
+			return cmd_error(CMD_ERR_ARG, "mappings");
+		
+		uintptr_t laddr, uaddr;
+		if (parse_number(argv[2], &laddr) ||
+			parse_number(argv[3], &uaddr))
+			return cmd_error(CMD_ERR_NUM,"");
+		
+		return show_mappings(laddr, uaddr);
+	}
+	
+	if (strcmp(operation, "clear") == 0) {
+		// mappings {clear vaddr [size]}
+		
+		if (argc < 3 || argc > 4)
+			return cmd_error(CMD_ERR_ARG, "mappings");
+		
+		uintptr_t vaddr;
+		if (parse_number(argv[2], &vaddr))
+			return cmd_error(CMD_ERR_NUM, NULL);
+		
+		size_t size;
+		if (argv[3]) {		// if size is provided explicitly
+			if (parse_number(argv[3], &size))
+				return cmd_error(CMD_ERR_NUM, NULL);
+		} else
+			size = 1;
+		
+		return clear_mappings(vaddr, size);
+	}
+	
+	if (strcmp(operation, "set") == 0) {
+		// mappings {set perm vaddr [size]}
+		if (argc < 4 || argc > 5)
+			return cmd_error(CMD_ERR_ARG, "mappings");
+		
+		int perm;
+		char *perm_str = argv[2];
+		if (parse_number(perm_str, &perm)) {
+			if (strlen(perm_str) != 2)
+				return cmd_error(CMD_ERR_STR, "Wrong privilege format!");
+			
+			perm = 0;
+			for (int i = 0; i < 2; ++i) {
+				if (perm_str[i] == 'u')
+					perm |= PTE_U;
+				else if (perm_str[i] == 'w')
+					perm |= PTE_W;
+				else if (perm_str[i] == '-')
+					continue;
+				else
+					return cmd_error(CMD_ERR_STR, "Wrong privilege format!");
+			}
+		}
+
+		uintptr_t vaddr;
+		if (parse_number(argv[3], &vaddr))
+			return cmd_error(CMD_ERR_NUM, NULL);
+		
+		size_t size;
+		if (argv[4]) {		// if size is provided explicitly
+			if (parse_number(argv[4], &size))
+				return cmd_error(CMD_ERR_NUM, NULL);
+		} else
+			size = 1;
+
+		return set_mappings(vaddr, size, perm);
+	}
+
+	return cmd_error(CMD_ERR_OPE, operation);
+}
+
+int
+dump_vmem(uintptr_t addr, size_t size)
+{
+	// dump memory using virtual address
+	// DWORD alignment
+	addr = ROUNDDOWN(addr, 4);
+	if (DOWRD_NUM(addr) + size > ndwords_in_4GB)
+		return cmd_error(CMD_ERR_STR, "Addresses exceed 4G memory!");
+
+	
+	while(size > 0) {
+		cprintf("%p:  ", addr);
+		
+		// display four dwords each line
+		for (int i = 0; i < 4 && size > 0; ++i) {
+			// we don't want to cause a page fault if addr points to a
+			// page that has not been mapped. So we check whether the
+			// page pointed has been mapped.
+			// In fact, as long as consecutive addrs reside in the same
+			// page, we don't need to call page_lookup() repeatedly. We
+			// will improve it later.
+			if (page_lookup(kern_pgdir, (void *)addr, NULL))
+				cprintf("0x%08x  ", *((uint32_t *)addr));
+			else
+				cprintf("not mapped  ");
+			
+			--size;
+			addr += DWORD_SIZE;
+		}
+
+		cprintf("\n");
+	}
+	return 0;
+}
+
+int
+dump_pmem(physaddr_t addr, size_t size) {
+	
+	while(size > 0) {
+		cprintf("%p:  ", addr);
+		
+		// display four dwords each line
+		for (int i = 0; i < 4 && size > 0; ++i) {
+			if (PGNUM(addr) >= npages) {
+				// pa2page() will panic JOS if addr is a invalid physical
+				// address. We don't want a normal shell command that
+				// doesn't have any side effect to panic our JOS. So we
+				// check the legitimacy of addr in advance. The same check
+				// is executed in pg2page() as well as page2kva() but we
+				// don't want those condition check branch into a panic().
+				cprintf("\nExceed physical memory!");
+				size = 0; // break outer while loop, too.
+				break;
+			}
+
+			// A physical address will always be transformed into a linear
+			// address above KERNBASE, where we has mapped that linear
+			// address space in mem_init(). So we needn't worry *num_ptr
+			// will cause a page fault.
+			uint32_t *num_ptr = page2kva(pa2page(addr)) + PGOFF(addr);
+			cprintf("0x%08x  ", *num_ptr);
+
+			--size;
+			addr += DWORD_SIZE;
+		}
+
+		cprintf("\n");
+	}
+	return 0;
+}
+
+int
+mon_dump(int argc, char **argv, struct Trapframe *tf) {
+	/*
+	 * dump addr_type addr [size==1]
+	 *     addr_type: address type, -p | -v
+	 *     addr: beginning address
+	 *     size: memory size in DWORD(32 bits)
+	 */
 
+	char *addr_type_str;
+	if ((addr_type_str = argv[1]) == 0) {
+		cprintf(CMD_DUMP_HELP_STR);
+		return 0;
+	}
+
+	if (argc < 3 || argc > 4)
+		return cmd_error(CMD_ERR_ARG, "dump");
+
+
+	uint32_t addr ;
+	if (parse_number(argv[2], &addr))
+		return cmd_error(CMD_ERR_NUM, NULL);
+	
+	size_t size;
+	if (argc == 4) {
+		if (parse_number(argv[3], &size))
+			return cmd_error(CMD_ERR_NUM, NULL);
+	} else
+		size = 1;
+	
+	if (strcmp(addr_type_str, "-v") == 0)
+		return dump_vmem((uintptr_t)addr, size);
+	else if (strcmp(addr_type_str, "-p") == 0)
+		return dump_pmem((physaddr_t)addr, size);
+	else
+		return cmd_error(CMD_ERR_STR, "Wrong address type!");
+}
 
 /***** Kernel monitor command interpreter *****/
 
diff --git a/kern/monitor.h b/kern/monitor.h
index 0aa0f26..33d53a2 100644
--- a/kern/monitor.h
+++ b/kern/monitor.h
@@ -15,5 +15,40 @@ void monitor(struct Trapframe *tf);
 int mon_help(int argc, char **argv, struct Trapframe *tf);
 int mon_kerninfo(int argc, char **argv, struct Trapframe *tf);
 int mon_backtrace(int argc, char **argv, struct Trapframe *tf);
+int mon_mappings(int argc, char **argv, struct Trapframe *tf);
+int mon_dump(int argc, char **argv, struct Trapframe *tf);
+
+#define CMD_HELP_HELP_STR       "\
+-SYNOPSIS:\n\
+    help {list | command name}\n\
+-DESCRIPTION:\n\
+    list: display all help information of all commands.\n\
+    command name: display help information of given name\n"
+
+#define CMD_KERNINFO_HELP_STR   "Display information about the kernel\n"
+
+#define CMD_BACKTRACE_HELP_STR  "Display the current call stack\n"
+
+#define CMD_MAPPINGS_HELP_STR   "\
+-SYNOPSIS:\n\
+    mappings {show laddr uaddr} | {clear vaddr [size==1]}\n\
+             | {set perm vaddr [size==1]}\n\
+-DESCRIPTION:\n\
+    show: display page mappings in [laddr, uaddr)\n\
+    clear: clear privilege of size(in page) pages from vaddr.\n\
+           The privilege is set to PTE_U == 0 and PTE_W == 1.\n\
+    set: set privilege of size(in page) pages from vaddr.\n\
+         The privilege 'perm' can be specified by a number or a\n\
+         two-character string containing 'u'(PTE_U), 'w'(PTE_W) or\n\
+         '-'(none)\n\
+    All addresses will be rounded down to page alignment.\n"
+
+#define CMD_DUMP_HELP_STR       "\
+-SYNOPSIS:\n\
+    dump addr_type addr [size==1]\n\
+-DESCRIPTION:\n\
+    addr_type: '-v' for virtual address, '-p for physical address\n\
+    addr: beginning address\n\
+    size: memory size in dwords(4 bytes)\n"
 
 #endif	// !JOS_KERN_MONITOR_H
diff --git a/kern/pmap.h b/kern/pmap.h
index 950cca1..9d447fc 100644
--- a/kern/pmap.h
+++ b/kern/pmap.h
@@ -9,6 +9,12 @@
 #include <inc/memlayout.h>
 #include <inc/assert.h>
 
+#define npages_in_4GB	(1<<(32-PGSHIFT))
+#define DWORD_SIZE		4		// four bytes per dword
+#define DOWRD_SHIFT		2		// log2(DWORD_SIZE)
+#define ndwords_in_4GB	(1<<(32-DOWRD_SHIFT))
+#define DOWRD_NUM(la)	(((uintptr_t) (la)) >> DOWRD_SHIFT)
+
 extern char bootstacktop[], bootstack[];
 
 extern struct PageInfo *pages;
```

### Challenge 3

> Q : Write up an outline of how a kernel could be designed to allow user environments unrestricted use of the full 4GB virtual and linear address space. Hint: the technique is sometimes known as "follow the bouncing kernel." In your design, be sure to address exactly what has to happen when the processor transitions between kernel and user modes, and how the kernel would accomplish such transitions. Also describe how the kernel would access physical memory and I/O devices in this scheme, and how the kernel would access a user environment's virtual address space during system calls and the like. Finally, think about and describe the advantages and disadvantages of such a scheme in terms of flexibility, performance, kernel complexity, and other factors you can think of.

I don't want to write an essay on this topic. We are surely able to allow user environments unrestricted use of the full 4GB virtual and linear address space. One way to fulfill this requirement is to change user program's page tables to kernel's every time it wants to interact with kernel, whose performance is lower than JOS's choice that a part of memory in user enviroments is reserved for kernel so that we don't need to change page tables every time the user program want to interact with kernel. Another way is to re-map kernel to another place every time user programs want to use memory occupied by kernel. This technique is called "bouncing kernel".

### Challenge 4

> Q : Since our JOS kernel's memory management system only allocates and frees memory on page granularity, we do not have anything comparable to a general-purpose malloc/free facility that we can use within the kernel. This could be a problem if we want to support certain types of I/O devices that require physically contiguous buffers larger than 4KB in size, or if we want user-level environments, and not just the kernel, to be able to allocate and map 4MB superpages for maximum processor efficiency. (See the earlier challenge problem about PTE_PS.)
> Generalize the kernel's memory allocation system to support pages of a variety of power-of-two allocation unit sizes from 4KB up to some reasonable maximum of your choice. Be sure you have some way to divide larger allocation units into smaller ones on demand, and to coalesce multiple small allocation units back into larger units when possible. Think about the issues that might arise in such a system.

**[Buddy System](https://en.wikipedia.org/wiki/Buddy_system)** is exactly the answer. In this challenge I implemented a buddy system and initialized the buddy system after all tests in `mem_init()` so that none of those test would fail. In addition, I wrote my own tests for my buddy system.

The result is shown below.

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
check_buddy_tree() succeeded!
check_buddy_alloc&free() succeed!
Welcome to the JOS kernel monitor!
Type 'help' for a list of commands.
K>
```

The code for buddy system is shown below.

```git
diff --git a/inc/memlayout.h b/inc/memlayout.h
index 89bf0e3..7ee983d 100644
--- a/inc/memlayout.h
+++ b/inc/memlayout.h
@@ -174,8 +174,8 @@ extern volatile pde_t uvpd[];     // VA of current page directory
  * with page2pa() in kern/pmap.h.
  */
 struct PageInfo {
-	// Next page on the free list.
-	struct PageInfo *pp_link;
+	struct PageInfo *pp_prev;	// previous page on the free list.
+	struct PageInfo *pp_next;	// next page on the free list
 
 	// pp_ref is the count of pointers (usually in page table entries)
 	// to this page, for pages allocated using page_alloc.
@@ -183,6 +183,11 @@ struct PageInfo {
 	// boot_alloc do not have valid reference count fields.
 
 	uint16_t pp_ref;
+
+	// pp_count == 2^N if this page is N-th order free.
+	// For the declaration of N-th order free, refer to kern/pmap.h
+	// pp_count == 0 means it's not free.
+	uint16_t pp_count;
 };
 
 #endif /* !__ASSEMBLER__ */
diff --git a/kern/pmap.c b/kern/pmap.c
index f22a5e5..384c08b 100644
--- a/kern/pmap.c
+++ b/kern/pmap.c
@@ -19,6 +19,7 @@ pde_t *kern_pgdir;		// Kernel's initial page directory
 struct PageInfo *pages;		// Physical page state array
 static struct PageInfo *page_free_list;	// Free list of physical pages
 
+static struct PageInfo *nOrder_free_pages[MAX_BUDDY_ORDER+1];
 
 // --------------------------------------------------------------
 // Detect machine's physical memory setup.
@@ -69,6 +70,8 @@ static void check_kern_pgdir(void);
 static physaddr_t check_va2pa(pde_t *pgdir, uintptr_t va);
 static void check_page(void);
 static void check_page_installed_pgdir(void);
+static void check_buddy_tree(void);
+static void check_buddy_alloc(void);
 
 // This simple physical memory allocator is used only while JOS is setting
 // up its virtual memory system.  page_alloc() is the real allocator.
@@ -235,6 +238,9 @@ mem_init(void)
 
 	// Some more checks, only possible after kern_pgdir is installed.
 	check_page_installed_pgdir();
+	buddy_tree_init(page_free_list);
+	check_buddy_tree();
+	check_buddy_alloc();
 }
 
 // --------------------------------------------------------------
@@ -311,7 +317,7 @@ page_init(void)
 	 * Figure 2:
 	 * 
 	 *       page N    +--------------------+
-	 *                 |      pp_link       | -----\
+	 *                 |      pp_next       | -----\
 	 *                 +--------------------+      |
 	 *                 |      pp_ref        |      |
 	 *                 +--------------------+      |
@@ -320,7 +326,7 @@ page_init(void)
 	 *                           |
 	 *                           v
 	 *       page N-1  +--------------------+
-	 *                 |      pp_link       | -----\
+	 *                 |      pp_next       | -----\
 	 *                 +--------------------+      |
 	 *                 |      pp_ref        |      |
 	 *                 +--------------------+      |
@@ -333,7 +339,7 @@ page_init(void)
 	 *                           |
 	 *                           v
 	 *       page 1    +--------------------+
-	 *                 |      pp_link       | -----\
+	 *                 |      pp_next       | -----\
 	 *                 +--------------------+      |
 	 *                 |      pp_ref        |      |
 	 *                 +--------------------+      |
@@ -342,7 +348,7 @@ page_init(void)
 	 *                           |
 	 *                           v
 	 *       page 0    +--------------------+
-	 *                 |    pp_link=NULL    |
+	 *                 |    pp_next=NULL    |
 	 *                 +--------------------+
 	 *                 |      pp_ref        |
 	 *                 +--------------------+
@@ -358,7 +364,7 @@ page_init(void)
 		// The rest of base memory, [PGSIZE, npages_basemem * PGSIZE)
 		// is free.
 		pages[i].pp_ref = 0;
-		pages[i].pp_link = page_free_list;
+		pages[i].pp_next = page_free_list;
 		page_free_list = &pages[i];
 	}
 
@@ -368,13 +374,13 @@ page_init(void)
 		// in physcial memory. So initialize their corresponding 
 		// page entries in one loop
 		pages[i].pp_ref = 1;
-		pages[i].pp_link = NULL;
+		pages[i].pp_next = NULL;
 	}
 	
 	for (; i < npages; ++i) {
 		// The rest of extended memory are not used
 		pages[i].pp_ref = 0;
-		pages[i].pp_link = page_free_list;
+		pages[i].pp_next = page_free_list;
 		page_free_list = &pages[i];
 	}
 }
@@ -385,7 +391,7 @@ page_init(void)
 // count of the page - the caller must do these if necessary (either explicitly
 // or via page_insert).
 //
-// Be sure to set the pp_link field of the allocated page to NULL so
+// Be sure to set the pp_next field of the allocated page to NULL so
 // page_free can check for double-free bugs.
 //
 // Returns NULL if out of free memory.
@@ -398,8 +404,8 @@ page_alloc(int alloc_flags)
 		return NULL;
 	
 	struct PageInfo *page_allocated = page_free_list;
-	page_free_list = page_free_list->pp_link;
-	page_allocated->pp_link = NULL;
+	page_free_list = page_free_list->pp_next;
+	page_allocated->pp_next = NULL;
 
 	if (alloc_flags & ALLOC_ZERO)
 		memset(page2kva(page_allocated), 0, PGSIZE);
@@ -416,9 +422,9 @@ page_free(struct PageInfo *pp)
 {
 	// Fill this function in
 	// Hint: You may want to panic if pp->pp_ref is nonzero or
-	// pp->pp_link is not NULL.
+	// pp->pp_next is not NULL.
 
-	if (pp->pp_link)		// pp->pp_link is not NULL, double free
+	if (pp->pp_next)		// pp->pp_next is not NULL, double free
 		panic("Double free page %u, pvaddr:%x, ppaddr:%x\n",\
 			(uint32_t)(pp-pages), page2kva(pp), page2pa(pp));
 	
@@ -426,7 +432,7 @@ page_free(struct PageInfo *pp)
 		panic("Free a page in use, pnum:%u, pvaddr:%x, ppaddr:%x\n",\
 			(uint32_t)(pp-pages), page2kva(pp), page2pa(pp));
 	
-	pp->pp_link = page_free_list;
+	pp->pp_next = page_free_list;
 	page_free_list = pp;
 	return;
 }
@@ -641,6 +647,252 @@ tlb_invalidate(pde_t *pgdir, void *va)
 	invlpg(va);
 }
 
+// --------------------------------------------------------------
+// Buddy system functions
+// --------------------------------------------------------------
+
+/*
+ * Each doubly linked list is represented by a PageInfo **ptr.
+ * list_push_front(PageInfo **list_ptr, PageInfo *node_ptr) pushes
+ * *node_ptr into the heade the list.
+ */
+struct PageInfo *
+list_push_front(struct PageInfo **list_ptr, struct PageInfo *node_ptr)
+{
+	struct PageInfo *head_ptr = *list_ptr;
+	node_ptr->pp_next = head_ptr;
+	node_ptr->pp_prev = NULL;
+
+	*list_ptr = node_ptr;
+
+	if (head_ptr == NULL)
+		return node_ptr;
+
+	head_ptr->pp_prev = node_ptr;
+
+	return node_ptr;
+}
+
+/*
+ * list_pop_front(PageInfo **list_ptr) pops out the head of the linked
+ * list and sets *list_ptr to head_ptr->pp_next. NULL is returned if
+ * list is empty.
+ */
+struct PageInfo *
+list_pop_front(struct PageInfo **list_ptr)
+{
+	struct PageInfo *head_ptr = *list_ptr;
+	if (head_ptr == NULL)
+		return NULL;
+
+	*list_ptr = head_ptr->pp_next;
+	
+	if (*list_ptr)
+		(*list_ptr)->pp_prev = NULL;
+	
+	head_ptr->pp_next = NULL;
+	return head_ptr;
+}
+
+/*
+ * Remove *node_ptr from list_ptr.
+ * This function doesn't check whether node_ptr really belongs
+ * to list_ptr. The caller should guarantee the validation of
+ * arguments. It's the caller's duty to assure node_ptr really
+ * belongs to list_ptr.
+ * 
+ * node_ptr->pp_prev and node_ptr->pp_next will be set to NULL.
+ * 
+ * if *list_ptr == NULL, node_ptr is return immediately.
+ */
+struct PageInfo *
+list_remove(struct PageInfo **list_ptr, struct PageInfo *node_ptr)
+{
+	// if list is an empty list
+	if (*list_ptr == NULL)
+		return node_ptr;
+
+	// if node_ptr is the first of the linked list, just pop it
+	if (*list_ptr == node_ptr)
+		return list_pop_front(list_ptr);
+	
+	// Last condition check assures node_ptr is not the first node
+	// in the linked list. So it's safe to use node_ptr->pp_prev.
+	if (node_ptr->pp_prev)
+		node_ptr->pp_prev->pp_next = node_ptr->pp_next;
+	
+	// if node_ptr is not the last node in the linked list.
+	if (node_ptr->pp_next)
+		node_ptr->pp_next->pp_prev = node_ptr->pp_prev;
+
+
+		
+	node_ptr->pp_prev = NULL;
+	node_ptr->pp_next = NULL;
+	
+	return node_ptr;
+}
+
+/*
+ * return the length of a linked list. An end of a linked list is
+ * represented by NULL. The caller should guarantee the security of
+ * the arugments.
+ */
+size_t
+list_length(struct PageInfo **list_ptr)
+{
+	size_t count = 0;
+	for (struct PageInfo *node_ptr = *list_ptr; node_ptr; node_ptr = node_ptr->pp_next)
+		++count;
+	return count;
+}
+
+
+/*
+ * get n order buddy
+ */
+struct PageInfo *
+get_buddy(struct PageInfo *pp, int order)
+{
+	// assure order provided resides in appropriate range
+	assert(order <= MAX_BUDDY_ORDER);
+	
+	size_t buddy_bit = (1 << order);
+	size_t index = page2index(pp);
+
+	// assure the caller demand a test on pp with appropriate order
+	assert((index &(buddy_bit - 1)) == 0);
+	
+	size_t buddy_index = index ^ buddy_bit;
+	
+	// The tree constructed by the buddy system may even not be a 
+	// complete binary tree because our memory is not always 2^N
+	// bytes big. It‘s necessary to assure the physical memory
+	// represented by the buddy really exists.
+	return (buddy_index < npages) ? (pages + buddy_index) : NULL;
+}
+
+/*
+ * check whether a buddy page area is N-th order free
+ * NUll is returned if not.
+ */
+struct PageInfo *
+buddy_is_free(struct PageInfo *pp, int order)
+{
+	struct PageInfo *buddy = get_buddy(pp, order);
+	
+	return (buddy != NULL && buddy->pp_count == (1 << order)) ? buddy : NULL;
+}
+
+/*
+ * initialize nOrder_free_pages
+ * free page is always inserted to the header of a linked list
+ * pp_free should be the linked list provided by page_init()
+ */
+void buddy_tree_init(struct PageInfo *pp_free)
+{
+	struct PageInfo *tmp_ptr;
+
+	// nOrder_free_pages == &nOrder_free_pages[0]
+	while (pp_free) {
+		pp_free->pp_count = 1;
+		tmp_ptr = pp_free->pp_next;
+		*nOrder_free_pages = list_push_front(nOrder_free_pages, pp_free);
+		pp_free = tmp_ptr;
+	}
+
+	for (int order = 0; nOrder_free_pages[order] && order < MAX_BUDDY_ORDER; ++order) {
+		int next_order = order + 1;
+		struct PageInfo *buddy_pp;
+		pp_free = nOrder_free_pages[order];
+
+		while (pp_free) {
+			if ((buddy_pp = buddy_is_free(pp_free, order)) != NULL) {
+				// remove [pp_free, buddy_pp] from nOrder_free_pages[this order]
+				list_remove(&nOrder_free_pages[order], pp_free);
+				list_remove(&nOrder_free_pages[order], buddy_pp);
+
+				// sort [pp_free, buddy_pp]
+				struct PageInfo *lower_addr = MIN(buddy_pp, pp_free);
+				
+				// increment the order of free size
+				lower_addr->pp_count = ((lower_addr->pp_count) << 1);
+
+				// insert the new block into the linked list of next order
+				list_push_front(&nOrder_free_pages[order+1], lower_addr);
+
+				pp_free = nOrder_free_pages[order];
+			} else
+				pp_free = pp_free->pp_next;
+		}
+	}
+}
+
+/*
+ * allocate order free page
+ * return NULL if allocation failed
+ */
+struct PageInfo *
+buddy_alloc_page(int alloc_flags, int order)
+{
+	struct PageInfo **best_fit_list = &nOrder_free_pages[order];
+	int best_fit_order = order;
+
+	// find best fit list
+	while (*best_fit_list == NULL && best_fit_order <= MAX_BUDDY_ORDER) {
+		++best_fit_order;
+		++best_fit_list;
+	}
+
+	// no contiguous memory big enough is available
+	if (best_fit_order > MAX_BUDDY_ORDER)
+		return NULL;
+	
+	// break best fit memory block into 2^order big
+	struct PageInfo *big_block = list_pop_front(best_fit_list);
+	for (; best_fit_order > order; --best_fit_order) {
+		struct PageInfo *lower_order_buddy = &pages[(big_block - pages) ^ (1 << (best_fit_order - 1))];
+		lower_order_buddy->pp_count = (1 << (best_fit_order - 1));
+		list_push_front(--best_fit_list, lower_order_buddy);
+	}
+
+	big_block->pp_count = (1 << order);
+	if (alloc_flags & ALLOC_ZERO)
+		memset(page2kva(big_block), 0, (big_block->pp_count) * PGSIZE);
+
+	return big_block;
+}
+
+/*
+ * free page back to buddy tree
+ * it's the caller's responsibility to assure
+ * pp != NULL, order <= MAX_BUDDY_ORDER and the
+ * contiguous memory to be freed beginning from
+ * pp is truly 2^order big.
+ */
+void
+buddy_free_page(struct PageInfo *pp, int order)
+{
+	//cprintf("pp num: %d    order: %d\n", pp-pages, order);
+	while (order < MAX_BUDDY_ORDER) {
+		struct PageInfo *buddy = buddy_is_free(pp, order);
+		//cprintf("buddy == NULL: %d\n", buddy == NULL);
+		//cprintf("order %d list length: %d\n", order, list_length(&nOrder_free_pages[order]));
+		if (buddy == NULL)
+			break;
+		list_remove(&nOrder_free_pages[order], buddy);
+		pp = MIN(pp, buddy);
+		++order;
+		pp->pp_count = (1<<order);
+	}
+
+	// avoid double free
+	struct PageInfo *tmp_buddy = get_buddy(pp, order);
+	if (tmp_buddy != NULL && tmp_buddy->pp_count > pp->pp_count)
+		return;
+	
+	list_push_front(&nOrder_free_pages[order], pp);
+}
 
 // --------------------------------------------------------------
 // Checking functions.
@@ -665,10 +917,10 @@ check_page_free_list(bool only_low_memory)
 		// list, since entry_pgdir does not map all pages.
 		struct PageInfo *pp1, *pp2;
 		struct PageInfo **tp[2] = { &pp1, &pp2 };
-		for (pp = page_free_list; pp; pp = pp->pp_link) {
+		for (pp = page_free_list; pp; pp = pp->pp_next) {
 			int pagetype = PDX(page2pa(pp)) >= pdx_limit;
 			*tp[pagetype] = pp;
-			tp[pagetype] = &pp->pp_link;
+			tp[pagetype] = &pp->pp_next;
 		}
 		*tp[1] = 0;
 		*tp[0] = pp2;
@@ -677,12 +929,12 @@ check_page_free_list(bool only_low_memory)
 
 	// if there's a page that shouldn't be on the free list,
 	// try to make sure it eventually causes trouble.
-	for (pp = page_free_list; pp; pp = pp->pp_link)
+	for (pp = page_free_list; pp; pp = pp->pp_next)
 		if (PDX(page2pa(pp)) < pdx_limit)
 			memset(page2kva(pp), 0x97, 128);
 
 	first_free_page = (char *) boot_alloc(0);
-	for (pp = page_free_list; pp; pp = pp->pp_link) {
+	for (pp = page_free_list; pp; pp = pp->pp_next) {
 		// check that we didn't corrupt the free list itself
 		assert(pp >= pages);
 		assert(pp < pages + npages);
@@ -707,6 +959,7 @@ check_page_free_list(bool only_low_memory)
 	cprintf("check_page_free_list() succeeded!\n");
 }
 
+
 //
 // Check the physical page allocator (page_alloc(), page_free(),
 // and page_init()).
@@ -724,7 +977,7 @@ check_page_alloc(void)
 		panic("'pages' is a null pointer!");
 
 	// check number of free pages
-	for (pp = page_free_list, nfree = 0; pp; pp = pp->pp_link)
+	for (pp = page_free_list, nfree = 0; pp; pp = pp->pp_next)
 		++nfree;
 
 	// should be able to allocate three pages
@@ -778,7 +1031,7 @@ check_page_alloc(void)
 	page_free(pp2);
 
 	// number of free pages should be the same
-	for (pp = page_free_list; pp; pp = pp->pp_link)
+	for (pp = page_free_list; pp; pp = pp->pp_next)
 		--nfree;
 	assert(nfree == 0);
 
@@ -958,7 +1211,7 @@ check_page(void)
 	// test re-inserting pp1 at PGSIZE
 	assert(page_insert(kern_pgdir, pp1, (void*) PGSIZE, 0) == 0);
 	assert(pp1->pp_ref);
-	assert(pp1->pp_link == NULL);
+	assert(pp1->pp_next == NULL);
 
 	// unmapping pp1 at PGSIZE should free it
 	page_remove(kern_pgdir, (void*) PGSIZE);
@@ -1050,3 +1303,113 @@ check_page_installed_pgdir(void)
 
 	cprintf("check_page_installed_pgdir() succeeded!\n");
 }
+
+/*
+ * check whether the buddy tree is established correctly.
+ */
+static void
+check_buddy_tree(void)
+{
+
+	size_t free_count = 0;	// number of free pages
+	size_t used_count = 0;	// number of used pages
+
+	// assure values of elements of PageInfo is consistent
+	// with each other and their own criteria.
+	for (size_t i = 0; i < npages; ++i) {
+		(pages[i].pp_ref == 0 && pages[i].pp_count > 0) ? ++free_count : ++used_count;
+
+		// pages[i].pp_count should be either zero or the power of two.
+		// It should not be larger than 2^MAX_BUDDY_ORDER.
+		// We should keep in mind that pages[i].pp_count is an unsigend integer.
+		assert(((pages[i].pp_count <= (1 << MAX_BUDDY_ORDER)) &&
+			((pages[i].pp_count & (pages[i].pp_count - 1)) == 0)));
+
+		// pages[i].pp_ref == 0 means page i is free
+		// pages[i].pp_count > 0 means page is 2^pp_count
+		// the boolean value of these two equations should be always the same.
+		assert(((pages[i].pp_ref == 0) == (pages[i].pp_count > 0)));
+	}
+
+	// assure free_count and used_count sums up to npages
+	assert((free_count + used_count == npages));
+
+	// count free pages from buddy tree
+	// check each page's buddy page is not free except for those at MAX_BUDDY_ORDER
+	size_t buddy_free_count = 0;
+	for (int order = 0; order <= MAX_BUDDY_ORDER; ++order) {
+		size_t counter = 0;
+		
+		for (struct PageInfo *node_ptr = nOrder_free_pages[order]; node_ptr; node_ptr = node_ptr->pp_next) {
+			++counter;
+			
+			assert((order == MAX_BUDDY_ORDER || (buddy_is_free(node_ptr, order) == NULL)));
+		}
+		
+		buddy_free_count += counter * (1 << order);
+	}
+
+	assert(buddy_free_count == free_count);
+
+/*
+	#define BYTE_UNIT	(1)
+	#define KB_UNIT		(BYTE_UNIT << 10)
+	#define MB_UNIT		(KB_UNIT << 10)
+
+	int free_memory = free_count * PGSIZE;
+	cprintf("free memory: %dMB", free_memory/MB_UNIT);
+	free_memory = free_memory % MB_UNIT;
+	cprintf(" %dKB", free_memory/KB_UNIT);
+	free_memory = free_memory % KB_UNIT;
+	cprintf(" %dB\n", free_memory);
+*/
+
+	cprintf("check_buddy_tree() succeeded!\n");
+}
+
+/*
+ * check buddy_alloc, buddy_free
+ */
+static void
+check_buddy_alloc(void)
+{
+	size_t free_page_count = 0;
+	size_t free_count_in_each_order[MAX_BUDDY_ORDER+1];
+	memset(free_count_in_each_order, 0, sizeof(free_count_in_each_order));
+
+	// record free page number
+	for (size_t order = 0; order <= MAX_BUDDY_ORDER; ++order) {
+		size_t counter = list_length(&nOrder_free_pages[order]);
+		
+		free_count_in_each_order[order] = counter;
+		
+		free_page_count += counter * (1 << order);
+	}
+	/*
+	cprintf("free page count: %d\n", free_page_count);
+	for (int i = 0; i <= MAX_BUDDY_ORDER; ++i)
+		cprintf("order: %d    count: %d\n", i, free_count_in_each_order[i]);
+	*/
+
+	// exhaust memory
+	size_t alloc_times = 0;
+	while (buddy_alloc_page(0, 0))
+		++alloc_times;
+
+	assert((alloc_times == free_page_count));
+
+	// free memory allocated before
+	for (size_t i = 0; i < npages; ++i) {
+		if (pages[i].pp_count == 1)
+			buddy_free_page(&pages[i], 0);
+	}
+
+	// check whether buddy tree is restored after freeing pages
+	for (size_t order = 0; order <= MAX_BUDDY_ORDER; ++order) {
+		size_t counter = list_length(&nOrder_free_pages[order]);
+
+		assert((counter == free_count_in_each_order[order]));
+	}
+
+	cprintf("check_buddy_alloc&free() succeed!\n");
+}
\ No newline at end of file
diff --git a/kern/pmap.h b/kern/pmap.h
index 9d447fc..da39b34 100644
--- a/kern/pmap.h
+++ b/kern/pmap.h
@@ -90,4 +90,33 @@ page2kva(struct PageInfo *pp)
 
 pte_t *pgdir_walk(pde_t *pgdir, const void *va, int create);
 
+
+/*
+ * Some concept in buddy system
+ * The minimal memory unit is a page(4KB).
+ * 2^order pages is allocated each time.
+ * The N-th order buddy page is the buddy of a page which is the
+ * first page of 2^order contiguous pages. Thus the buddy's order
+ * of a page can not be arbitrarily large. For eaxmple, (110)_2 page
+ * has 2-th order buddy page at most.
+ * Another trick we should point out is that if bit-N(counting from
+ * left and N starts at zero) is the first non-zero bit of index.
+ * Then this page has N-th buddy page at most. And by toggling bit-N,
+ * we get the index of its buddy page.
+ * If a page is the first page of 2^N contiguous pages, we call
+ * the page is N-th order free.
+ */
+
+#define MAX_BUDDY_ORDER	10	// the biggest guaranteed memory is 2^10 pages, i.e. 4MB
+
+struct PageInfo * buddy_is_free(struct PageInfo *pp, int order);
+void buddy_tree_init(struct PageInfo *pp_free);
+struct PageInfo *buddy_alloc_page(int alloc_flags, int order);
+void buddy_free_page(struct PageInfo *pp, int order);
+
+/*
+ * This macro takes a PageInfo pointer and returns its index in the pages array.
+ */
+#define page2index(pp)	((pp) - pages)
+
 #endif /* !JOS_KERN_PMAP_H */
```

## Grade

Finally, we got our grade.

```bash
running JOS: (2.2s)
  Physical page allocator: OK
  Page management: OK
  Kernel page directory: OK
  Page management 2: OK
Score: 70/70
```

```python {cmd=true hide=true run_on_save=true output="html" continue="global"}

footerStyle = "width:100%;text-align:center;font-family:Times"
footerTemplate = '<footer style="{_style}">End of {_title}<br/>Email: <a mailto="caoshuyang1996@pku.edu.cn">caoshuyang@pku.edu.cn</a> GitHub: <a href="https://github.com/CaoSY/JOS-Lab" title="JOS Lab">JOS-Lab</a></footer>'

# print footer
print(footerTemplate.format(_style=footerStyle, _title=title))
```