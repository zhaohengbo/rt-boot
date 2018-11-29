/*
 *  Atheros AR71xx built-in ethernet mac driver
 *
 *  Copyright (C) 2008-2010 Gabor Juhos <juhosg@openwrt.org>
 *  Copyright (C) 2008 Imre Kaloz <kaloz@openwrt.org>
 *
 *  Based on Atheros' AG7100 driver
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License version 2 as published
 *  by the Free Software Foundation.
 */
#include <kernel/rtthread.h>
#include <kernel/rthw.h>
#include <global/global.h>
#include <soc/net/ag71xx/regs.h>
#include <soc/net/ag71xx/ag71xx.h>
#include <soc/timer.h>

#define BITM(_count)	(BIT(_count) - 1)

#define AR7240_REG_MASK_CTRL		0x00
#define AR7240_MASK_CTRL_REVISION_M	BITM(8)
#define AR7240_MASK_CTRL_VERSION_M	BITM(8)
#define AR7240_MASK_CTRL_VERSION_S	8
#define   AR7240_MASK_CTRL_VERSION_AR7240 0x01
#define   AR7240_MASK_CTRL_VERSION_AR934X 0x02
#define AR7240_MASK_CTRL_SOFT_RESET	BIT(31)

#define AR7240_REG_MAC_ADDR0		0x20
#define AR7240_REG_MAC_ADDR1		0x24

#define AR7240_REG_FLOOD_MASK		0x2c
#define AR7240_FLOOD_MASK_BROAD_TO_CPU	BIT(26)

#define AR7240_REG_GLOBAL_CTRL		0x30
#define AR7240_GLOBAL_CTRL_MTU_M	BITM(11)
#define AR9340_GLOBAL_CTRL_MTU_M	BITM(14)

#define AR7240_REG_VTU			0x0040
#define   AR7240_VTU_OP			BITM(3)
#define   AR7240_VTU_OP_NOOP		0x0
#define   AR7240_VTU_OP_FLUSH		0x1
#define   AR7240_VTU_OP_LOAD		0x2
#define   AR7240_VTU_OP_PURGE		0x3
#define   AR7240_VTU_OP_REMOVE_PORT	0x4
#define   AR7240_VTU_ACTIVE		BIT(3)
#define   AR7240_VTU_FULL		BIT(4)
#define   AR7240_VTU_PORT		BITS(8, 4)
#define   AR7240_VTU_PORT_S		8
#define   AR7240_VTU_VID		BITS(16, 12)
#define   AR7240_VTU_VID_S		16
#define   AR7240_VTU_PRIO		BITS(28, 3)
#define   AR7240_VTU_PRIO_S		28
#define   AR7240_VTU_PRIO_EN		BIT(31)

#define AR7240_REG_VTU_DATA		0x0044
#define   AR7240_VTUDATA_MEMBER		BITS(0, 10)
#define   AR7240_VTUDATA_VALID		BIT(11)

#define AR7240_REG_ATU			0x50
#define AR7240_ATU_FLUSH_ALL		0x1

#define AR7240_REG_AT_CTRL		0x5c
#define AR7240_AT_CTRL_AGE_TIME		BITS(0, 15)
#define AR7240_AT_CTRL_AGE_EN		BIT(17)
#define AR7240_AT_CTRL_LEARN_CHANGE	BIT(18)
#define AR7240_AT_CTRL_RESERVED		BIT(19)
#define AR7240_AT_CTRL_ARP_EN		BIT(20)

#define AR7240_REG_TAG_PRIORITY		0x70

#define AR7240_REG_SERVICE_TAG		0x74
#define AR7240_SERVICE_TAG_M		BITM(16)

#define AR7240_REG_CPU_PORT		0x78
#define AR7240_MIRROR_PORT_S		4
#define AR7240_MIRROR_PORT_M		BITM(4)
#define AR7240_CPU_PORT_EN		BIT(8)

#define AR7240_REG_MIB_FUNCTION0	0x80
#define AR7240_MIB_TIMER_M		BITM(16)
#define AR7240_MIB_AT_HALF_EN		BIT(16)
#define AR7240_MIB_BUSY			BIT(17)
#define AR7240_MIB_FUNC_S		24
#define AR7240_MIB_FUNC_M		BITM(3)
#define AR7240_MIB_FUNC_NO_OP		0x0
#define AR7240_MIB_FUNC_FLUSH		0x1
#define AR7240_MIB_FUNC_CAPTURE		0x3

#define AR7240_REG_MDIO_CTRL		0x98
#define AR7240_MDIO_CTRL_DATA_M		BITM(16)
#define AR7240_MDIO_CTRL_REG_ADDR_S	16
#define AR7240_MDIO_CTRL_PHY_ADDR_S	21
#define AR7240_MDIO_CTRL_CMD_WRITE	0
#define AR7240_MDIO_CTRL_CMD_READ	BIT(27)
#define AR7240_MDIO_CTRL_MASTER_EN	BIT(30)
#define AR7240_MDIO_CTRL_BUSY		BIT(31)

