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

typedef void (*HID_SetReportFunc_t)(uint8_t **report);

typedef struct
{
    uint8_t data[37];        /* Received USB data */
} usbInData_t;

void HID_Init();
int HID_SetupCmd(const USB_Setup_TypeDef *setup);
void HID_StateChangeEvent(USBD_State_TypeDef oldState,
		USBD_State_TypeDef newState);
int HID_TransferCompleteCallback(USB_Status_TypeDef status, uint32_t xferred,
		uint32_t remaining);

#ifdef __cplusplus
}
#endif


#endif /* HID_H_ */
