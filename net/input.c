#include "ns.h"

#define DEBUG 0

extern union Nsipc nsipcbuf;

void
input(envid_t ns_envid)
{
	binaryname = "ns_input";

	// LAB 6: Your code here:
	// 	- read a packet from the device driver
	//	- send it to the network server
	// Hint: When you IPC a page to the network server, it will be
	// reading from it for a while, so don't immediately receive
	// another packet in to the same physical page.

	struct jif_pkt *pkt = (struct jif_pkt *)REQVA;
	int r, i;
	
	while (true) {
		// we allocate a new physical page at pkt each time we
		// receive new data, so we don't need to wait a few schedule
		// cycles for the page being read, which is prone to be buggy.
		while((r = sys_page_alloc(0, pkt, PTE_P | PTE_U | PTE_W)) < 0) {
			if (r != -E_NO_MEM)
				panic("sys_page_alloc: %e\n", r);
			
			sys_yield();
		}

		while ((r = sys_net_receive(pkt->jp_data)) < 0) {
			if (r != -E_RXD_ARRAY_EMPTY)
				panic("Net receive failed. %e\n", r);
			
			sys_yield();
		}

		pkt->jp_len = r;

		#if DEBUG
		cprintf("ns req %d to %08x [page 0x%08x]\n",
			NSREQ_INPUT, ns_envid, uvpt[PGNUM(REQVA)]);
		#endif

		ipc_send(ns_envid, NSREQ_INPUT, pkt, PTE_U | PTE_P | PTE_W);
	}
}