#define AR7240_REG_PORT_BASE(_port)	(0x100 + (_port) * 0x100)

#define AR7240_REG_PORT_STATUS(_port)	(AR7240_REG_PORT_BASE((_port)) + 0x00)
#define AR7240_PORT_STATUS_SPEED_S	0
#define AR7240_PORT_STATUS_SPEED_M	BITM(2)
#define AR7240_PORT_STATUS_SPEED_10	0
#define AR7240_PORT_STATUS_SPEED_100	1
#define AR7240_PORT_STATUS_SPEED_1000	2
#define AR7240_PORT_STATUS_TXMAC	BIT(2)
#define AR7240_PORT_STATUS_RXMAC	BIT(3)
#define AR7240_PORT_STATUS_TXFLOW	BIT(4)
#define AR7240_PORT_STATUS_RXFLOW	BIT(5)
#define AR7240_PORT_STATUS_DUPLEX	BIT(6)
#define AR7240_PORT_STATUS_LINK_UP	BIT(8)
#define AR7240_PORT_STATUS_LINK_AUTO	BIT(9)
#define AR7240_PORT_STATUS_LINK_PAUSE	BIT(10)

#define AR7240_REG_PORT_CTRL(_port)	(AR7240_REG_PORT_BASE((_port)) + 0x04)
#define AR7240_PORT_CTRL_STATE_M	BITM(3)
#define	AR7240_PORT_CTRL_STATE_DISABLED	0
#define AR7240_PORT_CTRL_STATE_BLOCK	1
#define AR7240_PORT_CTRL_STATE_LISTEN	2
#define AR7240_PORT_CTRL_STATE_LEARN	3
#define AR7240_PORT_CTRL_STATE_FORWARD	4
#define AR7240_PORT_CTRL_LEARN_LOCK	BIT(7)
#define AR7240_PORT_CTRL_VLAN_MODE_S	8
#define AR7240_PORT_CTRL_VLAN_MODE_KEEP	0
#define AR7240_PORT_CTRL_VLAN_MODE_STRIP 1
#define AR7240_PORT_CTRL_VLAN_MODE_ADD	2
#define AR7240_PORT_CTRL_VLAN_MODE_DOUBLE_TAG 3
#define AR7240_PORT_CTRL_IGMP_SNOOP	BIT(10)
#define AR7240_PORT_CTRL_HEADER		BIT(11)
#define AR7240_PORT_CTRL_MAC_LOOP	BIT(12)
#define AR7240_PORT_CTRL_SINGLE_VLAN	BIT(13)
#define AR7240_PORT_CTRL_LEARN		BIT(14)
#define AR7240_PORT_CTRL_DOUBLE_TAG	BIT(15)
#define AR7240_PORT_CTRL_MIRROR_TX	BIT(16)
#define AR7240_PORT_CTRL_MIRROR_RX	BIT(17)

#define AR7240_REG_PORT_VLAN(_port)	(AR7240_REG_PORT_BASE((_port)) + 0x08)

#define AR7240_PORT_VLAN_DEFAULT_ID_S	0
#define AR7240_PORT_VLAN_DEST_PORTS_S	16
#define AR7240_PORT_VLAN_MODE_S		30
#define AR7240_PORT_VLAN_MODE_PORT_ONLY	0
#define AR7240_PORT_VLAN_MODE_PORT_FALLBACK	1
#define AR7240_PORT_VLAN_MODE_VLAN_ONLY	2
#define AR7240_PORT_VLAN_MODE_SECURE	3


#define AR7240_REG_STATS_BASE(_port)	(0x20000 + (_port) * 0x100)

