#ifndef __AG71XX_H__
#define __AG71XX_H__

//#define AG71XX_DEBUG

#ifndef AG71XX_DEBUG
#define ag71xx_error_printf(fmt,args...) rt_kprintf(fmt,##args)
#define ag71xx_debug_printf(fmt,args...)
#else
#define ag71xx_error_printf(fmt,args...) rt_kprintf(fmt,##args)
#define ag71xx_debug_printf(fmt,args...) rt_kprintf(fmt,##args)
#endif

/* Register offsets */
#define AG71XX_REG_MAC_CFG1	0x0000
#define AG71XX_REG_MAC_CFG2	0x0004
#define AG71XX_REG_MAC_IPG	0x0008
#define AG71XX_REG_MAC_HDX	0x000c
#define AG71XX_REG_MAC_MFL	0x0010
#define AG71XX_REG_MII_CFG	0x0020
#define AG71XX_REG_MII_CMD	0x0024
#define AG71XX_REG_MII_ADDR	0x0028
#define AG71XX_REG_MII_CTRL	0x002c
#define AG71XX_REG_MII_STATUS	0x0030
#define AG71XX_REG_MII_IND	0x0034
#define AG71XX_REG_MAC_IFCTL	0x0038
#define AG71XX_REG_MAC_ADDR1	0x0040
#define AG71XX_REG_MAC_ADDR2	0x0044
#define AG71XX_REG_FIFO_CFG0	0x0048
#define AG71XX_REG_FIFO_CFG1	0x004c
#define AG71XX_REG_FIFO_CFG2	0x0050
#define AG71XX_REG_FIFO_CFG3	0x0054
#define AG71XX_REG_FIFO_CFG4	0x0058
#define AG71XX_REG_FIFO_CFG5	0x005c
#define AG71XX_REG_FIFO_RAM0	0x0060
#define AG71XX_REG_FIFO_RAM1	0x0064
#define AG71XX_REG_FIFO_RAM2	0x0068
#define AG71XX_REG_FIFO_RAM3	0x006c
#define AG71XX_REG_FIFO_RAM4	0x0070
#define AG71XX_REG_FIFO_RAM5	0x0074
#define AG71XX_REG_FIFO_RAM6	0x0078
#define AG71XX_REG_FIFO_RAM7	0x007c

#define AG71XX_REG_TX_CTRL	0x0180
#define AG71XX_REG_TX_DESC	0x0184
#define AG71XX_REG_TX_STATUS	0x0188
#define AG71XX_REG_RX_CTRL	0x018c
#define AG71XX_REG_RX_DESC	0x0190
#define AG71XX_REG_RX_STATUS	0x0194
#define AG71XX_REG_INT_ENABLE	0x0198
#define AG71XX_REG_INT_STATUS	0x019c

#define AG71XX_REG_FIFO_DEPTH	0x01a8
#define AG71XX_REG_RX_SM	0x01b0
#define AG71XX_REG_TX_SM	0x01b4

#define MAC_CFG1_TXE		BIT(0)	/* Tx Enable */
#define MAC_CFG1_STX		BIT(1)	/* Synchronize Tx Enable */
#define MAC_CFG1_RXE		BIT(2)	/* Rx Enable */
#define MAC_CFG1_SRX		BIT(3)	/* Synchronize Rx Enable */
#define MAC_CFG1_TFC		BIT(4)	/* Tx Flow Control Enable */
#define MAC_CFG1_RFC		BIT(5)	/* Rx Flow Control Enable */
#define MAC_CFG1_LB		BIT(8)	/* Loopback mode */
#define MAC_CFG1_SR		BIT(31)	/* Soft Reset */

#define MAC_CFG2_FDX		BIT(0)
#define MAC_CFG2_CRC_EN		BIT(1)
#define MAC_CFG2_PAD_CRC_EN	BIT(2)
#define MAC_CFG2_LEN_CHECK	BIT(4)
#define MAC_CFG2_HUGE_FRAME_EN	BIT(5)
#define MAC_CFG2_IF_1000	BIT(9)
#define MAC_CFG2_IF_10_100	BIT(8)

