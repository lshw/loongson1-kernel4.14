/*
 * Copyright (c) 2011-2016 Zhang, Keguang <keguang.zhang@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 */

#include <linux/leds.h>
#include <linux/mtd/partitions.h>
#include <linux/sizes.h>

#include <loongson1.h>
#include <dma.h>
#include <nand.h>
#include <platform.h>

struct plat_ls1x_dma ls1x_dma_pdata = {
	.nr_channels	= 3,
};
#include <ls1x_nand.h>
static struct mtd_partition ls1x_nand_partitions[] = {
	/*
	{
		.name        = "kernel",
		.offset      = 0,
		.size        = SZ_16M,
	},
	{
		.name        = "rootfs",
		.offset      = MTDPART_OFS_APPEND,
		.size        = MTDPART_SIZ_FULL,
	},
	*/
	{
		.name	= "kernel",
		.offset	= MTDPART_OFS_APPEND,
		.size	= 14*1024*1024,
	}, {
		.name	= "rootfs",
		.offset	= MTDPART_OFS_APPEND,
		.size	= 100*1024*1024,
	}, {
		.name	= "data",
		.offset	= MTDPART_OFS_APPEND,
		.size	= MTDPART_SIZ_FULL,
	},
};

struct ls1x_nand_platform_data ls1x_nand_parts = {
	.parts		= ls1x_nand_partitions,
	.nr_parts	= ARRAY_SIZE(ls1x_nand_partitions),
	//.hold_cycle	= 0x2,
	//.wait_cycle	= 0xc,
};

static const struct gpio_led ls1x_gpio_leds[] __initconst = {
	{
		.name			= "LED9",
		.default_trigger	= "heartbeat",
		.gpio			= 38,
		.active_low		= 1,
		.default_state		= LEDS_GPIO_DEFSTATE_OFF,
	}, {
		.name			= "LED6",
		.default_trigger	= "nand-disk",
		.gpio			= 39,
		.active_low		= 1,
		.default_state		= LEDS_GPIO_DEFSTATE_OFF,
	},
};

static const struct gpio_led_platform_data ls1x_led_pdata __initconst = {
	.num_leds	= ARRAY_SIZE(ls1x_gpio_leds),
	.leds		= ls1x_gpio_leds,
};

#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE)
#include <linux/spi/flash.h>
static struct mtd_partition ls1x_spi_flash_partitions[] = {
	{
		.name = "pmon(spi)",
		.size = 0x00080000,
		.offset = 0,
//		.mask_flags = MTD_CAP_ROM
	}
};

static struct flash_platform_data ls1x_spi_flash_data = {
	.name = "spi-flash",
	.parts = ls1x_spi_flash_partitions,
	.nr_parts = ARRAY_SIZE(ls1x_spi_flash_partitions),
	.type = "w25x40",
};
#endif

#ifdef CONFIG_SPI_LS1X_SPI0
#include <linux/spi/spi_ls1x.h>
static struct resource ls1x_spi0_resource[] = {
	[0] = {
		.start	= LS1X_SPI0_BASE,
		.end	= LS1X_SPI0_BASE + SZ_16K - 1,
		.flags	= IORESOURCE_MEM,
	},
#if defined(CONFIG_SPI_IRQ_MODE)
	[1] = {
		.start	= LS1X_SPI0_IRQ,
		.end	= LS1X_SPI0_IRQ,
		.flags	= IORESOURCE_IRQ,
	},
#endif
};

extern struct ls1x_spi_platform_data ls1x_spi0_platdata;

struct platform_device ls1x_spi0_pdev = {
	.name		= "spi_ls1x",
	.id 		= 0,
	.num_resources	= ARRAY_SIZE(ls1x_spi0_resource),
	.resource	= ls1x_spi0_resource,
	.dev		= {
		.platform_data	= &ls1x_spi0_platdata,
	},
};
#endif


#ifdef CONFIG_SPI_LS1X_SPI0
#include <irq.h>
#include <linux/spi/spi.h>
#include <linux/spi/spi_ls1x.h>
#if defined(CONFIG_SPI_CS_USED_GPIO)
static int spi0_gpios_cs[] = { 27, 28, 29, 30 };
#endif