#define AR7240_STATS_RXBROAD		0x00
#define AR7240_STATS_RXPAUSE		0x04
#define AR7240_STATS_RXMULTI		0x08
#define AR7240_STATS_RXFCSERR		0x0c
#define AR7240_STATS_RXALIGNERR		0x10
#define AR7240_STATS_RXRUNT		0x14
#define AR7240_STATS_RXFRAGMENT		0x18
#define AR7240_STATS_RX64BYTE		0x1c
#define AR7240_STATS_RX128BYTE		0x20
#define AR7240_STATS_RX256BYTE		0x24
#define AR7240_STATS_RX512BYTE		0x28
#define AR7240_STATS_RX1024BYTE		0x2c
#define AR7240_STATS_RX1518BYTE		0x30
#define AR7240_STATS_RXMAXBYTE		0x34
#define AR7240_STATS_RXTOOLONG		0x38
#define AR7240_STATS_RXGOODBYTE		0x3c
#define AR7240_STATS_RXBADBYTE		0x44
#define AR7240_STATS_RXOVERFLOW		0x4c
#define AR7240_STATS_FILTERED		0x50
#define AR7240_STATS_TXBROAD		0x54
#define AR7240_STATS_TXPAUSE		0x58
#define AR7240_STATS_TXMULTI		0x5c
#define AR7240_STATS_TXUNDERRUN		0x60
#define AR7240_STATS_TX64BYTE		0x64
#define AR7240_STATS_TX128BYTE		0x68
#define AR7240_STATS_TX256BYTE		0x6c
#define AR7240_STATS_TX512BYTE		0x70
#define AR7240_STATS_TX1024BYTE		0x74
#define AR7240_STATS_TX1518BYTE		0x78
#define AR7240_STATS_TXMAXBYTE		0x7c
#define AR7240_STATS_TXOVERSIZE		0x80
#define AR7240_STATS_TXBYTE		0x84
#define AR7240_STATS_TXCOLLISION	0x8c
#define AR7240_STATS_TXABORTCOL		0x90
#define AR7240_STATS_TXMULTICOL		0x94
#define AR7240_STATS_TXSINGLECOL	0x98
#define AR7240_STATS_TXEXCDEFER		0x9c
#define AR7240_STATS_TXDEFER		0xa0
#define AR7240_STATS_TXLATECOL		0xa4

#define AR7240_PORT_CPU		0
#define AR7240_NUM_PORTS	6
#define AR7240_NUM_PHYS		5

#define AR7240_PHY_ID1		0x004d
#define AR7240_PHY_ID2		0xd041

#define AR934X_PHY_ID1		0x004d
#define AR934X_PHY_ID2		0xd042

#define AR7240_MAX_VLANS	16

#define AR934X_REG_OPER_MODE0		0x04
#define   AR934X_OPER_MODE0_MAC_GMII_EN	BIT(6)
#define   AR934X_OPER_MODE0_PHY_MII_EN	BIT(10)

#define AR934X_REG_OPER_MODE1		0x08
#define   AR934X_REG_OPER_MODE1_PHY4_MII_EN	BIT(28)

#define AR934X_REG_FLOOD_MASK		0x2c
#define   AR934X_FLOOD_MASK_MC_DP(_p)	BIT(16 + (_p))
#define   AR934X_FLOOD_MASK_BC_DP(_p)	BIT(25 + (_p))

#define AR934X_REG_QM_CTRL		0x3c
#define   AR934X_QM_CTRL_ARP_EN		BIT(15)

#define AR934X_REG_AT_CTRL		0x5c
#define   AR934X_AT_CTRL_AGE_TIME	BITS(0, 15)
#define   AR934X_AT_CTRL_AGE_EN		BIT(17)
#define   AR934X_AT_CTRL_LEARN_CHANGE	BIT(18)

#define AR934X_MIB_ENABLE		BIT(30)

#define AR934X_REG_PORT_BASE(_port)	(0x100 + (_port) * 0x100)

#define AR934X_REG_PORT_VLAN1(_port)	(AR934X_REG_PORT_BASE((_port)) + 0x08)
#define   AR934X_PORT_VLAN1_DEFAULT_SVID_S		0
#define   AR934X_PORT_VLAN1_FORCE_DEFAULT_VID_EN 	BIT(12)
#define   AR934X_PORT_VLAN1_PORT_TLS_MODE		BIT(13)
#define   AR934X_PORT_VLAN1_PORT_VLAN_PROP_EN		BIT(14)
#define   AR934X_PORT_VLAN1_PORT_CLONE_EN		BIT(15)
#define   AR934X_PORT_VLAN1_DEFAULT_CVID_S		16
#define   AR934X_PORT_VLAN1_FORCE_PORT_VLAN_EN		BIT(28)
#define   AR934X_PORT_VLAN1_ING_PORT_PRI_S		29

#define AR934X_REG_PORT_VLAN2(_port)	(AR934X_REG_PORT_BASE((_port)) + 0x0c)
#define   AR934X_PORT_VLAN2_PORT_VID_MEM_S		16
#define   AR934X_PORT_VLAN2_8021Q_MODE_S		30
#define   AR934X_PORT_VLAN2_8021Q_MODE_PORT_ONLY	0
#define   AR934X_PORT_VLAN2_8021Q_MODE_PORT_FALLBACK	1
#define   AR934X_PORT_VLAN2_8021Q_MODE_VLAN_ONLY	2
#define   AR934X_PORT_VLAN2_8021Q_MODE_SECURE		3

