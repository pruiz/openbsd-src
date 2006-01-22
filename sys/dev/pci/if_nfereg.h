/*	$OpenBSD: if_nfereg.h,v 1.4 2006/01/22 21:35:08 damien Exp $	*/
/*
 * Copyright (c) 2005 Jonathan Gray <jsg@openbsd.org>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#define NFE_PCI_BA		0x10

#define NFE_RX_RING_COUNT	128
#define NFE_TX_RING_COUNT	64

#define NFE_MAX_SCATTER		NFE_TX_RING_COUNT

#define NFE_IRQ_STATUS		0x000
#define NFE_IRQ_MASK		0x004
#define NFE_SETUP_R6		0x008
#define NFE_TIMER_INT		0x00c
#define NFE_MISC1		0x080
#define NFE_TX_CTL		0x084
#define NFE_TX_STATUS		0x088
#define NFE_MULTI_FLAGS		0x08c
#define NFE_OFFLD_CFG		0x090
#define NFE_RX_CTL		0x094
#define NFE_RX_STATUS		0x098
#define NFE_RDM_SEED		0x09c
#define NFE_SETUP_R1		0x0a0
#define NFE_SETUP_R2		0x0a4
#define NFE_MACADDR_HI		0x0a8
#define NFE_MACADDR_LO		0x0ac
#define NFE_MULT_ADDR1		0x0b0
#define NFE_MULT_ADDR2		0x0b4
#define NFE_MULT_MASK1		0x0b8
#define NFE_MULT_MASK2		0x0bc
#define NFE_PHY_INT		0x0c0
#define NFE_TX_RING_ADDR	0x100
#define NFE_RX_RING_ADDR	0x104
#define NFE_RING_SIZE		0x108
#define NFE_TX_UNK		0x10c
#define NFE_LINKSPEED		0x110
#define NFE_SETUP_R5		0x130
#define NFE_SETUP_R3		0x13C
#define NFE_SETUP_R7		0x140
#define NFE_RXTX_CTL		0x144
#define NFE_PHY_STATUS		0x180
#define NFE_SETUP_R4		0x184
#define NFE_STATUS		0x188
#define NFE_PHY_SPEED		0x18c
#define NFE_PHY_CTL		0x190
#define NFE_PHY_DATA		0x194
#define NFE_WOL_CTL		0x200
#define NFE_PATTERN_CRC		0x204
#define NFE_PATTERN_MASK	0x208
#define NFE_PWR_CAP		0x268
#define NFE_PWR_STATE		0x26c

#define NFE_PHY_ERROR		0x00001
#define NFE_PHY_WRITE		0x00400
#define NFE_PHY_BUSY		0x08000
#define NFE_PHYADD_SHIFT	5

#define NFE_R1_MAGIC		0x16070f
#define NFE_R2_MAGIC		0x16
#define NFE_R4_MAGIC		0x08
#define NFE_WOL_MAGIC		0x7770
#define NFE_RX_START		0x01
#define NFE_TX_START		0x01

#define NFE_IRQ_RXERR		0x0001
#define NFE_IRQ_RX		0x0002
#define NFE_IRQ_RX_NOBUF	0x0004
#define NFE_IRQ_TXERR		0x0008
#define NFE_IRQ_TX_DONE		0x0010
#define NFE_IRQ_TIMER		0x0020
#define NFE_IRQ_LINK		0x0040
#define NFE_IRQ_TXERR2		0x0080
#define NFE_IRQ_WANTED		0x00ff
#define NFE_IRQ_TX1		0x0100

#define NFE_RXTX_KICKTX		0x0001
#define NFE_RXTX_BIT1		0x0002
#define NFE_RXTX_BIT2		0x0004
#define NFE_RXTX_RESET		0x0010
#define NFE_RXTX_RXCHECK	0x0400
#define NFE_RXTX_V2MAGIC	0x2100
#define NFE_RXTX_V3MAGIC	0x2200

#define NFE_MC_ALWAYS		0x7F0008
#define NFE_MC_PROMISC		0x000080
#define NFE_MC_MYADDR		0x000020

#define NFE_MEDIA_SET		0x10000
#define	NFE_MEDIA_1000T		0x00032
#define NFE_MEDIA_100TX		0x00064
#define NFE_MEDIA_10T		0x003e8

#define NFE_PHY_100TX		0x01
#define NFE_PHY_1000T		0x02

#define NFE_RDM_SEED_FORCE1	0x7f00
#define NFE_RDM_SEED_FORCE2	0x2d00
#define NFE_RDM_SEED_FORCE3	0x7400

/* Rx/Tx descriptor */
struct nfe_desc32 {
	uint32_t	physaddr;
	uint16_t	length;
	uint16_t	flags;
#define NFE_RX_VALID_V1		(1 <<  0)
#define NFE_RX_VALID_V2		(1 << 13)
#define NFE_RX_ERROR		(1 << 14)
#define NFE_RX_READY		(1 << 15)
#define NFE_RX_CSUMOK		0x1c00
#define NFE_RX_FIXME_V1		0x6004
#define NFE_RX_FIXME_V2		0x4300
} __packed;

/* V2 Rx/Tx descriptor */
struct nfe_desc64 {
	uint32_t	physaddr[2];
	uint32_t	reserved;
	uint16_t	length;
	uint16_t	flags;
#define NFE_TX_LASTFRAG_V1	(1 <<  0)
#define NFE_TX_TCP_CSUM		(1 << 10)
#define NFE_TX_IP_CSUM		(1 << 11)
#define NFE_TX_LASTFRAG_V2	(1 << 13)
#define NFE_TX_VALID		(1 << 15)
#define NFE_TX_ERROR_V1		0x7808
#define NFE_TX_ERROR_V2		0x5c04
} __packed;

#define NFE_READ(sc, reg) \
	bus_space_read_4((sc)->sc_memt, (sc)->sc_memh, (reg))

#define NFE_WRITE(sc, reg, val) \
	bus_space_write_4((sc)->sc_memt, (sc)->sc_memh, (reg), (val))