#define FIFO_CFG0_WTM		BIT(0)	/* Watermark Module */
#define FIFO_CFG0_RXS		BIT(1)	/* Rx System Module */
#define FIFO_CFG0_RXF		BIT(2)	/* Rx Fabric Module */
#define FIFO_CFG0_TXS		BIT(3)	/* Tx System Module */
#define FIFO_CFG0_TXF		BIT(4)	/* Tx Fabric Module */
#define FIFO_CFG0_ALL	(FIFO_CFG0_WTM | FIFO_CFG0_RXS | FIFO_CFG0_RXF \
			| FIFO_CFG0_TXS | FIFO_CFG0_TXF)

#define FIFO_CFG0_ENABLE_SHIFT	8

#define FIFO_CFG4_DE		BIT(0)	/* Drop Event */
#define FIFO_CFG4_DV		BIT(1)	/* RX_DV Event */
#define FIFO_CFG4_FC		BIT(2)	/* False Carrier */
#define FIFO_CFG4_CE		BIT(3)	/* Code Error */
#define FIFO_CFG4_CR		BIT(4)	/* CRC error */
#define FIFO_CFG4_LM		BIT(5)	/* Length Mismatch */
#define FIFO_CFG4_LO		BIT(6)	/* Length out of range */
#define FIFO_CFG4_OK		BIT(7)	/* Packet is OK */
#define FIFO_CFG4_MC		BIT(8)	/* Multicast Packet */
#define FIFO_CFG4_BC		BIT(9)	/* Broadcast Packet */
#define FIFO_CFG4_DR		BIT(10)	/* Dribble */
#define FIFO_CFG4_LE		BIT(11)	/* Long Event */
#define FIFO_CFG4_CF		BIT(12)	/* Control Frame */
#define FIFO_CFG4_PF		BIT(13)	/* Pause Frame */
#define FIFO_CFG4_UO		BIT(14)	/* Unsupported Opcode */
#define FIFO_CFG4_VT		BIT(15)	/* VLAN tag detected */
#define FIFO_CFG4_FT		BIT(16)	/* Frame Truncated */
#define FIFO_CFG4_UC		BIT(17)	/* Unicast Packet */

#define FIFO_CFG5_DE		BIT(0)	/* Drop Event */
#define FIFO_CFG5_DV		BIT(1)	/* RX_DV Event */
#define FIFO_CFG5_FC		BIT(2)	/* False Carrier */
#define FIFO_CFG5_CE		BIT(3)	/* Code Error */
#define FIFO_CFG5_LM		BIT(4)	/* Length Mismatch */
#define FIFO_CFG5_LO		BIT(5)	/* Length Out of Range */
#define FIFO_CFG5_OK		BIT(6)	/* Packet is OK */
#define FIFO_CFG5_MC		BIT(7)	/* Multicast Packet */
#define FIFO_CFG5_BC		BIT(8)	/* Broadcast Packet */
#define FIFO_CFG5_DR		BIT(9)	/* Dribble */
#define FIFO_CFG5_CF		BIT(10)	/* Control Frame */
#define FIFO_CFG5_PF		BIT(11)	/* Pause Frame */
#define FIFO_CFG5_UO		BIT(12)	/* Unsupported Opcode */
#define FIFO_CFG5_VT		BIT(13)	/* VLAN tag detected */
#define FIFO_CFG5_LE		BIT(14)	/* Long Event */
#define FIFO_CFG5_FT		BIT(15)	/* Frame Truncated */
#define FIFO_CFG5_16		BIT(16)	/* unknown */
#define FIFO_CFG5_17		BIT(17)	/* unknown */
#define FIFO_CFG5_SF		BIT(18)	/* Short Frame */
#define FIFO_CFG5_BM		BIT(19)	/* Byte Mode */

#define AG71XX_INT_TX_PS	BIT(0)
#define AG71XX_INT_TX_UR	BIT(1)
#define AG71XX_INT_TX_BE	BIT(3)
#define AG71XX_INT_RX_PR	BIT(4)
#define AG71XX_INT_RX_OF	BIT(6)
#define AG71XX_INT_RX_BE	BIT(7)

#define MAC_IFCTL_SPEED		BIT(16)

#define MII_CFG_CLK_DIV_4	0
#define MII_CFG_CLK_DIV_6	2
#define MII_CFG_CLK_DIV_8	3
#define MII_CFG_CLK_DIV_10	4
#define MII_CFG_CLK_DIV_14	5
#define MII_CFG_CLK_DIV_20	6
#define MII_CFG_CLK_DIV_28	7
#define MII_CFG_CLK_DIV_34	8
#define MII_CFG_CLK_DIV_42	9
#define MII_CFG_CLK_DIV_50	10
#define MII_CFG_CLK_DIV_58	11
#define MII_CFG_CLK_DIV_66	12
#define MII_CFG_CLK_DIV_74	13
#define MII_CFG_CLK_DIV_82	14
#define MII_CFG_CLK_DIV_98	15
#define MII_CFG_RESET		BIT(31)

