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
	envid_t whom;
	struct jif_pkt *pkt = (struct jif_pkt*)REQVA;

	while(true) {
		int32_t req = ipc_recv(&whom, pkt, &perm);

		#if DEBUG
		cprintf("ns req %d from 0x%08x [page 0x%08x]\n",
			req, whom, uvpt[PGNUM(REQVA)]);
		#endif

		assert(whom == ns_envid);

		if (!(perm & PTE_P)) {
			#if DEBUG
			cprintf("Invalide request from [%08x]: no argument page\n", whom);
			#endif
			continue;
		}

		if (req == NSREQ_OUTPUT) {
			while ((r = sys_net_transmit(pkt->jp_data, pkt->jp_len)) < 0) {
				if (r != E_TXD_ARRAY_FULL)
					panic("Net transmit failed to output request of [%08x]\n", whom);
				#if DEBUG
				else
					cprintf("Output request of [%08x] failed\n", whom);
				#endif
			}
		}
		#if DEBUG
		else {
			cprintf("Invalid request code %d from [%08x]\n", req, whom);
		}
		#endif
	}
}
