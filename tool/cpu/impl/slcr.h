// -*-Mode: C;-*-
/*!@file     slcr.h
*
* @brief     ARM System Level Control Registers (SLCR) implementation
*
* @author    R. Claus -- REG/DRD - (claus@slac.stanford.edu)
*
* @date      March 2, 2013 -- Created
*
* @verbatim                    Copyright 2013
*                                     by
*                        The Board of Trustees of the
*                      Leland Stanford Junior University.
*                             All rights reserved.
* @endverbatim
*/

#define SLCR_BASE_ADDRESS               0xf8000000 // System Level Control Registers

#define SLCR_LOCK_KEY                       0x767B // The keys to the palace
#define SLCR_UNLOCK_KEY                     0xDF0D // The keys to the palace

#define SLCR_LOCK                             0x4 // SLCR lock register
#define SLCR_UNLOCK                           0x8 // SCLR unlock register
#define SLCR_APER_CLK_CTRL_OFFSET           0x12C // AMBA Peripheral Clk Control
#define SLCR_USB0_CLK_CTRL_OFFSET           0x130 // USB 0 ULPI Clock Control
#define SLCR_USB1_CLK_CTRL_OFFSET           0x134 // USB 1 ULPI Clock Control
#define SLCR_EMAC0_RCLK_CTRL_OFFSET         0x138 // EMAC0 RX Clock Control
#define SLCR_EMAC1_RCLK_CTRL_OFFSET         0x13C // EMAC1 RX Clock Control
#define SLCR_EMAC0_CLK_CTRL_OFFSET          0x140 // EMAC0 Reference Clk Control
#define SLCR_EMAC1_CLK_CTRL_OFFSET          0x144 // EMAC1 Reference Clk Control
#define SLCR_SMC_CLK_CTRL_OFFSET            0x148 // SMC Reference Clock Control
#define SLCR_QSPI_CLK_CTRL_OFFSET           0x14C // QSPI Reference Clock Control
#define SLCR_SDIO_CLK_CTRL_OFFSET           0x150 // SDIO Reference Clock Control
#define SLCR_UART_CLK_CTRL_OFFSET           0x154 // UART Reference Clock Control
#define SLCR_SPI_CLK_CTRL_OFFSET            0x158 // SPI Reference Clock Control
#define SLCR_CAN_CLK_CTRL_OFFSET            0x15C // CAN Reference Clock Control
#define SLCR_PSS_RST_CTRL_OFFSET            0x200 // PSS Software Reset Control
#define SLCR_DDR_RST_CTRL_OFFSET            0x204 // DDR Software Reset Control
#define SLCR_AMBA_RST_CTRL_OFFSET           0x208 // AMBA Software Reset Control
#define SLCR_DMAC_RST_CTRL_OFFSET           0x20C // DMAC Software Reset Control
#define SLCR_USB_RST_CTRL_OFFSET            0x210 // USB Software Reset Control
#define SLCR_EMAC_RST_CTRL_OFFSET           0x214 // EMAC Software Reset Control
#define SLCR_SDIO_RST_CTRL_OFFSET           0x218 // SDIO Software Reset Control
#define SLCR_SPI_RST_CTRL_OFFSET            0x21C // SPI Software Reset Control
#define SLCR_CAN_RST_CTRL_OFFSET            0x220 // CAN Software Reset Control
#define SLCR_I2C_RST_CTRL_OFFSET            0x224 // I2C Software Reset Control
#define SLCR_UART_RST_CTRL_OFFSET           0x228 // UART Software Reset Control
#define SLCR_GPIO_RST_CTRL_OFFSET           0x22C // GPIO Software Reset Control
#define SLCR_QSPI_RST_CTRL_OFFSET           0x230 // QSpI Software Reset Control
#define SLCR_SMC_RST_CTRL_OFFSET            0x234 // SMC Software Reset Control
#define SLCR_OCM_RST_CTRL_OFFSET            0x238 // OCM Software Reset Control
#define SLCR_DEVC_RST_CTRL_OFFSET           0x23C // Dev Cfg SW Reset Control
#define SLCR_FPGA_RST_CTRL_OFFSET           0x240 // FPGA Software Reset Control
#define SLCR_A9_CPU_RST_CTRL                0x244 // CPU Software Reset Control
#define SLCR_REBOOT_STATUS                  0x258 // PS Reboot Status
#define SLCR_MIO_PIN_00_OFFSET              0x700 // MIO PIN0 control register
#define SLCR_MIO_LOOPBACK_OFFSET            0x804 // Loopback function within MIO
#define SLCR_MIO_MST_TRI0_OFFSET            0x80C // MIO pin Tri-state Enables, 31:0
#define SLCR_MIO_MST_TRI1_OFFSET            0x810 // MIO pin Tri-state Enables, 53:32
#define SLCR_SD0_WP_CD_SEL_OFFSET           0x830 // SDIO 0 WP CD select
#define SLCR_SD1_WP_CD_SEL_OFFSET           0x834 // SDIO 1 WP CD select
#define SLCR_LVL_SHFTR_EN_OFFSET            0x900 // Level Shifters Enable
#define SLCR_OCM_CFG_OFFSET                 0x910 // OCM Address Mapping
#define SLCR_Reserved_OFFSET                0xA1C // Reserved
#define SLCR_GPIOB_CTRL_OFFSET              0xB00 // PS IO Buffer Control
#define SLCR_GPIOB_CFG_CMOS18_OFFSET        0xB04 // MIO GPIOB CMOS 1.8V config
#define SLCR_GPIOB_CFG_CMOS25_OFFSET        0xB08 // MIO GPIOB CMOS 2.5V config
#define SLCR_GPIOB_CFG_CMOS33_OFFSET        0xB0C // MIO GPIOB CMOS 3.3V config
#define SLCR_GPIOB_CFG_HSTL_OFFSET          0xB14 // MIO GPIOB HSTL config
#define SLCR_GPIOB_DRVR_BIAS_CTRL_OFFSET    0xB18 // MIO GPIOB Driver Bias Control
#define SLCR_DDRIOB_ADDR0_OFFSET            0xB40 // DDR IOB Config for A[14:0], CKE and DRST_B
#define SLCR_DDRIOB_ADDR1_OFFSET            0xB44 // DDR IOB Config for BA[2:0], ODT, CS_B, WE_B, RAS_B and CAS_B
#define SLCR_DDRIOB_DATA0_OFFSET            0xB48 // DDR IOB Config for Data 15:0
#define SLCR_DDRIOB_DATA1_OFFSET            0xB4C // DDR IOB Config for Data 31:16
#define SLCR_DDRIOB_DIFF0_OFFSET            0xB50 // DDR IOB Config for DQS 1:0
#define SLCR_DDRIOB_DIFF1_OFFSET            0xB54 // DDR IOB Config for DQS 3:2
#define SLCR_DDRIOB_CLOCK_OFFSET            0xB58 // DDR IOB Config for Clock Output
#define SLCR_DDRIOB_DRIVE_SLEW_ADDR_OFFSET  0xB5C // Drive and Slew controls for Address and Command pins of the DDR Interface
#define SLCR_DDRIOB_DRIVE_SLEW_DATA_OFFSET  0xB60 // Drive and Slew controls for DQ pins of the DDR Interface
#define SLCR_DDRIOB_DRIVE_SLEW_DIFF_OFFSET  0xB64 // Drive and Slew controls for DQS pins of the DDR Interface
#define SLCR_DDRIOB_DRIVE_SLEW_CLOCK_OFFSET 0xB68 // Drive and Slew controls for Clock pins of the DDR Interface
#define SLCR_DDRIOB_DDR_CTRL_OFFSET         0xB6C // DDR IOB Buffer Control
#define SLCR_DDRIOB_DCI_CTRL_OFFSET         0xB70 // DDR IOB DCI Config
#define SLCR_DDRIOB_DCI_STATUS_OFFSET       0xB74 // DDR IO Buffer DCI Status

