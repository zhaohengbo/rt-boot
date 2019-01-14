#ifndef __RALINK_MMAP__
#define __RALINK_MMAP__

#define RALINK_SYSCTL_BASE              0xB0000000
#define RALINK_TIMER_BASE               0xB0000100
#define RALINK_INTCL_BASE               0xB0000200
#define RALINK_MEMCTRL_BASE             0xB0000300
#define RALINK_RBUS_MATRIXCTL_BASE      0xB0000400
#define RALINK_MIPS_CNT_BASE            0xB0000500
#define RALINK_PIO_BASE                 0xB0000600
#define RALINK_I2C_BASE                 0xB0000900
#define RALINK_I2S_BASE                 0xB0000A00
#define RALINK_SPI_BASE                 0xB0000B00
#define RALINK_UART_LITE_BASE           0xB0000C00
#define RALINK_UART_LITE2_BASE          0xB0000D00
#define RALINK_UART_LITE3_BASE          0xB0000E00
#define RALINK_PCM_BASE                 0xB0002000
#define RALINK_GDMA_BASE                0xB0002800
#define RALINK_AES_ENGING_BASE          0xB0004000
#define RALINK_CRYPTO_ENGING_BASE       0xB0004000
#define RALINK_RGCTRL_BASE				0xB0001000
#define RALINK_FRAME_ENGINE_BASE        0xB0100000
#define RALINK_ETH_SW_BASE              0xB0110000
#define RALINK_USB_DEV_BASE             0xB0120000
#define RALINK_MSDC_BASE                0xB0130000
#define RALINK_PCI_BASE                 0xB0140000
#define RALINK_11N_MAC_BASE             0xB0180000
#define RALINK_USB_HOST_BASE            0xB01C0000

#define RALINK_MCNT_CFG                 0xB0000D00
#define RALINK_COMPARE                  0xB0000D04
#define RALINK_COUNT                    0xB0000D08

#define RALINK_SYS_CGF0_REG             0xB0000010

#define RALINK_CPLLCFG0_REG		(RALINK_SYSCTL_BASE+0x54)
#define RALINK_CPLLCFG1_REG		(RALINK_SYSCTL_BASE+0x58)

#define RALINK_IRQMASK_REG              0xB0000270
#define RALINK_IRQEND_REG               0xB0000288
#define RALINK_IRQSTAT_REG              0xB000029C
//Interrupt Controller
#define RALINK_INTCTL_SYSCTL            (1<<0)
#define RALINK_INTCTL_TIMER0            (1<<1)
#define RALINK_INTCTL_WDTIMER           (1<<2)
#define RALINK_INTCTL_ILL_ACCESS        (1<<3)
#define RALINK_INTCTL_PCM               (1<<4)
#define RALINK_INTCTL_UART              (1<<5)
#define RALINK_INTCTL_PIO               (1<<6)
#define RALINK_INTCTL_DMA               (1<<7)
#define RALINK_INTCTL_PC                (1<<9)
#define RALINK_INTCTL_I2S               (1<<10)
#define RALINK_INTCTL_SPI               (1<<11)
#define RALINK_INTCTL_UARTLITE          (1<<12)
#define RALINK_INTCTL_CRYPTO            (1<<13)
#define RALINK_INTCTL_ESW               (1<<17)
#define RALINK_INTCTL_UHST              (1<<18)
#define RALINK_INTCTL_UDEV              (1<<19)
#define RALINK_INTCTL_GLOBAL            (1<<31)

#define RALINK_SYS_RST_REG              0xB0000034
//Reset Control Register
#define RALINK_SYS_RST                  (1<<0)
#define RALINK_TIMER_RST                (1<<8)
#define RALINK_INTC_RST                 (1<<9)
#define RALINK_MC_RST                   (1<<10)
#define RALINK_PCM_RST                  (1<<11)
#define RALINK_UART_RST                 (1<<12)
#define RALINK_PIO_RST                  (1<<13)
#define RALINK_DMA_RST                  (1<<14)
#define RALINK_I2C_RST                  (1<<16)
#define RALINK_I2S_RST                  (1<<17)
#define RALINK_SPI_RST                  (1<<18)
#define RALINK_UARTL_RST                (1<<19)
#define RALINK_FE_RST                   (1<<21)
#define RALINK_UHST_RST                 (1<<22)
#define RALINK_ESW_RST                  (1<<23)
#define RALINK_EPHY_RST                 (1<<24)
#define RALINK_UDEV_RST                 (1<<25)
#define RALINK_PCIE0_RST                (1<<26)
#define RALINK_PCIE1_RST                (1<<27)
#define RALINK_MIPS_CNT_RST             (1<<28)
#define RALINK_CRYPTO_RST               (1<<29)


//Clock Conf Register
#define RALINK_UPHY1_CLK_EN     (1<<22)
#define RALINK_UPHY0_CLK_EN     (1<<25)
#define RALINK_PCIE_CLK_EN    	(1<<26)

/* CPLL related */
#define RALINK_CPLLCFG0_REG	(RALINK_SYSCTL_BASE+0x54)
#define RALINK_CPLLCFG1_REG	(RALINK_SYSCTL_BASE+0x58)
#define CPLL_SW_CONFIG                  (0x1UL << 31)
#define CPLL_MULT_RATIO_SHIFT           16
#define CPLL_MULT_RATIO                 (0x7UL << CPLL_MULT_RATIO_SHIFT)
#define CPLL_DIV_RATIO_SHIFT            10
#define CPLL_DIV_RATIO                  (0x3UL << CPLL_DIV_RATIO_SHIFT)
#define BASE_CLOCK                      40      /* Mhz */

/* Timer related */
#define RALINK_CLKCFG0_REG      0xB000002C
#define RALINK_DYN_CFG0_REG		0xB0000440

#define __REG(x)	(*((volatile unsigned int *)(x)))

#endif