/* Generic MII registers. */
#define MII_BMCR		0x00	/* Basic mode control register */
#define MII_BMSR		0x01	/* Basic mode status register  */
#define MII_PHYSID1		0x02	/* PHYS ID 1                   */
#define MII_PHYSID2		0x03	/* PHYS ID 2                   */
#define MII_ADVERTISE		0x04	/* Advertisement control reg   */
#define MII_LPA			0x05	/* Link partner ability reg    */
#define MII_EXPANSION		0x06	/* Expansion register          */
#define MII_CTRL1000		0x09	/* 1000BASE-T control          */
#define MII_STAT1000		0x0a	/* 1000BASE-T status           */
#define MII_MMD_CTRL		0x0d	/* MMD Access Control Register */
#define MII_MMD_DATA		0x0e	/* MMD Access Data Register */
#define MII_ESTATUS		0x0f	/* Extended Status             */
#define MII_MIPSCR		0x11
#define MII_DCOUNTER		0x12	/* Disconnect counter          */
#define MII_FCSCOUNTER		0x13	/* False carrier counter       */
#define MII_NWAYTEST		0x14	/* N-way auto-neg test reg     */
#define MII_RERRCOUNTER		0x15	/* Receive error counter       */
#define MII_SREVISION		0x16	/* Silicon revision            */
#define MII_RESV1		0x17	/* Reserved...                 */
#define MII_LBRERROR		0x18	/* Lpback, rx, bypass error    */
#define MII_PHYADDR		0x19	/* PHY address                 */
#define MII_RESV2		0x1a	/* Reserved...                 */
#define MII_TPISTATUS		0x1b	/* TPI status for 10mbps       */
#define MII_NCONFIG		0x1c	/* Network interface config    */

/* Basic mode control register. */
#define BMCR_RESV		0x003f	/* Unused...                   */
#define BMCR_SPEED1000		0x0040	/* MSB of Speed (1000)         */
#define BMCR_CTST		0x0080	/* Collision test              */
#define BMCR_FULLDPLX		0x0100	/* Full duplex                 */
#define BMCR_ANRESTART		0x0200	/* Auto negotiation restart    */
#define BMCR_ISOLATE		0x0400	/* Isolate data paths from MII */
#define BMCR_PDOWN		0x0800	/* Enable low power state      */
#define BMCR_ANENABLE		0x1000	/* Enable auto negotiation     */
#define BMCR_SPEED100		0x2000	/* Select 100Mbps              */
#define BMCR_LOOPBACK		0x4000	/* TXD loopback bits           */
#define BMCR_RESET		0x8000	/* Reset to default state      */
#define BMCR_SPEED10		0x0000	/* Select 10Mbps               */

/* Basic mode status register. */
#define BMSR_ERCAP		0x0001	/* Ext-reg capability          */
#define BMSR_JCD		0x0002	/* Jabber detected             */
#define BMSR_LSTATUS		0x0004	/* Link status                 */
#define BMSR_ANEGCAPABLE	0x0008	/* Able to do auto-negotiation */
#define BMSR_RFAULT		0x0010	/* Remote fault detected       */
#define BMSR_ANEGCOMPLETE	0x0020	/* Auto-negotiation complete   */
#define BMSR_RESV		0x00c0	/* Unused...                   */
#define BMSR_ESTATEN		0x0100	/* Extended Status in R15      */
#define BMSR_100HALF2		0x0200	/* Can do 100BASE-T2 HDX       */
#define BMSR_100FULL2		0x0400	/* Can do 100BASE-T2 FDX       */
#define BMSR_10HALF		0x0800	/* Can do 10mbps, half-duplex  */
#define BMSR_10FULL		0x1000	/* Can do 10mbps, full-duplex  */
#define BMSR_100HALF		0x2000	/* Can do 100mbps, half-duplex */
#define BMSR_100FULL		0x4000	/* Can do 100mbps, full-duplex */
#define BMSR_100BASE4		0x8000	/* Can do 100mbps, 4k packets  */

/* Advertisement control register. */
#define ADVERTISE_SLCT		0x001f	/* Selector bits               */
#define ADVERTISE_CSMA		0x0001	/* Only selector supported     */
#define ADVERTISE_10HALF	0x0020	/* Try for 10mbps half-duplex  */
#define ADVERTISE_1000XFULL	0x0020	/* Try for 1000BASE-X full-duplex */
#define ADVERTISE_10FULL	0x0040	/* Try for 10mbps full-duplex  */
#define ADVERTISE_1000XHALF	0x0040	/* Try for 1000BASE-X half-duplex */
#define ADVERTISE_100HALF	0x0080	/* Try for 100mbps half-duplex */
#define ADVERTISE_1000XPAUSE	0x0080	/* Try for 1000BASE-X pause    */
#define ADVERTISE_100FULL	0x0100	/* Try for 100mbps full-duplex */
#define ADVERTISE_1000XPSE_ASYM	0x0100	/* Try for 1000BASE-X asym pause */
#define ADVERTISE_100BASE4	0x0200	/* Try for 100mbps 4k packets  */
#define ADVERTISE_PAUSE_CAP	0x0400	/* Try for pause               */
#define ADVERTISE_PAUSE_ASYM	0x0800	/* Try for asymetric pause     */
#define ADVERTISE_RESV		0x1000	/* Unused...                   */
#define ADVERTISE_RFAULT	0x2000	/* Say we can detect faults    */
#define ADVERTISE_LPACK		0x4000	/* Ack link partners response  */
#define ADVERTISE_NPAGE		0x8000	/* Next page bit               */

