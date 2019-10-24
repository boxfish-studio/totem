/***************************************************************************//**
 * @file hidkbd.c
 * @brief USB Human Interface Devices (HID) class driver.
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

#include "hid.h"
#include "descriptors.h"

static volatile uint8_t rcv_buf[USB_PACKET_SIZE] __attribute__ ((aligned(4)));

static HID_SetReportFunc_t setReportFunc = NULL;

static const USBD_Callbacks_TypeDef callbacks = {
		.usbReset = NULL,
		.usbStateChange = HID_StateChangeEvent,
		.setupCmd = HID_SetupCmd,
		.isSelfPowered = NULL,
		.sofInt = NULL };

static const USBD_Init_TypeDef initStruct = {
		.deviceDescriptor = &USBDESC_deviceDesc,
		.configDescriptor = USBDESC_configDesc,
		.stringDescriptors = USBDESC_strings,
		.numberOfStrings = sizeof(USBDESC_strings) / sizeof(void*),
		.callbacks = &callbacks,
		.bufferingMultiplier = USBDESC_bufferingMultiplier, .reserved = 0 };

static int OutputReportReceived(USB_Status_TypeDef status, uint32_t xferred,
		uint32_t remaining);

/**************************************************************************//**
 * @brief
 *   Callback function called when the data stage of a USB_HID_SET_REPORT
 *   setup command has completed.
 *
 * @param[in] status    Transfer status code.
 * @param[in] xferred   Number of bytes transferred.
 * @param[in] remaining Number of bytes not transferred.
 *
 * @return USB_STATUS_OK.
 *****************************************************************************/
static int OutputReportReceived(USB_Status_TypeDef status, uint32_t xferred,
		uint32_t remaining) {
	(void) remaining;

	if ((status == USB_STATUS_OK) && (setReportFunc != NULL)) {
		setReportFunc((uint8_t**) &rcv_buf, (uint8_t) remaining);
	}

	return USB_STATUS_OK;
}

/** @endcond */

/***************************************************************************//**
 * @brief
 *  Initialize HID driver.
 *
 * @param[in] init
 *  Pointer to a HID_Init_t struct with configuration options.
 ******************************************************************************/
void HID_Init(HID_SetReportFunc_t callbackFunction) {
	USBD_Init(&initStruct);
	setReportFunc = callbackFunction;
}

/**************************************************************************//**
 * @brief
 *   Handle USB setup commands. Implements HID class specific commands.
 *   This function must be called each time the device receive a setup command.
 *
 *
 * @param[in] setup Pointer to the setup packet received.
 *
 * @return USB_STATUS_OK if command accepted,
 *         USB_STATUS_REQ_UNHANDLED when command is unknown. In the latter case
 *         the USB device stack will handle the request.
 *****************************************************************************/
int HID_SetupCmd(const USB_Setup_TypeDef *setup) {
	STATIC_UBUF(hidDesc, USB_HID_DESCSIZE);

	int retVal = USB_STATUS_REQ_UNHANDLED;

	if ((setup->Type == USB_SETUP_TYPE_STANDARD)
			&& (setup->Direction == USB_SETUP_DIR_IN)
			&& (setup->Recipient == USB_SETUP_RECIPIENT_INTERFACE)) {
		/* A HID device must extend the standard GET_DESCRIPTOR command   */
		/* with support for HID descriptors.                              */
		switch (setup->bRequest) {
		case GET_DESCRIPTOR:
			/********************/
			if ((setup->wValue >> 8) == USB_HID_REPORT_DESCRIPTOR) {
				USBD_Write(0, (void*) ReportDescriptor,
						SL_MIN(sizeof(ReportDescriptor), setup->wLength),
						NULL);
				retVal = USB_STATUS_OK;
			} else if ((setup->wValue >> 8) == USB_HID_DESCRIPTOR) {
				/* The HID descriptor might be misaligned ! */
				memcpy(hidDesc,
						&USBDESC_configDesc[USB_CONFIG_DESCSIZE
								+ USB_INTERFACE_DESCSIZE], USB_HID_DESCSIZE);
				USBD_Write(0, hidDesc, SL_MIN(USB_HID_DESCSIZE, setup->wLength),
						NULL);
				retVal = USB_STATUS_OK;
			}
			break;
		}
	}

	else if ((setup->Type == USB_SETUP_TYPE_CLASS)
			&& (setup->Recipient == USB_SETUP_RECIPIENT_INTERFACE)) {
		/* Implement the necessary HID class specific commands.           */
		switch (setup->bRequest) {
		case USB_HID_SET_REPORT:
			/********************/
			USBD_Read(0, &rcv_buf, USB_PACKET_SIZE, OutputReportReceived);
			retVal = USB_STATUS_OK;
			break;
		}
	}

	return retVal;
}

/**************************************************************************//**
 * @brief
 *   Handle USB state change events, this function must be called each time
 *   the USB device state is changed.
 *
 * @param[in] oldState The device state the device has just left.
 * @param[in] newState The new device state.
 *****************************************************************************/
void HID_StateChangeEvent(USBD_State_TypeDef oldState,
		USBD_State_TypeDef newState) {
	if (newState == USBD_STATE_CONFIGURED) {
		/* We have been configured, start HID functionality ! */

	}

	else if ((oldState == USBD_STATE_CONFIGURED)
			&& (newState != USBD_STATE_SUSPENDED)) {
		/* We have been de-configured, stop HID functionality */

	}

	else if (newState == USBD_STATE_SUSPENDED) {
		/* We have been suspended, stop HID functionality */
		/* Reduce current consumption to below 2.5 mA.    */

	}
}

/** @endcond */
