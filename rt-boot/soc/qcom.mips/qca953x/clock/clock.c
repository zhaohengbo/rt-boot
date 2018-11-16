/*
 * The early init code for MIPS
 * Copyright ZhaoXiaowei 2018
 * Github:github.com/zhaohengbo
 */

#include <kernel/rtthread.h>
#include <common/global.h>
#include <soc/qca953x/qca953x_map.h>
#include <soc/qca953x/qca953x_clock.h>
#include <soc/qca953x/qca953x_reset.h>

rt_uint32_t qca953x_get_xtal_hz(void)
{
	if ((qca_soc_reg_read(QCA_RST_BOOTSTRAP_REG) & 
		 QCA_RST_BOOTSTRAP_REF_CLK_MASK) >> QCA_RST_BOOTSTRAP_REF_CLK_SHIFT)
	{
		return VAL_40MHz;
	}
	else
	{
		return VAL_25MHz;
	}
}

static rt_uint32_t qca953x_get_pll(rt_uint32_t ref_clk,
					   rt_uint32_t refdiv,
					   rt_uint32_t nfrac,
					   rt_uint32_t nfracdiv,
					   rt_uint32_t nint,
					   rt_uint32_t outdiv)
{
	rt_uint64_t pll_mul, pll_div;

	pll_mul = ref_clk;
	pll_div = refdiv;

	if (pll_div == 0)
		pll_div = 1;

	if (nfrac > 0) {
		pll_mul = pll_mul * ((nint * nfracdiv) + nfrac);
		pll_div = pll_div * nfracdiv;
	} else {
		pll_mul = pll_mul * nint;
	}

	pll_div = pll_div << outdiv;

	return (rt_uint32_t)(pll_mul / pll_div);
}