struct ls1x_spi_platform_data ls1x_spi0_platdata = {
#if defined(CONFIG_SPI_CS_USED_GPIO)
	.gpio_cs_count = ARRAY_SIZE(spi0_gpios_cs),
	.gpio_cs = spi0_gpios_cs,
#elif defined(CONFIG_SPI_CS)
	.cs_count = SPI0_CS3 + 1,
#endif
};

static struct spi_board_info ls1x_spi0_devices[] = {
#if defined(CONFIG_MTD_M25P80) || defined(CONFIG_MTD_M25P80_MODULE)
	{
		.modalias	= "m25p80",
		.bus_num 		= 0,
		.chip_select	= SPI0_CS0,
		.max_speed_hz	= 60000000,
		.platform_data	= &ls1x_spi_flash_data,
		.mode = SPI_MODE_3,
	},
#endif

#ifdef CONFIG_TOUCHSCREEN_ADS7846
	{
		.modalias = "ads7846",
		.platform_data = &ads_info,
		.bus_num 		= 0,
		.chip_select 	= SPI0_CS1,
		.max_speed_hz 	= 2500000,
		.mode 			= SPI_MODE_1,
		.irq			= LS1X_GPIO_FIRST_IRQ + ADS7846_GPIO_IRQ,
	},
#endif

//==================================================================================
///*
#if defined(CONFIG_ENC28J60)//dstling add 20220623
	{
		.modalias		= "enc28j60",  //enc28j60  驱动路径drivers\net\ethernet\microchip
		.bus_num		= 0, //由SPI0 1 2决定
		.chip_select	= SPI0_CS2,
		.max_speed_hz	= 60000000,
		.mode = SPI_MODE_0,//SPI_MODE_0,
		.irq = LS1X_GPIO_FIRST_IRQ+ENC_IRQ_GPIO,//enc28j60 LS1X_SPI0_ENC28J60_IRQ,irq=116 52 53 不能用！！
	},
#endif //end CONFIG_ENC28J60

#if defined(CONFIG_WIZNET_W5100) //w5500 spi net
#define WIZNET_W5100_SPI_IRQ_GPIO  36  //spi网卡的中断GPIO口
	{
		.modalias		= "w5100",  //w5500  驱动路径drivers\net\ethernet\wiznet
		.bus_num		= 0, //由SPI0 1 2决定
		.chip_select	= SPI0_CS3,//SPI0_CS3,
		.max_speed_hz	= 25000000,
		.mode = SPI_MODE_0,//SPI_MODE_0,
		.irq = LS1X_GPIO_FIRST_IRQ+WIZNET_W5100_SPI_IRQ_GPIO,//w5500 直接使用GPIO口
	},
#endif //end CONFIG_WIZNET_W5100
//*/
//==================================================================================
};
#endif //end CONFIG_SPI_LS1X_SPI0





static struct platform_device *ls1b_platform_devices[] __initdata = {
	&ls1x_uart_pdev,
	&ls1x_cpufreq_pdev,

#ifdef CONFIG_SPI_LS1X_SPI0
	&ls1x_spi0_pdev,
#endif	
	&ls1x_nand_pdev,
	&ls1x_dma_pdev,
	&ls1x_eth0_pdev,
	&ls1x_eth1_pdev,
	&ls1x_ehci_pdev,
	&ls1x_gpio0_pdev,
	&ls1x_gpio1_pdev,
	&ls1x_rtc_pdev,
	&ls1x_wdt_pdev,
};

static int __init ls1b_platform_init(void)
{
	ls1x_serial_set_uartclk(&ls1x_uart_pdev);

#if defined(CONFIG_SPI_LS1X_SPI0)
	spi_register_board_info(ls1x_spi0_devices, ARRAY_SIZE(ls1x_spi0_devices));
#endif

	//ls1x_dma_set_platdata(&ls1x_dma_pdata);
	//ls1x_nand_set_platdata(&ls1x_nand_pdata);

	gpio_led_register_device(-1, &ls1x_led_pdata);

	return platform_add_devices(ls1b_platform_devices,
				   ARRAY_SIZE(ls1b_platform_devices));
}

arch_initcall(ls1b_platform_init);