#define ADVERTISE_FULL		(ADVERTISE_100FULL | ADVERTISE_10FULL | \
				 ADVERTISE_CSMA)
#define ADVERTISE_ALL		(ADVERTISE_10HALF | ADVERTISE_10FULL | \
				 ADVERTISE_100HALF | ADVERTISE_100FULL)

/* Link partner ability register. */
#define LPA_SLCT		0x001f	/* Same as advertise selector  */
#define LPA_10HALF		0x0020	/* Can do 10mbps half-duplex   */
#define LPA_1000XFULL		0x0020	/* Can do 1000BASE-X full-duplex */
#define LPA_10FULL		0x0040	/* Can do 10mbps full-duplex   */
#define LPA_1000XHALF		0x0040	/* Can do 1000BASE-X half-duplex */
#define LPA_100HALF		0x0080	/* Can do 100mbps half-duplex  */
#define LPA_1000XPAUSE		0x0080	/* Can do 1000BASE-X pause     */
#define LPA_100FULL		0x0100	/* Can do 100mbps full-duplex  */
#define LPA_1000XPAUSE_ASYM	0x0100	/* Can do 1000BASE-X pause asym*/
#define LPA_100BASE4		0x0200	/* Can do 100mbps 4k packets   */
#define LPA_PAUSE_CAP		0x0400	/* Can pause                   */
#define LPA_PAUSE_ASYM		0x0800	/* Can pause asymetrically     */
#define LPA_RESV		0x1000	/* Unused...                   */
#define LPA_RFAULT		0x2000	/* Link partner faulted        */
#define LPA_LPACK		0x4000	/* Link partner acked us       */
#define LPA_NPAGE		0x8000	/* Next page bit               */

#define LPA_DUPLEX		(LPA_10FULL | LPA_100FULL)
#define LPA_100			(LPA_100FULL | LPA_100HALF | LPA_100BASE4)

/* Expansion register for auto-negotiation. */
#define EXPANSION_NWAY		0x0001	/* Can do N-way auto-nego      */
#define EXPANSION_LCWP		0x0002	/* Got new RX page code word   */
#define EXPANSION_ENABLENPAGE	0x0004	/* This enables npage words    */
#define EXPANSION_NPCAPABLE	0x0008	/* Link partner supports npage */
#define EXPANSION_MFAULTS	0x0010	/* Multiple faults detected    */
#define EXPANSION_RESV		0xffe0	/* Unused...                   */

#define ESTATUS_1000_XFULL	0x8000	/* Can do 1000BX Full */
#define ESTATUS_1000_XHALF	0x4000	/* Can do 1000BX Half */
#define ESTATUS_1000_TFULL	0x2000	/* Can do 1000BT Full          */
#define ESTATUS_1000_THALF	0x1000	/* Can do 1000BT Half          */

/* N-way test register. */
#define NWAYTEST_RESV1		0x00ff	/* Unused...                   */
#define NWAYTEST_LOOPBACK	0x0100	/* Enable loopback for N-way   */
#define NWAYTEST_RESV2		0xfe00	/* Unused...                   */

/* 1000BASE-T Control register */
#define ADVERTISE_1000FULL	0x0200  /* Advertise 1000BASE-T full duplex */
#define ADVERTISE_1000HALF	0x0100  /* Advertise 1000BASE-T half duplex */
#define CTL1000_AS_MASTER	0x0800
#define CTL1000_ENABLE_MASTER	0x1000

/* 1000BASE-T Status register */
#define LPA_1000LOCALRXOK	0x2000	/* Link partner local receiver status */
#define LPA_1000REMRXOK		0x1000	/* Link partner remote receiver status */
#define LPA_1000FULL		0x0800	/* Link partner 1000BASE-T full duplex */
#define LPA_1000HALF		0x0400	/* Link partner 1000BASE-T half duplex */

/* Flow control flags */
#define FLOW_CTRL_TX		0x01
#define FLOW_CTRL_RX		0x02

/* MMD Access Control register fields */
#define MII_MMD_CTRL_DEVAD_MASK	0x1f	/* Mask MMD DEVAD*/
#define MII_MMD_CTRL_ADDR	0x0000	/* Address */
#define MII_MMD_CTRL_NOINCR	0x4000	/* no post increment */
#define MII_MMD_CTRL_INCR_RDWT	0x8000	/* post increment on reads & writes */
#define MII_MMD_CTRL_INCR_ON_WT	0xC000	/* post increment on writes only */