/* Bit masks for AMBA Peripheral Clock Control register */
#define SLCR_APER_CLK_CTRL_DMA0_MASK   0x00000001 // DMA0 AMBA Clock active
#define SLCR_APER_CLK_CTRL_USB0_MASK   0x00000004 // USB0 AMBA Clock active
#define SLCR_APER_CLK_CTRL_USB1_MASK   0x00000008 // USB1 AMBA Clock active
#define SLCR_APER_CLK_CTRL_EMAC0_MASK  0x00000040 // EMAC0 AMBA Clock active
#define SLCR_APER_CLK_CTRL_EMAC1_MASK  0x00000080 // EMAC1 AMBA Clock active
#define SLCR_APER_CLK_CTRL_SDI0_MASK   0x00000400 // SDIO0 AMBA Clock active
#define SLCR_APER_CLK_CTRL_SDI1_MASK   0x00000800 // SDIO1 AMBA Clock active
#define SLCR_APER_CLK_CTRL_SPI0_MASK   0x00004000 // SPI0 AMBA Clock active
#define SLCR_APER_CLK_CTRL_SPI1_MASK   0x00008000 // SPI1 AMBA Clock active
#define SLCR_APER_CLK_CTRL_CAN0_MASK   0x00010000 // CAN0 AMBA Clock active
#define SLCR_APER_CLK_CTRL_CAN1_MASK   0x00020000 // CAN1 AMBA Clock active
#define SLCR_APER_CLK_CTRL_I2C0_MASK   0x00040000 // I2C0 AMBA Clock active
#define SLCR_APER_CLK_CTRL_I2C1_MASK   0x00080000 // I2C1 AMBA Clock active
#define SLCR_APER_CLK_CTRL_UART0_MASK  0x00100000 // UART0 AMBA Clock active
#define SLCR_APER_CLK_CTRL_UART1_MASK  0x00200000 // UART1 AMBA Clock active
#define SLCR_APER_CLK_CTRL_GPIO_MASK   0x00400000 // GPIO AMBA Clock active
#define SLCR_APER_CLK_CTRL_QSPI_MASK   0x00800000 // QSPI AMBA Clock active
#define SLCR_APER_CLK_CTRL_SMC_MASK    0x01000000 // SMC AMBA Clock active

