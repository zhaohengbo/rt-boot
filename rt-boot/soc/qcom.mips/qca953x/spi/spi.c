#include <kernel/rtthread.h>
#include <global/global.h>
#include <soc/qca953x/qca953x_map.h>
#include <soc/qca953x/qca953x_spi.h>

/* Use CS0 by default */
static rt_uint32_t qca953x_cs_mask = QCA_SPI_SHIFT_CNT_CHNL_CS0_MASK;

void qca953x_spi_enable(void)
{
	qca_soc_reg_write(QCA_SPI_FUNC_SEL_REG, 1);
}

void qca953x_spi_disable(void)
{
	qca_soc_reg_write(QCA_SPI_SHIFT_CNT_REG, 0);
	qca_soc_reg_write(QCA_SPI_FUNC_SEL_REG, 0);
}

rt_uint32_t qca953x_shift_in(void)
{
	return qca_soc_reg_read(QCA_SPI_SHIFT_DATAIN_REG);
}

/*
 * Shifts out 'bits_cnt' bits from 'data_out' value
 * If 'terminate' is zero, then CS is not driven high at end of transaction
 */
void qca953x_shift_out(rt_uint32_t data_out, rt_uint32_t bits_cnt, rt_uint32_t terminate)
{
	rt_uint32_t reg_val = 0;

	qca_soc_reg_write(QCA_SPI_SHIFT_CNT_REG, 0);

	/* Data to shift out */
	qca_soc_reg_write(QCA_SPI_SHIFT_DATAOUT_REG, data_out);

	reg_val = reg_val | bits_cnt
					  | qca953x_cs_mask
					  | QCA_SPI_SHIFT_CNT_SHIFT_EN_MASK;

	if (terminate)
		reg_val = reg_val | QCA_SPI_SHIFT_CNT_TERMINATE_MASK;

	/* Enable shifting in/out */
	qca_soc_reg_write(QCA_SPI_SHIFT_CNT_REG, reg_val);
}

void qca953x_change_cs(rt_uint32_t cs_bank)
{
	switch (cs_bank) {
	case 0:
		qca953x_cs_mask = QCA_SPI_SHIFT_CNT_CHNL_CS0_MASK;
		break;
	case 1:
		qca953x_cs_mask = QCA_SPI_SHIFT_CNT_CHNL_CS1_MASK;
		break;
	case 2:
		qca953x_cs_mask = QCA_SPI_SHIFT_CNT_CHNL_CS2_MASK;
		break;
	default:
		qca953x_cs_mask = QCA_SPI_SHIFT_CNT_CHNL_CS0_MASK;
		break;
	}
}