static inline rt_uint16_t mk_phy_addr(rt_uint32_t reg)
{
	return 0x17 & ((reg >> 4) | 0x10);
}

static inline rt_uint16_t mk_phy_reg(rt_uint32_t reg)
{
	return (reg << 1) & 0x1e;
}

static inline rt_uint16_t mk_high_addr(rt_uint32_t reg)
{
	return (reg >> 7) & 0x1ff;
}

static rt_uint32_t ar7240sw_reg_read(struct ag71xx_mdio *am, rt_uint32_t reg)
{
	rt_uint16_t phy_addr;
	rt_uint16_t phy_reg;
	rt_uint32_t hi, lo;

	reg = (reg & 0xfffffffc) >> 2;
	phy_addr = mk_phy_addr(reg);
	phy_reg = mk_phy_reg(reg);

	am->mdio_write(am, 0x1f, 0x10, mk_high_addr(reg));
	lo = (rt_uint32_t) am->mdio_read(am, phy_addr, phy_reg);
	hi = (rt_uint32_t) am->mdio_read(am, phy_addr, phy_reg + 1);

	return (hi << 16) | lo;
}

static void ar7240sw_reg_write(struct ag71xx_mdio *am, rt_uint32_t reg, rt_uint32_t val)
{
	rt_uint16_t phy_addr;
	rt_uint16_t phy_reg;

	reg = (reg & 0xfffffffc) >> 2;
	phy_addr = mk_phy_addr(reg);
	phy_reg = mk_phy_reg(reg);

	am->mdio_write(am, 0x1f, 0x10, mk_high_addr(reg));
	
	/*
	 * The switch on AR933x has some special register behavior, which
	 * expects particular write order of their nibbles:
	 *   0x40 ..... MSB first, LSB second
	 *   0x50 ..... MSB first, LSB second
	 *   0x98 ..... LSB first, MSB second
	 *   others ... don't care
	 */
	if (reg == 0x26)//I think AR934X has the same requirment
	{
		am->mdio_write(am, phy_addr, phy_reg, (val & 0xffff));
		am->mdio_write(am, phy_addr, phy_reg + 1, (val >> 16));
	}
	else
	{
		am->mdio_write(am, phy_addr, phy_reg + 1, (val >> 16));
		am->mdio_write(am, phy_addr, phy_reg, (val & 0xffff));
	}
}

static rt_uint32_t ar7240sw_reg_rmw(struct ag71xx_mdio *am, rt_uint32_t reg, rt_uint32_t mask, rt_uint32_t val)
{
	rt_uint32_t t;

	t = ar7240sw_reg_read(am, reg);
	t &= ~mask;
	t |= val;
	ar7240sw_reg_write(am, reg, t);

	return t;
}

static void ar7240sw_reg_set(struct ag71xx_mdio *am, rt_uint32_t reg, rt_uint32_t val)
{
	rt_uint32_t t;

	t = ar7240sw_reg_read(am, reg);
	t |= val;
	ar7240sw_reg_write(am, reg, t);
}

static rt_int32_t ar7240sw_reg_wait(struct ag71xx_mdio *am, rt_uint32_t reg, rt_uint32_t mask, rt_uint32_t val,
			       rt_uint32_t timeout)
{
	int i;

	for (i = 0; i < timeout; i++) {
		rt_uint32_t t;

		t = ar7240sw_reg_read(am, reg);
		if ((t & mask) == val)
			return 0;

		udelay(1500);
	}

	return -1;
}

rt_uint16_t ar7240sw_phy_read(struct ag71xx_mdio *am, rt_uint32_t phy_addr,
		      rt_uint32_t reg_addr)
{
	rt_uint32_t t, val = 0xffff;
	rt_int32_t err;

	if (phy_addr >= AR7240_NUM_PHYS)
		return 0xffff;

	t = (reg_addr << AR7240_MDIO_CTRL_REG_ADDR_S) |
	    (phy_addr << AR7240_MDIO_CTRL_PHY_ADDR_S) |
	    AR7240_MDIO_CTRL_MASTER_EN |
	    AR7240_MDIO_CTRL_BUSY |
	    AR7240_MDIO_CTRL_CMD_READ;

	ar7240sw_reg_write(am, AR7240_REG_MDIO_CTRL, t);
	err = ar7240sw_reg_wait(am, AR7240_REG_MDIO_CTRL,
				  AR7240_MDIO_CTRL_BUSY, 0, 5);
	if (!err)
		val = ar7240sw_reg_read(am, AR7240_REG_MDIO_CTRL);

	return val & AR7240_MDIO_CTRL_DATA_M;
}

