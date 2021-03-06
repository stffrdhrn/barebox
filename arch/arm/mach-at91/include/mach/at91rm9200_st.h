/* SPDX-License-Identifier: GPL-2.0-or-later */
/* SPDX-FileCopyrightText: 2005 Ivan Kokshaysky */
/* SPDX-FileCopyrightText: SAN People */

/*
 * arch/arm/mach-at91/include/mach/at91_st.h
 *
 * System Timer (ST) - System peripherals registers.
 * Based on AT91RM9200 datasheet revision E.
 */

#ifndef AT91RM9200_ST_H
#define AT91RM9200_ST_H

#define	AT91RM9200_ST_CR		(0x00)	/* Control Register */
#define 	AT91RM9200_ST_WDRST		(1 << 0)		/* Watchdog Timer Restart */

#define	AT91RM9200_ST_PIMR		(0x04)	/* Period Interval Mode Register */
#define		AT91RM9200_ST_PIV		(0xffff <<  0)		/* Period Interval Value */

#define	AT91RM9200_ST_WDMR		(0x08)	/* Watchdog Mode Register */
#define		AT91RM9200_ST_WDV		(0xffff <<  0)		/* Watchdog Counter Value */
#define		AT91RM9200_ST_RSTEN		(1	<< 16)		/* Reset Enable */
#define		AT91RM9200_ST_EXTEN		(1	<< 17)		/* External Signal Assertion Enable */

#define	AT91RM9200_ST_RTMR		(0x0c)	/* Real-time Mode Register */
#define		AT91RM9200_ST_RTPRES		(0xffff <<  0)		/* Real-time Prescalar Value */

#define	AT91RM9200_ST_SR		(0x10)	/* Status Register */
#define		AT91RM9200_ST_PITS		(1 << 0)		/* Period Interval Timer Status */
#define		AT91RM9200_ST_WDOVF		(1 << 1) 		/* Watchdog Overflow */
#define		AT91RM9200_ST_RTTINC		(1 << 2) 		/* Real-time Timer Increment */
#define		AT91RM9200_ST_ALMS		(1 << 3) 		/* Alarm Status */

#define	AT91RM9200_ST_IER		(0x14)	/* Interrupt Enable Register */
#define	AT91RM9200_ST_IDR		(0x18)	/* Interrupt Disable Register */
#define	AT91RM9200_ST_IMR		(0x1c)	/* Interrupt Mask Register */

#define	AT91RM9200_ST_RTAR		(0x20)	/* Real-time Alarm Register */
#define		AT91RM9200_ST_ALMV		(0xfffff << 0)		/* Alarm Value */

#define	AT91RM9200_ST_CRTR		(0x24)	/* Current Real-time Register */
#define		AT91RM9200_ST_CRTV		(0xfffff << 0)		/* Current Real-Time Value */

#endif
