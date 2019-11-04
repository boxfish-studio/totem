/***************************************************************************//**
 * @file usbconfig.h
 * @brief USB protocol stack library, application supplied configuration options.
 * @version 5.1.2
 *******************************************************************************
 * @section License
 * <b>Copyright 2015 Silicon Labs, Inc. http://www.silabs.com</b>
 *******************************************************************************
 *
 * This file is licensed under the Silabs License Agreement. See the file
 * "Silabs_License_Agreement.txt" for details. Before using this software for
 * any purpose, you must agree to the terms of that agreement.
 *
 ******************************************************************************/

#ifndef __USBCONFIG_H
#define __USBCONFIG_H

#ifdef __cplusplus
extern "C" {
#endif

//#define BUSPOWERED      /* Uncomment to build buspowered device */

#if defined(BUSPOWERED)
  #define USB_PWRSAVE_MODE (USB_PWRSAVE_MODE_ONSUSPEND | USB_PWRSAVE_MODE_ENTEREM2)
#else
  #define USB_PWRSAVE_MODE  (USB_PWRSAVE_MODE_ONVBUSOFF | USB_PWRSAVE_MODE_ONSUSPEND)
#endif

#define USB_USBC_32kHz_CLK   USB_USBC_32kHz_CLK_LFRCO

#define USB_DEVICE        /* Compile stack for device mode. */

/****************************************************************************
**                                                                         **
** Specify number of endpoints used (in addition to EP0).                  **
**                                                                         **
*****************************************************************************/
#define NUM_EP_USED 1

/****************************************************************************
**                                                                         **
** Specify number of application timers you need.                          **
**                                                                         **
*****************************************************************************/
#define NUM_APP_TIMERS 0

/****************************************************************************
**                                                                         **
** Configure serial port debug output.                                     **
**                                                                         **
*****************************************************************************/
#if !defined(BUSPOWERED)

/* Define a function for transmitting a single char on the serial port. */
#if DEBUG_PRINT
extern int RETARGET_WriteChar(char c);
#define USER_PUTCHAR  ITM_SendChar
#endif

/* Debug USB API functions (illegal input parameters etc.) */
#define DEBUG_USB_API

#endif /* !defined(BUSPOWERED) */

/****************************************************************************
**                                                                         **
** USB HID class device driver definitions.                       **
**                                                                         **
*****************************************************************************/
#define HID_INTR_IN_EP_ADDR  0x81    /* Bit 7 must be set.                 */

#ifdef __cplusplus
}
#endif

#endif /* __USBCONFIG_H */
