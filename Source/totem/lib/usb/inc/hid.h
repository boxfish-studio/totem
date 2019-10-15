/***************************************************************************//**
 * @file hidkbd.h
 * @brief USB Human Interface Devices (HID) class keyboard driver.
 * @version 5.1.1
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
#ifndef HID_H_
#define HID_H_

#include <stdint.h>

#include "em_usb.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*HID_SetReportFunc_t)( uint8_t report );

/** HID driver initialization structure.
 *  This data structure contains configuration options that the driver
 *  needs. The structure must be passed to @ref HID_Init() when initializing
 *  the driver.
 */
typedef struct
{
  void                  *hidDescriptor; /**< Pointer to the HID class descriptor in the user application. */
  HID_SetReportFunc_t  	setReportFunc;  /**< Callback function pointer for HID output reports, may be NULL when no callback is needed. */
} HID_Init_t;

void HID_Init( HID_Init_t *init );
int  HID_SetupCmd( const USB_Setup_TypeDef *setup );
void HID_StateChangeEvent( USBD_State_TypeDef oldState,
                              USBD_State_TypeDef newState );

#ifdef __cplusplus
}
#endif

/** @} (end group HidKeyboard) */
/** @} (end group Drivers) */

#endif /* __SILICON_LABS_HIDKBD_H__ */
