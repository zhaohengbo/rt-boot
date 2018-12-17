/*
 * The early init code for MIPS
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <kernel/rtthread.h>
#include <global/global.h>
#include <soc/qca953x/qca953x_map.h>
#include <soc/qca953x/qca953x_reset.h>
#include <soc/timer.h>

/*
 * Performs full chip reset
 */
void qca953x_full_reset(void)
{
	volatile rt_uint32_t i = 1;

	do {
		qca_soc_reg_write(QCA_RST_RESET_REG,
						  QCA_RST_RESET_FULL_CHIP_RST_MASK
						  | QCA_RST_RESET_DDR_RST_MASK);
	} while (i);
}

void qca953x_net_reset(void)
{
	qca_soc_reg_read_set(QCA_RST_RESET_REG,
						  QCA_RST_RESET_GE0_MDIO_RST_MASK
						| QCA_RST_RESET_GE1_MDIO_RST_MASK
						| QCA_RST_RESET_GE1_MAC_RST_MASK
						| QCA_RST_RESET_ETH_SWITCH_ARST_MASK
						| QCA_RST_RESET_GE0_MAC_RST_MASK
						| QCA_RST_RESET_ETH_SWITCH_RST_MASK);
	mdelay(1);
	qca_soc_reg_read_clear(QCA_RST_RESET_REG,
						  QCA_RST_RESET_GE0_MDIO_RST_MASK
						| QCA_RST_RESET_GE1_MDIO_RST_MASK
						| QCA_RST_RESET_GE1_MAC_RST_MASK
						| QCA_RST_RESET_ETH_SWITCH_ARST_MASK
						| QCA_RST_RESET_GE0_MAC_RST_MASK
						| QCA_RST_RESET_ETH_SWITCH_RST_MASK);
}