rt_int32_t ar7240sw_phy_write(struct ag71xx_mdio *am, rt_uint32_t phy_addr,
		       rt_uint32_t reg_addr, rt_uint16_t reg_val)
{
	rt_uint32_t t;
	rt_int32_t ret;

	if (phy_addr >= AR7240_NUM_PHYS)
		return -1;

	t = (phy_addr << AR7240_MDIO_CTRL_PHY_ADDR_S) |
	    (reg_addr << AR7240_MDIO_CTRL_REG_ADDR_S) |
	    AR7240_MDIO_CTRL_MASTER_EN |
	    AR7240_MDIO_CTRL_BUSY |
	    AR7240_MDIO_CTRL_CMD_WRITE |
	    reg_val;

	ar7240sw_reg_write(am, AR7240_REG_MDIO_CTRL, t);
	ret = ar7240sw_reg_wait(am, AR7240_REG_MDIO_CTRL,
				  AR7240_MDIO_CTRL_BUSY, 0, 5);

	return ret;
}

static rt_int32_t ar7240sw_setup_wan(struct ag71xx_mdio *am)
{
	rt_int32_t phymax;
	if(am->pdata->is_ar934x)
		phymax = 5;
	else
		phymax = 4;
	/* Configure switch port 4 (GMAC0) */
	
    ar7240sw_reg_write(am, AR7240_REG_PORT_STATUS(phymax), AR7240_PORT_STATUS_LINK_AUTO);
	/* Disable Atheros header */
	ar7240sw_reg_write(am, phymax * 0x100 + 4, 0x4004);
	
        return ar7240sw_phy_write(am, phymax - 1, MII_BMCR, 0x9000);
}

static rt_int32_t ar7240sw_setup_lan(struct ag71xx_mdio *am)
{
	rt_int32_t i, ret, phymax;
	rt_uint32_t reg;
	
	if(am->pdata->is_ar934x)
		phymax = 5;
	else
		phymax = 4;

	/* Reset the switch */
	ar7240sw_reg_rmw(am,0,0,BIT(31));

	do {
		reg = ar7240sw_reg_read(am, 0);
	} while (reg & BIT(31));

	/* Configure switch ports 0...3 (GMAC1) */
	for (i = 0; i < phymax; i++) {
		ret = ar7240sw_phy_write(am, i, MII_BMCR, 0x9000);
		if (ret)
			return ret;
#define ATHR_DEBUG_PORT_ADDRESS          29
#define ATHR_DEBUG_PORT_DATA             30
        /* extend the cable length */
        ar7240sw_phy_write(am,i, ATHR_DEBUG_PORT_ADDRESS, 0x14);
        ar7240sw_phy_write(am,i, ATHR_DEBUG_PORT_DATA, 0xf52);

       /* Force Class A setting phys */
        ar7240sw_phy_write(am,i, ATHR_DEBUG_PORT_ADDRESS, 4);
        ar7240sw_phy_write(am,i, ATHR_DEBUG_PORT_DATA, 0xebbb);
        ar7240sw_phy_write(am,i, ATHR_DEBUG_PORT_ADDRESS, 5);
        ar7240sw_phy_write(am,i, ATHR_DEBUG_PORT_DATA, 0x2c47);

        /* fine-tune PHYs */
        ar7240sw_phy_write(am,i, ATHR_DEBUG_PORT_ADDRESS, 0x3c);
        ar7240sw_phy_write(am,i, ATHR_DEBUG_PORT_DATA, 0x1c1);
        ar7240sw_phy_write(am,i, ATHR_DEBUG_PORT_ADDRESS, 0x37);
        ar7240sw_phy_write(am,i, ATHR_DEBUG_PORT_DATA, 0xd600);

        /* turn off power saving */
        ar7240sw_phy_write(am,i, ATHR_DEBUG_PORT_ADDRESS, 41);
        ar7240sw_phy_write(am,i, ATHR_DEBUG_PORT_DATA, 0);
	}

	/* Enable CPU port */
	//ar7240sw_reg_write(am, AR7240_REG_CPU_PORT, AR7240_CPU_PORT_EN);
	ar7240sw_reg_write(am, 0x4, 0x40);
	
        for (i = 0; i < phymax; i++) {
		
		if(i == AR7240_PORT_CPU)
			ar7240sw_reg_write(am, AR7240_REG_PORT_STATUS(AR7240_PORT_CPU),
				   AR7240_PORT_STATUS_SPEED_1000 |
				   AR7240_PORT_STATUS_TXMAC |
				   AR7240_PORT_STATUS_RXMAC |
				   AR7240_PORT_STATUS_DUPLEX);
		else
			ar7240sw_reg_write(am, AR7240_REG_PORT_STATUS(i), AR7240_PORT_STATUS_LINK_AUTO);
		/* Disable Atheros header */
		ar7240sw_reg_write(am, AR7240_REG_PORT_CTRL(i), 0x4004);
	}

	/* Tag priority mapping */
	ar7240sw_reg_write(am, AR7240_REG_TAG_PRIORITY, 0xfa50);

	if(am->pdata->is_ar934x)
	{
		/* Enable aging, MAC replacing */
		ar7240sw_reg_write(am, AR934X_REG_AT_CTRL,
			0x2b /* 5 min age time */ |
			AR934X_AT_CTRL_AGE_EN |
			AR934X_AT_CTRL_LEARN_CHANGE);
		/* Enable ARP frame acknowledge */
		//ar7240sw_reg_set(am, AR934X_REG_QM_CTRL, AR934X_QM_CTRL_ARP_EN);
		ar7240sw_reg_write(am, 0x38, 0xc000050e);
		ar7240sw_reg_set(am,0x5C,0x100000);
		
		/* Enable Broadcast/Multicast frames transmitted to the CPU */
		ar7240sw_reg_set(am, AR934X_REG_FLOOD_MASK,
				 AR934X_FLOOD_MASK_BC_DP(0) | AR934X_FLOOD_MASK_BC_DP(1) |
				 AR934X_FLOOD_MASK_MC_DP(0) | AR934X_FLOOD_MASK_MC_DP(1));

		/* setup MTU */
		ar7240sw_reg_set(am, AR7240_REG_GLOBAL_CTRL,0x5e8);

		/* Enable MIB counters */
        ar7240sw_reg_set(am, AR7240_REG_MIB_FUNCTION0, AR934X_MIB_ENABLE);
	}
	else
	{
		/* Enable ARP frame acknowledge, aging, MAC replacing */
		ar7240sw_reg_write(am, AR7240_REG_AT_CTRL,
			AR7240_AT_CTRL_RESERVED |
			0x2b /* 5 min age time */ |
			AR7240_AT_CTRL_AGE_EN |
			AR7240_AT_CTRL_ARP_EN |
			AR7240_AT_CTRL_LEARN_CHANGE);
		
		/* Enable Broadcast frames transmitted to the CPU */
		ar7240sw_reg_set(am, AR7240_REG_FLOOD_MASK,
				 AR7240_FLOOD_MASK_BROAD_TO_CPU);

		/* setup MTU */
		ar7240sw_reg_rmw(am, AR7240_REG_GLOBAL_CTRL,
				 AR7240_GLOBAL_CTRL_MTU_M,
				 AR7240_GLOBAL_CTRL_MTU_M);
	}

	return 0;
}