#define MII_CMD_WRITE		0x0
#define MII_CMD_READ		0x1
#define MII_ADDR_SHIFT		8
#define MII_IND_BUSY		BIT(0)
#define MII_IND_INVALID		BIT(2)

#define TX_CTRL_TXE		BIT(0)	/* Tx Enable */

#define TX_STATUS_PS		BIT(0)	/* Packet Sent */
#define TX_STATUS_UR		BIT(1)	/* Tx Underrun */
#define TX_STATUS_BE		BIT(3)	/* Bus Error */

#define RX_CTRL_RXE		BIT(0)	/* Rx Enable */

#define RX_STATUS_PR		BIT(0)	/* Packet Received */
#define RX_STATUS_OF		BIT(2)	/* Rx Overflow */
#define RX_STATUS_BE		BIT(3)	/* Bus Error */

#define AG71XX_INT_ERR	(AG71XX_INT_RX_BE | AG71XX_INT_TX_BE)
#define AG71XX_INT_TX	(AG71XX_INT_TX_PS)
#define AG71XX_INT_RX	(AG71XX_INT_RX_PR | AG71XX_INT_RX_OF)

#define AG71XX_INT_POLL	(AG71XX_INT_RX | AG71XX_INT_TX)
#define AG71XX_INT_INIT	(AG71XX_INT_ERR | AG71XX_INT_POLL)

struct ag71xx_platform_data {
	rt_uint32_t		mac_base;
	rt_uint32_t		cfg_base;
	rt_uint32_t		int_number;
	rt_int32_t		speed;
	rt_int32_t		duplex;
	rt_uint32_t		reset_bit;
	rt_uint32_t		reset_addr;
	rt_uint8_t		has_gbit:1;
	rt_uint8_t		is_ar91xx:1;
	rt_uint8_t		is_ar7240:1;
	rt_uint8_t		is_ar724x:1;
	rt_uint8_t		is_ar9330:1;
	rt_uint8_t		is_ar934x:1;
	rt_uint8_t		is_wan:1;
	rt_uint8_t		switch_only:1;
	rt_uint8_t		use_flow_control:1;
	rt_uint32_t		max_frame_len;
	rt_uint32_t		desc_pktlen_mask;
	rt_uint8_t		builtin_switch:1;
	rt_uint64_t		mdio_clock;
	rt_uint64_t		ref_clock;
	rt_uint8_t		mac_addr[6];
};

struct ag71xx_mdio {
	const char		*mdio_name;
	
	rt_int32_t (*mdio_init)(struct ag71xx_mdio *am);
	void (*mdio_dump)(struct ag71xx_mdio *am);
	rt_uint16_t (*mdio_read)(struct ag71xx_mdio *mdio_handle, rt_uint32_t addr, rt_uint32_t reg);
	void (*mdio_write)(struct ag71xx_mdio *mdio_handle, rt_uint32_t addr, rt_uint32_t reg, rt_uint16_t val);
	rt_int32_t (*mdio_reset)(struct ag71xx_mdio *mdio_handle);
	
	struct ag71xx_platform_data *pdata;
};

struct ag71xx_gmac {
	const char		*gmac_name;
	
	void (*gmac_init)(struct ag71xx_gmac *ag);
	void (*gmac_dump)(struct ag71xx_gmac *ag);
	
	struct ag71xx_platform_data *pdata;
};

struct ag71xx_phy {
	const char		*phy_name;
	
	rt_int32_t (*phy_init)(struct ag71xx_mdio *am);
	rt_uint32_t (*phy_get_link_status)(struct ag71xx_mdio *am);
	rt_uint16_t (*phy_read)(struct ag71xx_mdio *am, rt_uint32_t phy_addr, rt_uint32_t reg_addr);
	rt_int32_t (*phy_write)(struct ag71xx_mdio *am, rt_uint32_t phy_addr, rt_uint32_t reg_addr, rt_uint16_t reg_val);
	
	struct ag71xx_mdio *am_bus;
	struct ag71xx_platform_data *pdata;
};

