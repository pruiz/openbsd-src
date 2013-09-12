/* $OpenBSD: if_cpswreg.h,v 1.3 2013/09/12 03:16:59 dlg Exp $ */

/*-
 * Copyright (c) 2012 Damjan Marion <dmarion@Freebsd.org>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#ifndef	_IF_CPSWREG_H
#define	_IF_CPSWREG_H

#define CPSW_SS_OFFSET			0x0000
#define CPSW_SS_IDVER			(CPSW_SS_OFFSET + 0x00)
#define CPSW_SS_SOFT_RESET		(CPSW_SS_OFFSET + 0x08)
#define CPSW_SS_STAT_PORT_EN		(CPSW_SS_OFFSET + 0x0C)
#define CPSW_SS_PTYPE			(CPSW_SS_OFFSET + 0x10)

#define CPSW_PORT_OFFSET		0x0100
#define CPSW_PORT_P_TX_PRI_MAP(p)	(CPSW_PORT_OFFSET + 0x118 + ((p-1) * 0x100))
#define CPSW_PORT_P0_CPDMA_TX_PRI_MAP	(CPSW_PORT_OFFSET + 0x01C)
#define CPSW_PORT_P0_CPDMA_RX_CH_MAP	(CPSW_PORT_OFFSET + 0x020)
#define CPSW_PORT_P_SA_LO(p)		(CPSW_PORT_OFFSET + 0x120 + ((p-1) * 0x100))
#define CPSW_PORT_P_SA_HI(p)		(CPSW_PORT_OFFSET + 0x124 + ((p-1) * 0x100))

#define CPSW_CPDMA_OFFSET		0x0800
#define CPSW_CPDMA_TX_CONTROL		(CPSW_CPDMA_OFFSET + 0x04)
#define CPSW_CPDMA_TX_TEARDOWN		(CPSW_CPDMA_OFFSET + 0x08)
#define CPSW_CPDMA_RX_CONTROL		(CPSW_CPDMA_OFFSET + 0x14)
#define CPSW_CPDMA_RX_TEARDOWN		(CPSW_CPDMA_OFFSET + 0x18)
#define CPSW_CPDMA_SOFT_RESET		(CPSW_CPDMA_OFFSET + 0x1c)
#define CPSW_CPDMA_DMACONTROL		(CPSW_CPDMA_OFFSET + 0x20)
#define CPSW_CPDMA_DMASTATUS		(CPSW_CPDMA_OFFSET + 0x24)
#define CPSW_CPDMA_RX_BUFFER_OFFSET	(CPSW_CPDMA_OFFSET + 0x28)
#define CPSW_CPDMA_TX_INTSTAT_RAW	(CPSW_CPDMA_OFFSET + 0x80)
#define CPSW_CPDMA_TX_INTSTAT_MASKED	(CPSW_CPDMA_OFFSET + 0x84)
#define CPSW_CPDMA_TX_INTMASK_SET	(CPSW_CPDMA_OFFSET + 0x88)
#define CPSW_CPDMA_TX_INTMASK_CLEAR	(CPSW_CPDMA_OFFSET + 0x8C)
#define CPSW_CPDMA_CPDMA_EOI_VECTOR	(CPSW_CPDMA_OFFSET + 0x94)
#define CPSW_CPDMA_RX_INTSTAT_RAW	(CPSW_CPDMA_OFFSET + 0xA0)
#define CPSW_CPDMA_RX_INTSTAT_MASKED	(CPSW_CPDMA_OFFSET + 0xA4)
#define CPSW_CPDMA_RX_INTMASK_SET	(CPSW_CPDMA_OFFSET + 0xA8)
#define CPSW_CPDMA_RX_INTMASK_CLEAR	(CPSW_CPDMA_OFFSET + 0xAc)
#define CPSW_CPDMA_DMA_INTSTAT_RAW	(CPSW_CPDMA_OFFSET + 0xB0)
#define CPSW_CPDMA_DMA_INTSTAT_MASKED	(CPSW_CPDMA_OFFSET + 0xB4)
#define CPSW_CPDMA_DMA_INTMASK_SET	(CPSW_CPDMA_OFFSET + 0xB8)
#define CPSW_CPDMA_DMA_INTMASK_CLEAR	(CPSW_CPDMA_OFFSET + 0xBC)
#define CPSW_CPDMA_RX_FREEBUFFER(p)	(CPSW_CPDMA_OFFSET + 0x0e0 + ((p) * 0x04))

#define CPSW_STATS_OFFSET		0x0900

#define CPSW_STATERAM_OFFSET		0x0A00
#define CPSW_CPDMA_TX_HDP(p)		(CPSW_STATERAM_OFFSET + 0x00 + ((p) * 0x04))
#define CPSW_CPDMA_RX_HDP(p)		(CPSW_STATERAM_OFFSET + 0x20 + ((p) * 0x04))
#define CPSW_CPDMA_TX_CP(p)		(CPSW_STATERAM_OFFSET + 0x40 + ((p) * 0x04))
#define CPSW_CPDMA_RX_CP(p)		(CPSW_STATERAM_OFFSET + 0x60 + ((p) * 0x04))

#define CPSW_CPTS_OFFSET		0x0C00

#define CPSW_ALE_OFFSET			0x0D00
#define CPSW_ALE_CONTROL		(CPSW_ALE_OFFSET + 0x08)
#define CPSW_ALE_TBLCTL			(CPSW_ALE_OFFSET + 0x20)
#define CPSW_ALE_TBLW2			(CPSW_ALE_OFFSET + 0x34)
#define CPSW_ALE_TBLW1			(CPSW_ALE_OFFSET + 0x38)
#define CPSW_ALE_TBLW0			(CPSW_ALE_OFFSET + 0x3C)
#define CPSW_ALE_PORTCTL(p)		(CPSW_ALE_OFFSET + 0x40 + ((p) * 0x04))

#define CPSW_SL_OFFSET			0x0D80
#define CPSW_SL_MACCONTROL(p)		(CPSW_SL_OFFSET + (0x40 * (p)) + 0x04)
#define CPSW_SL_SOFT_RESET(p)		(CPSW_SL_OFFSET + (0x40 * (p)) + 0x0C)
#define CPSW_SL_RX_MAXLEN(p)		(CPSW_SL_OFFSET + (0x40 * (p)) + 0x10)
#define CPSW_SL_RX_PRI_MAP(p)		(CPSW_SL_OFFSET + (0x40 * (p)) + 0x24)

#define MDIO_OFFSET			0x1000
#define MDIOCONTROL			(MDIO_OFFSET + 0x04)
#define MDIOUSERACCESS0			(MDIO_OFFSET + 0x80)
#define MDIOUSERPHYSEL0			(MDIO_OFFSET + 0x84)

#define CPSW_WR_OFFSET			0x1200
#define CPSW_WR_SOFT_RESET		(CPSW_WR_OFFSET + 0x04)
#define CPSW_WR_CONTROL			(CPSW_WR_OFFSET + 0x08)
#define CPSW_WR_INT_CONTROL		(CPSW_WR_OFFSET + 0x0c)
#define CPSW_WR_C_RX_THRESH_EN(p)	(CPSW_WR_OFFSET + (0x10 * (p)) + 0x10)
#define CPSW_WR_C_RX_EN(p)		(CPSW_WR_OFFSET + (0x10 * (p)) + 0x14)
#define CPSW_WR_C_TX_EN(p)		(CPSW_WR_OFFSET + (0x10 * (p)) + 0x18)
#define CPSW_WR_C_MISC_EN(p)		(CPSW_WR_OFFSET + (0x10 * (p)) + 0x1C)
#define CPSW_WR_C_RX_THRESH_STAT(p)	(CPSW_WR_OFFSET + (0x10 * (p)) + 0x40)
#define CPSW_WR_C_RX_STAT(p)		(CPSW_WR_OFFSET + (0x10 * (p)) + 0x44)
#define CPSW_WR_C_TX_STAT(p)		(CPSW_WR_OFFSET + (0x10 * (p)) + 0x48)
#define CPSW_WR_C_MISC_STAT(p)		(CPSW_WR_OFFSET + (0x10 * (p)) + 0x4C)

#define CPSW_CPPI_RAM_OFFSET		0x2000

#define __BIT32(x) ((uint32_t)__BIT(x))
#define __BITS32(x, y) ((uint32_t)__BITS((x), (y)))

/* flags for desciptor word 3 */
#define CPDMA_BD_SOP		__BIT32(31)
#define CPDMA_BD_EOP		__BIT32(30)
#define CPDMA_BD_OWNER		__BIT32(29)
#define CPDMA_BD_EOQ		__BIT32(28)
#define CPDMA_BD_TDOWNCMPLT	__BIT32(27)
#define CPDMA_BD_PASSCRC	__BIT32(26)
#define CPDMA_BD_PKT_ERR_MASK	__BITS32(21,20)

struct cpsw_cpdma_bd {
	uint32_t word[4];
};

/* Interrupt offsets */
#define CPSW_INTROFF_RXTH	0
#define CPSW_INTROFF_RX		1
#define CPSW_INTROFF_TX		2
#define CPSW_INTROFF_MISC	3

#endif /*_IF_CPSWREG_H */