static rt_int32_t ar7240sw_phy_port_reset(struct ag71xx_mdio *am, rt_int32_t port)
{
	rt_int32_t ret;

	ret = ar7240sw_phy_write(am, port, MII_ADVERTISE,
				ADVERTISE_ALL | ADVERTISE_PAUSE_CAP |
				ADVERTISE_PAUSE_ASYM);
	if (ret)
		return ret;

	ar7240sw_phy_write(am, port, MII_BMCR,
				 BMCR_ANENABLE | BMCR_RESET);

	do {
		ret = ar7240sw_phy_read(am, port, MII_BMCR);
		if (ret < 0)
			return ret;
		mdelay(10);
	} while (ret & BMCR_RESET);

	return 0;
}

static rt_int32_t ar7240sw_phy_setup(struct ag71xx_mdio *am)
{
	rt_int32_t i, ret, phymax;
	
	if(am->pdata->is_ar934x)
		phymax = 5;
	else
		phymax = 4;
	
	if (am->pdata->is_wan) {
		ret = ar7240sw_phy_port_reset(am, phymax);
		if (ret)
			return ret;

		/* Read out link status */
		ret = ar7240sw_phy_read(am, phymax, MII_MIPSCR);
		if (ret < 0)
			return ret;
	}
	else
	{
		if(!am->pdata->switch_only)
			phymax++;
		/* Switch ports */
		for (i = 0; i < phymax; i++) {
			ret = ar7240sw_phy_port_reset(am, i);
			if (ret)
				return ret;
		}
	
		for (i = 0; i < phymax; i++) {
			/* Read out link status */
			ret = ar7240sw_phy_read(am, i, MII_MIPSCR);
			if (ret < 0)
				return ret;
		}
	}
	
	return 0;
}

static rt_int32_t ar7240sw_phy_init(struct ag71xx_mdio *am)
{
	if(am->pdata->switch_only)
	{
		ar7240sw_setup_lan(am);
		ar7240sw_setup_wan(am);
	}
	else if(am->pdata->is_wan)
		ar7240sw_setup_wan(am);		
	else
		ar7240sw_setup_lan(am);
	
	return ar7240sw_phy_setup(am);
}

void ar7240sw_phy_struct_init(struct ag71xx_phy *ag_phy,struct ag71xx_switch *ag_sw)
{
	ag_phy->phy_init = ar7240sw_phy_init;
	ag_phy->phy_read = ar7240sw_phy_read;
	ag_phy->phy_write = ar7240sw_phy_write;
	
	ag_sw->switch_reg_read = ar7240sw_reg_read;
	ag_sw->switch_reg_write = ar7240sw_reg_write;
}