#define SLCR_MIO_L0_SHIFT 1;
#define SLCR_MIO_L1_SHIFT 2;
#define SLCR_MIO_L2_SHIFT 3;
#define SLCR_MIO_L3_SHIFT 5;

#define SLCR_MIO_LMASK    0x000000FE

#define SLCR_MIO_PIN_XX_TRI_ENABLE 0x00000001

/* The following constants define L0 Mux Peripheral Enables */
#define SLCR_MIO_PIN_EMAC_ENABLE       (0x01 << SLCR_MIO_L0_SHIFT)
#define SLCR_MIO_PIN_QSPI_ENABLE       (0x01 << SLCR_MIO_L0_SHIFT)

/* The following constants define L1 Mux Enables */
#define SLCR_MIO_PIN_USB_ENABLE        (0x01 << SLCR_MIO_L1_SHIFT)
#define SLCR_MIO_PIN_TRACE_PORT_ENABLE (0x01 << SLCR_MIO_L1_SHIFT)

/* The following constants define L2 Mux Peripheral Enables */
#define SLCR_MIO_PIN_SRAM_NOR_ENABLE   (0x01 << SLCR_MIO_L2_SHIFT)
#define SLCR_MIO_PIN_NAND_ENABLE       (0x02 << SLCR_MIO_L2_SHIFT)