void qca953x_clock_info_init(void)
{
	rt_uint32_t qca_ahb_clk, qca_cpu_clk, qca_ddr_clk, qca_ref_clk;
	rt_uint32_t nint, outdiv, refdiv;
	rt_uint32_t nfrac, nfracdiv;
	rt_uint32_t reg_val, temp;

	rt_uint32_t cpu_pll, ddr_pll;

	qca_ref_clk = qca953x_get_xtal_hz();

	/*
	 * Main AR934x/QCA95xx CPU/DDR PLL clock calculation
	 */

	/* CPU PLL */
	reg_val = qca_soc_reg_read(QCA_PLL_SRIF_CPU_DPLL2_REG);

	/* CPU PLL settings from SRIF CPU DPLL2? */
	if (reg_val & QCA_PLL_SRIF_DPLL2_LOCAL_PLL_MASK) {
		outdiv = (reg_val & QCA_PLL_SRIF_DPLL2_OUTDIV_MASK)
				 >> QCA_PLL_SRIF_DPLL2_OUTDIV_SHIFT;

		reg_val = qca_soc_reg_read(QCA_PLL_SRIF_CPU_DPLL1_REG);

		nfrac = (reg_val & QCA_PLL_SRIF_DPLL1_NFRAC_MASK)
				>> QCA_PLL_SRIF_DPLL1_NFRAC_SHIFT;

		nfracdiv = 1 << 18;

		nint = (reg_val & QCA_PLL_SRIF_DPLL1_NINT_MASK)
			   >> QCA_PLL_SRIF_DPLL1_NINT_SHIFT;

		refdiv = (reg_val & QCA_PLL_SRIF_DPLL1_REFDIV_MASK)
				 >> QCA_PLL_SRIF_DPLL1_REFDIV_SHIFT;
	} else {
		reg_val = qca_soc_reg_read(QCA_PLL_CPU_PLL_DITHER_REG);

		if (reg_val & QCA_PLL_CPU_PLL_DITHER_DITHER_EN_MASK) {
			reg_val = qca_soc_reg_read(QCA_PLL_CPU_PLL_CFG_REG);
			nfrac = (reg_val & QCA_PLL_CPU_PLL_CFG_NFRAC_MASK)
					>> QCA_PLL_CPU_PLL_CFG_NFRAC_SHIFT;
		} else {
			/* NFRAC = NFRAC_MIN if DITHER_EN is 0 */
			nfrac = (reg_val & QCA_PLL_CPU_PLL_DITHER_NFRAC_MIN_MASK)
					>> QCA_PLL_CPU_PLL_DITHER_NFRAC_MIN_SHIFT;
		}

		nfracdiv = 1 << 6;

		reg_val = qca_soc_reg_read(QCA_PLL_CPU_PLL_CFG_REG);

		nint = (reg_val & QCA_PLL_CPU_PLL_CFG_NINT_MASK)
			   >> QCA_PLL_CPU_PLL_CFG_NINT_SHIFT;

		refdiv = (reg_val & QCA_PLL_CPU_PLL_CFG_REFDIV_MASK)
				 >> QCA_PLL_CPU_PLL_CFG_REFDIV_SHIFT;

		outdiv = (reg_val & QCA_PLL_CPU_PLL_CFG_OUTDIV_MASK)
				 >> QCA_PLL_CPU_PLL_CFG_OUTDIV_SHIFT;
	}

	/* Final CPU PLL value */
	cpu_pll = qca953x_get_pll(qca_ref_clk, refdiv,
						  nfrac, nfracdiv, nint, outdiv);

	/* DDR PLL */
	reg_val = qca_soc_reg_read(QCA_PLL_SRIF_DDR_DPLL2_REG);

	/* DDR PLL settings from SRIF DDR DPLL2? */
	if (reg_val & QCA_PLL_SRIF_DPLL2_LOCAL_PLL_MASK) {
		outdiv = (reg_val & QCA_PLL_SRIF_DPLL2_OUTDIV_MASK)
				 >> QCA_PLL_SRIF_DPLL2_OUTDIV_SHIFT;

		reg_val = qca_soc_reg_read(QCA_PLL_SRIF_DDR_DPLL1_REG);

		nfrac = (reg_val & QCA_PLL_SRIF_DPLL1_NFRAC_MASK)
				>> QCA_PLL_SRIF_DPLL1_NFRAC_SHIFT;

		nfracdiv = 1 << 18;

		nint = (reg_val & QCA_PLL_SRIF_DPLL1_NINT_MASK)
			   >> QCA_PLL_SRIF_DPLL1_NINT_SHIFT;

		refdiv = (reg_val & QCA_PLL_SRIF_DPLL1_REFDIV_MASK)
				 >> QCA_PLL_SRIF_DPLL1_REFDIV_SHIFT;
	} else {
		reg_val = qca_soc_reg_read(QCA_PLL_DDR_PLL_DITHER_REG);

		if (reg_val & QCA_PLL_DDR_PLL_DITHER_DITHER_EN_MASK) {
			reg_val = qca_soc_reg_read(QCA_PLL_DDR_PLL_CFG_REG);
			nfrac = (reg_val & QCA_PLL_DDR_PLL_CFG_NFRAC_MASK)
					>> QCA_PLL_DDR_PLL_CFG_NFRAC_SHIFT;
		} else {
			/* NFRAC = NFRAC_MIN if DITHER_EN is 0 */
			nfrac = (reg_val & QCA_PLL_DDR_PLL_DITHER_NFRAC_MIN_MASK)
					>> QCA_PLL_DDR_PLL_DITHER_NFRAC_MIN_SHIFT;
		}

		nfracdiv = 1 << 10;

		reg_val = qca_soc_reg_read(QCA_PLL_DDR_PLL_CFG_REG);

		nint = (reg_val & QCA_PLL_DDR_PLL_CFG_NINT_MASK)
			   >> QCA_PLL_DDR_PLL_CFG_NINT_SHIFT;

		refdiv = (reg_val & QCA_PLL_DDR_PLL_CFG_REFDIV_MASK)
				 >> QCA_PLL_DDR_PLL_CFG_REFDIV_SHIFT;

		outdiv = (reg_val & QCA_PLL_DDR_PLL_CFG_OUTDIV_MASK)
				 >> QCA_PLL_DDR_PLL_CFG_OUTDIV_SHIFT;
	}

	/* Final DDR PLL value */
	ddr_pll = qca953x_get_pll(qca_ref_clk, refdiv,
						  nfrac, nfracdiv, nint, outdiv);

	/* CPU clock divider */
	reg_val = qca_soc_reg_read(QCA_PLL_CPU_DDR_CLK_CTRL_REG);

	temp = ((reg_val & QCA_PLL_CPU_DDR_CLK_CTRL_CPU_POST_DIV_MASK)
			>> QCA_PLL_CPU_DDR_CLK_CTRL_CPU_POST_DIV_SHIFT) + 1;

	if (reg_val & QCA_PLL_CPU_DDR_CLK_CTRL_CPU_PLL_BYPASS_MASK) {
		qca_cpu_clk = qca_ref_clk;
	} else if (reg_val & QCA_PLL_CPU_DDR_CLK_CTRL_CPUCLK_FROM_CPUPLL_MASK) {
		qca_cpu_clk = cpu_pll / temp;
	} else {
		qca_cpu_clk = ddr_pll / temp;
	}

	/* DDR clock divider */
	temp = ((reg_val & QCA_PLL_CPU_DDR_CLK_CTRL_DDR_POST_DIV_MASK)
			>> QCA_PLL_CPU_DDR_CLK_CTRL_DDR_POST_DIV_SHIFT) + 1;

	if (reg_val & QCA_PLL_CPU_DDR_CLK_CTRL_DDR_PLL_BYPASS_MASK) {
		qca_ddr_clk = qca_ref_clk;
	} else if (reg_val & QCA_PLL_CPU_DDR_CLK_CTRL_DDRCLK_FROM_DDRPLL_MASK) {
		qca_ddr_clk = ddr_pll / temp;
	} else {
		qca_ddr_clk = cpu_pll / temp;
	}

	/* AHB clock divider */
	temp = ((reg_val & QCA_PLL_CPU_DDR_CLK_CTRL_AHB_POST_DIV_MASK)
			>> QCA_PLL_CPU_DDR_CLK_CTRL_AHB_POST_DIV_SHIFT) + 1;

	if (reg_val & QCA_PLL_CPU_DDR_CLK_CTRL_AHB_PLL_BYPASS_MASK) {
		qca_ahb_clk = qca_ref_clk;
	} else if (reg_val & QCA_PLL_CPU_DDR_CLK_CTRL_AHBCLK_FROM_DDRPLL_MASK) {
		qca_ahb_clk = ddr_pll / temp;
	} else {
		qca_ahb_clk = cpu_pll / temp;
	}

	/* Return values */
	rtboot_data.cpu_clk = qca_cpu_clk;
	rtboot_data.ddr_clk = qca_ddr_clk;
	rtboot_data.ahb_clk = qca_ahb_clk;
	rtboot_data.ref_clk = qca_ref_clk;
	
	rtboot_data.system_frequency = rtboot_data.cpu_clk;
}

void qca953x_clock_init(void)
{
	if(qca953x_get_xtal_hz() == VAL_40MHz)
		qca_soc_reg_write(QCA_PLL_SWITCH_CLK_CTRL_REG,0x530);
	else
		qca_soc_reg_write(QCA_PLL_SWITCH_CLK_CTRL_REG,0x231);
	
	qca953x_clock_info_init();
}