struct ag71xx_switch {
	const char		*switch_name;
	
	rt_uint32_t (*switch_reg_read)(struct ag71xx_mdio *am, rt_uint32_t reg);
	void (*switch_reg_write)(struct ag71xx_mdio *am, rt_uint32_t reg, rt_uint32_t val);
	
	struct ag71xx_mdio *am_bus;
	struct ag71xx_platform_data *pdata;
};

#define CONFIG_TX_DESCR_NUM	8
#define CONFIG_RX_DESCR_NUM	8
#define CONFIG_ETH_BUFSIZE	2048
#define TX_TOTAL_BUFSIZE	(CONFIG_ETH_BUFSIZE * CONFIG_TX_DESCR_NUM)
#define RX_TOTAL_BUFSIZE	(CONFIG_ETH_BUFSIZE * CONFIG_RX_DESCR_NUM)

#define AG71XX_DMADESC_IS_EMPTY			BIT(31)
#define AG71XX_DMADESC_FTPP_OVERRIDE_OFFSET	16
#define AG71XX_DMADESC_PKT_SIZE_OFFSET		0
#define AG71XX_DMADESC_PKT_SIZE_MASK		0xfff

struct ag71xx_dma_desc {
	rt_uint32_t	data_addr;
	rt_uint32_t	config;
	rt_uint32_t	next_desc;
	rt_uint32_t	_pad[5];
};

struct ar7xxx_eth_fifos {
    struct ag71xx_dma_desc	*tx_mac_descrtable;//[CONFIG_TX_DESCR_NUM];
    struct ag71xx_dma_desc	*rx_mac_descrtable;//[CONFIG_RX_DESCR_NUM];
    rt_int8_t		*txbuffs;//[TX_TOTAL_BUFSIZE] __aligned(ARCH_DMA_MINALIGN);
    rt_int8_t		*rxbuffs;//[RX_TOTAL_BUFSIZE] __aligned(ARCH_DMA_MINALIGN);

	rt_uint32_t			tx_currdescnum;
	rt_uint32_t			rx_currdescnum;
};

struct ag71xx_eth {
	const char		*eth_name;
	
	rt_int32_t (*eth_init)(struct ag71xx_eth *ag_eth);
	rt_int32_t (*eth_send)(struct ag71xx_eth *ag_eth, void *packet, rt_int32_t length);
	void (*eth_reg_send_cb)(struct ag71xx_eth *ag_eth, void (*cb)(void *), void *para);
	rt_int32_t (*eth_recv)(struct ag71xx_eth *ag_eth, rt_int32_t flags, rt_uint8_t **packetp);
	void (*eth_reg_recv_cb)(struct ag71xx_eth *ag_eth, void (*cb)(void *), void *para);
	rt_int32_t (*eth_free_pkt)(struct ag71xx_eth *ag_eth, rt_uint8_t *packet, rt_int32_t length);
	rt_int32_t (*eth_start)(struct ag71xx_eth *ag_eth);
	void (*eth_stop)(struct ag71xx_eth *ag_eth);
	
	void (*send_cb)(void *para);
	void *send_cb_para;
	void (*recv_cb)(void *para);
	void *recv_cb_para;
	
	struct ag71xx_mdio *mdio;
	struct ag71xx_gmac *gmac;
	struct ag71xx_phy *phy;
	struct ag71xx_switch *switcher;
	struct ar7xxx_eth_fifos *eth_fifos;
	struct ag71xx_platform_data *pdata;
};

#ifdef AG71XX_DEBUG
struct ag71xx_debug {
	struct ag71xx_mdio *debug_mdio[2];
    struct ag71xx_phy *debug_phy_mdio;
	struct ag71xx_switch *debug_switcher;
	struct ag71xx_gmac *debug_gmac[2];
	struct ag71xx_eth *debug_eth[2];
};
#endif

void ag71xx_mdio_struct_init(struct ag71xx_mdio *am);
void ar7240sw_phy_struct_init(struct ag71xx_phy *ag_phy,struct ag71xx_switch *ag_sw);
void ag71xx_gmac_struct_init(struct ag71xx_gmac *ag_gmac);
void ag71xx_eth_struct_init(struct ag71xx_eth *ag_eth);
#ifdef AG71XX_DEBUG
void ag71xx_debug_init(struct ag71xx_debug *ag_dbg);
#endif

#endif