/* The following constants define L3 Mux Peripheral Enables */
#define SLCR_MIO_PIN_GPIO_ENABLE       (0x00 << SLCR_MIO_L3_SHIFT)
#define SLCR_MIO_PIN_CAN_ENABLE        (0x01 << SLCR_MIO_L3_SHIFT)
#define SLCR_MIO_PIN_IIC_ENABLE        (0x02 << SLCR_MIO_L3_SHIFT)
#define SLCR_MIO_PIN_WDT_ENABLE        (0x03 << SLCR_MIO_L3_SHIFT)
#define SLCR_MIO_PIN_JTAG_ENABLE       (0x03 << SLCR_MIO_L3_SHIFT)
#define SLCR_MIO_PIN_SDIO_ENABLE       (0x04 << SLCR_MIO_L3_SHIFT)
#define SLCR_MIO_PIN_MDIO0_ENABLE      (0x04 << SLCR_MIO_L3_SHIFT)
#define SLCR_MIO_PIN_MDIO1_ENABLE      (0x05 << SLCR_MIO_L3_SHIFT)
#define SLCR_MIO_PIN_SPI_ENABLE        (0x05 << SLCR_MIO_L3_SHIFT)
#define SLCR_MIO_PIN_TTC_ENABLE        (0x06 << SLCR_MIO_L3_SHIFT)
#define SLCR_MIO_PIN_UART_ENABLE       (0x07 << SLCR_MIO_L3_SHIFT)

/* The following constants define the number of pins associated with each
 * peripheral */
#define SLCR_MIO_NUM_EMAC_PINS          12
#define SLCR_MIO_NUM_USB_PINS           12
#define SLCR_MIO_NUM_TRACE_DATA2_PINS   04
#define SLCR_MIO_NUM_TRACE_DATA4_PINS   06
#define SLCR_MIO_NUM_TRACE_DATA8_PINS   10
#define SLCR_MIO_NUM_TRACE_DATA16_PINS  18
#define SLCR_MIO_NUM_NAND_PINS         (21+1)
#define SLCR_MIO_NUM_SMC_A25_PINS       01
#define SLCR_MIO_NUM_SMC_CS_PINS        01
#define SLCR_MIO_NUM_NAND_CS_PINS       01
#define SLCR_MIO_NUM_SRAM_NOR_PINS      38
#define SLCR_MIO_NUM_QSPI_PINS          05
#define SLCR_MIO_NUM_QSPI_SEL_PINS      01
#define SLCR_MIO_NUM_QSPI_FOC_PINS      01
#define SLCR_MIO_NUM_GPIO_PINS          01
#define SLCR_MIO_NUM_CAN_PINS           02
#define SLCR_MIO_NUM_IIC_PINS           02
#define SLCR_MIO_NUM_JTAG_PINS          04
#define SLCR_MIO_NUM_WDT_PINS           02
#define SLCR_MIO_NUM_MDIO_PINS          02
#define SLCR_MIO_NUM_SDIO_PINS          06
#define SLCR_MIO_NUM_SPI_PINS           06
#define SLCR_MIO_NUM_TTC_PINS           02
#define SLCR_MIO_NUM_UART_PINS          02

/* The following two constants define the indices of the MIO peripherals EMAC0/1
 * in the array mio_periph_name */
#define SLCR_MIO_EMAC0    0
#define SLCR_MIO_EMAC1    1

#define SLCR_MDIO_PIN_0  52
#define SLCR_MIO_MAX_PIN 54


static inline uint32_t TOOL_CPU_SLCR_readReg(uint32_t offset)
{
  return *(volatile uint32_t*)(SLCR_BASE_ADDRESS + offset);
}

static inline void TOOL_CPU_SLCR_writeReg(uint32_t offset, uint32_t value)
{
  *(volatile uint32_t*)(SLCR_BASE_ADDRESS + offset) = value;
}

inline void TOOL_CPU_SLCR_unlock()
{
  TOOL_CPU_SLCR_writeReg(SLCR_UNLOCK, SLCR_UNLOCK_KEY);
}

inline void TOOL_CPU_SLCR_lock()
{
  TOOL_CPU_SLCR_writeReg(SLCR_LOCK, SLCR_LOCK_KEY);
}
