#include "ns.h"

#define DEBUG 0

extern union Nsipc nsipcbuf;

void
output(envid_t ns_envid)
{
	binaryname = "ns_output";

	// LAB 6: Your code here:
	// 	- read a packet from the network server
	//	- send the packet to the device driver

	int r, perm;
	uint32_t req;
	envid_t whom;
	void *pg;
	struct jif_pkt *pkt = (struct jif_pkt*)REQVA;

	while(true) {
		req = ipc_recv(&whom, (void *)REQVA, &perm);

		#if DEBUG
		cprintf("ns req %d from 0x%08x [page 0x%08x]\n",
			req, whom, uvpt[PGNUM(REQVA)]);
		#endif

		assert(whom == ns_envid);

		if (!(perm & PTE_P)) {
			cprintf("Invalide request from [%08x]: no argument page\n", whom);
			continue;
		}

		pg = NULL;

		if (req == NSREQ_OUTPUT) {
			if ((r = sys_net_transmit(pkt->jp_data, pkt->jp_len)) < 0)
				cprintf("Output request of [%08x] failed\n", whom);
		} else {
			cprintf("Invalid request code %d from [%08x]\n", req, whom);
		}
	}
}
