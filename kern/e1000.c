#include <kern/e1000.h>
#include <inc/error.h>
#include <inc/string.h>

#define DEBUG 0
#define TEST_TX 0

volatile uint32_t *e1000;

struct e1000_tx_desc tx_descs[NTDESC];
uint8_t tx_packets[NTDESC][MAXPKTLEN];

struct e1000_tx_desc rx_descs[NRDESC];
uint8_t rx_packets[NRDESC][MAXPKTLEN];

// store mac address
uint8_t e1000_mac[E1000_MAC_LENGTH];

// LAB 6: Your driver code here
int
e1000_attach(struct pci_func *pcif)
{
    pci_func_enable(pcif);
    e1000 = mmio_map_region(pcif->reg_base[0], pcif->reg_size[0]);
    cprintf("Status is 0x%08x %s Desired: 0x80080783\n",
        e1000[E1000_STATUS], e1000[E1000_STATUS] == 0x80080783 ? "==" : "!=");

    static_assert(sizeof(tx_descs) % 128 == 0); // should be 128-byte aligned
    static_assert(sizeof(rx_descs) % 128 == 0); // should be 128-byte aligned

    // perform transmit initialization
    e1000[E1000_TDBAL] = PADDR(tx_descs);
    e1000[E1000_TDLEN] = sizeof(tx_descs);
    e1000[E1000_TDH] = 0;
    e1000[E1000_TDT] = 0;
    e1000[E1000_TCTL] |= E1000_TCTL_EN;
    e1000[E1000_TCTL] |= E1000_TCTL_PSP;
    e1000[E1000_TCTL] |= E1000_TCTL_CT_INIT;
    e1000[E1000_TCTL] |= E1000_TCTL_COLD_INIT;
    e1000[E1000_TIPG] |= E1000_TIPG_INIT;

    // init transmit descriptors
    memset(tx_descs, 0, sizeof(tx_descs));
    for (size_t i = 0; i < NTDESC; ++i) {
        tx_descs[i].addr = PADDR(tx_packets[i]);
        tx_descs[i].cmd |= E1000_TXD_CMD_RS;
        tx_descs[i].status |= E1000_TXD_STA_DD;
    }

    // perform receive initialization
    *(uint16_t *)e1000_mac = e1000_read_eeprom(E1000_EEPROM_MAC_ADDR_BYTE_2_1);
    *(uint16_t *)(e1000_mac + 2) = e1000_read_eeprom(E1000_EEPROM_MAC_ADDR_BYTE_4_3);
    *(uint16_t *)(e1000_mac + 4) = e1000_read_eeprom(E1000_EEPROM_MAC_ADDR_BYTE_6_5);
    e1000[E1000_RAL] = *(uint32_t *)e1000_mac;
    e1000[E1000_RAH] = *(uint16_t *)(e1000_mac + 4) | E1000_RAH_AV;
    e1000[E1000_RDBAL] = PADDR(rx_descs);
    e1000[E1000_RDLEN] = sizeof(rx_descs);
    e1000[E1000_RDH] = 0;
    e1000[E1000_RDT] = NRDESC - 1;
    e1000[E1000_RCTL] |= E1000_RCTL_EN;
    e1000[E1000_RCTL] &= ~E1000_RCTL_LPE;   // turn off long packet
    e1000[E1000_RCTL] |= E1000_RCTL_LBM_NO;
    e1000[E1000_RCTL] |= E1000_RCTL_RDMTS_HALF;
    e1000[E1000_RCTL] |= E1000_RCTL_MO_0;
    e1000[E1000_RCTL] |= E1000_RCTL_BAM;
    e1000[E1000_RCTL] |= E1000_RCTL_SZ_2048;
    e1000[E1000_RCTL] |= E1000_RCTL_SECRC;

    // init receive descriptors
    memset(rx_descs, 0, sizeof(rx_descs));
    for (size_t i = 0; i < NRDESC; ++i) {
        rx_descs[i].addr = PADDR(rx_packets[i]);
    }

    #if TEST_TX
        char int_packet[200];
        for (size_t i = 0; i < 200; ++i)
            int_packet[i] = i;
        for (size_t i = 0; i < 2 * NTDESC; ++i) {
            cprintf("transmit packet %d, length: %d\n", i, sizeof(int_packet));
            int_packet[0] = i;
            e1000_tx(int_packet, sizeof(int_packet));
        }
    #endif
    
    return 0;
}

// Return 0 on success.
// Return -E_TXD_ARRAY_FULL if transmit descriptor array is full.
int
e1000_tx(void *addr, size_t length)
{
    if (length > MAXPKTLEN)
        return -E_INVAL;
    
    size_t tail = e1000[E1000_TDT];

    #if DEBUG
        cprintf("e1000 transmit tail: %d\n", tail);
        cprintf("transmit tail status %0x\n", tx_descs[tail].status);
    #endif

    if (!(tx_descs[tail].status & E1000_TXD_STA_DD)) {
        cprintf("e1000 transmit descriptors array is full\n");
        return -E_TXD_ARRAY_FULL;
    }

    memcpy(KADDR(tx_descs[tail].addr), addr, length);
    tx_descs[tail].length = length;
    tx_descs[tail].status &= ~E1000_TXD_STA_DD;
    tx_descs[tail].cmd |= E1000_TXD_CMD_EOP;
    e1000[E1000_TDT] = (tail + 1) % NTDESC;

    return 0;
}

// Return length on success.
// Return -E_RXD_ARRAY_EMPTY if receive descriptor array is full.
int
e1000_rx(void *addr) {
    // To avoid tail catching up head which prevent the network
    // card from accepting packets, we let the tail always one slot
    // behind slots that are filled and update tail first when we
    // fetch a received packet. This could waste one slot.
    uint32_t tail = (e1000[E1000_RDT] + 1) % NRDESC;

    #if DEBUG
    cprintf("prepare to receive tail: %d\n", e1000[E1000_RDT]);
    cprintf("receive tail status %d0x\n", rx_descs[tail].status);
    #endif

    if (!(rx_descs[tail].status & E1000_RXD_STA_DD)) {
        // if the dd field is not set, there is nothing to receive
        return -E_RXD_ARRAY_EMPTY;
    }

    // assume there is no long packets
    assert(rx_descs[tail].status & E1000_RXD_STA_EOP);

    size_t length = rx_descs[tail].length;
    memcpy(addr, KADDR(rx_descs[tail].addr), length);
    rx_descs[tail].status &= ~E1000_RXD_STA_DD;
    rx_descs[tail].status &= ~E1000_RXD_STA_EOP;
    rx_descs[tail].status &= ~E1000_RXD_STA_IXSM;
    e1000[E1000_RDT] = tail;

    return length;
}

uint16_t
e1000_read_eeprom(uint8_t addr)
{
    e1000[E1000_EERD] = (addr << E1000_EERD_ADDR_SHIFT) | E1000_EERD_START;

    while ((e1000[E1000_EERD] & E1000_EERD_DONE) == 0)
        ;   /* polling */
    
    return e1000[E1000_EERD] >> E1000_EERD_DATA_SHIFT;
}