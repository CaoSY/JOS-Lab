#ifndef JOS_KERN_E1000_H
#define JOS_KERN_E1000_H

#include <kern/pci.h>
#include <kern/pmap.h>

#define E1000_VENDOR_ID     0x8086
#define E1000_DEV_ID_82540EM             0x100E

/* The value given in the manual is byte indexed. In our code, we use uint32_t index.*/
#define E1000_STATUS    (0x00008 / sizeof(uint32_t))  /* Device Status - RO */

/* Transmit Control Register */
/* The value given in the manual is byte indexed. In our code, we use uint32_t index.*/
#define E1000_TDBAL     (0x03800 / sizeof(uint32_t))  /* TX Descriptor Base Address Low - RW */
#define E1000_TDLEN     (0x03808 / sizeof(uint32_t))  /* TX Descriptor Length - RW */ 
#define E1000_TDH       (0x03810 / sizeof(uint32_t))  /* TX Descriptor Head - RW */
#define E1000_TDT       (0x03818 / sizeof(uint32_t))  /* TX Descriptor Tail - RW */ 
#define E1000_TCTL      (0x00400 / sizeof(uint32_t))  /* TX Control - RW */
#define E1000_TIPG      (0x00410 / sizeof(uint32_t))  /* Transmit IPG Register */


/* Transmit Control */
#define E1000_TCTL_RST    0x00000001    /* software reset */
#define E1000_TCTL_EN     0x00000002    /* enable tx */
#define E1000_TCTL_BCE    0x00000004    /* busy check enable */
#define E1000_TCTL_PSP    0x00000008    /* pad short packets */
#define E1000_TCTL_CT     0x00000ff0    /* collision threshold */
#define E1000_TCTL_COLD   0x003ff000    /* collision distance */
#define E1000_TCTL_SWXOFF 0x00400000    /* SW Xoff transmission */
#define E1000_TCTL_PBE    0x00800000    /* Packet Burst Enable */
#define E1000_TCTL_RTLC   0x01000000    /* Re-transmit on late collision */
#define E1000_TCTL_NRTU   0x02000000    /* No Re-transmit on underrun */
#define E1000_TCTL_MULR   0x10000000    /* Multiple request support */

#define E1000_TCTL_CT_INIT      0x00000010      /* initial collision threshold */
#define E1000_TCTL_COLD_INIT    0x00040000      /* initial collision distance */
#define E1000_TIPG_INIT         0x0060200a      /* init values for TIPG in 13.4.34 */

#define E1000_TXD_CMD_RS    0x08    /* Transmit Desc Report Status */
#define E1000_TXD_CMD_EOP   0x01    /* Transmit Desc End of Packet */
#define E1000_TXD_STA_DD    0x1     /* Transmit Desc Status DD field */

/* Receive Control Register */
/* The value given in the manual is byte indexed. In our code, we use uint32_t index.*/
#define E1000_RCTL     (0x00100 / sizeof(uint32_t)) /* RX Control - RW */
#define E1000_RDBAL    (0x02800 / sizeof(uint32_t)) /* RX Descriptor Base Address Low - RW */
#define E1000_RDLEN    (0x02808 / sizeof(uint32_t)) /* RX Descriptor Length - RW */
#define E1000_RDH      (0x02810 / sizeof(uint32_t)) /* RX Descriptor Head - RW */
#define E1000_RDT      (0x02818 / sizeof(uint32_t)) /* RX Descriptor Tail - RW */
#define E1000_RA       (0x05400 / sizeof(uint32_t)) /* Receive Address - RW Array */
#define E1000_RAL      (0x05400 / sizeof(uint32_t)) /* Receive Address Low - RW */
#define E1000_RAH      (0x05404 / sizeof(uint32_t)) /* Receive Address HIGH - RW */

/* Receive Control Registers */
#define E1000_RCTL_EN			0x00000002    /* enable */
#define E1000_RCTL_LPE			0x00000020    /* long packet enable */
#define E1000_RCTL_LBM_NO		0x00000000    /* no loopback mode */
#define E1000_RCTL_RDMTS_HALF	0x00000000    /* rx desc min threshold size */
#define E1000_RCTL_MO_0			0x00000000    /* multicast offset 11:0 */
#define E1000_RCTL_BAM			0x00008000    /* broadcast enable */
#define E1000_RCTL_SECRC		0x04000000    /* Strip Ethernet CRC */
#define E1000_RCTL_BSEX			0x02000000    /* Buffer size extension */
#define E1000_RCTL_SZ_2048      0x00000000    /* rx buffer size 2048 */
#define E1000_RCTL_SZ_4096      0x00030000    /* rx buffer size 4096 */
#define E1000_RAH_AV			0x80000000    /* Receive descriptor valid */

#define E1000_RXD_STA_DD        0x01    /* Descriptor Done */
#define E1000_RXD_STA_EOP       0x02    /* End of Packet */
#define E1000_RXD_STA_IXSM      0x04    /* Ignore Checksum Indication */


#define NTDESC      64      // the number of the transmit descriptors, somehow arbitrary,
                            // According lab 6 instruction(Exercise 5), NTDESC should not be larger than 64
#define NRDESC      128     // the number of receive descriptors, Exercise 10 claims at least 128
#define MAXPKTLEN   1518    // The maximum size of an Ethernet packet

#define E_TXD_ARRAY_FULL    1   // Transmit Descriptor Array is full
#define E_RXD_ARRAY_FULL    2   // Receive Descriptor Array is full

/* Transmit Descriptor */
struct e1000_tx_desc {
    uint64_t addr;          /* Address of the descriptor's data buffer */
    uint16_t length;        /* Data buffer length */
    uint8_t cso;            /* Checksum offset */
    uint8_t cmd;            /* Descriptor control */
    uint8_t status;         /* Descriptor status */
    uint8_t css;            /* Checksum start */
    uint16_t special;
}__attribute__((packed));

/* Receive Descriptor */
struct e1000_rx_desc {
    uint64_t buffer_addr;   /* Address of the descriptor's data buffer */
    uint16_t length;        /* Length of data DMAed into data buffer */
    uint16_t csum;          /* Packet checksum */
    uint8_t status;         /* Descriptor status */
    uint8_t errors;         /* Descriptor Errors */
    uint16_t special;
}__attribute__((packed));


int e1000_attach(struct pci_func *pcif);
int e1000_tx(void *addr, size_t length);
int e1000_rx(void *addr);

#endif	// JOS_KERN_E1000_H
