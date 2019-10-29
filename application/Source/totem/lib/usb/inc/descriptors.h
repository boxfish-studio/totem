/***************************************************************************//**
 * @file descriptors.h
 * @brief USB descriptor prototypes for CDC Serial Port adapter example project.
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
#ifndef __SILICON_LABS_DESCRIPTORS_H__
#define __SILICON_LABS_DESCRIPTORS_H__

#include "em_usb.h"
#include "hid.h"

#ifdef __cplusplus
extern "C" {
#endif

#define USB_PACKET_SIZE			37

#define TOTEM_VERSION_RELEASE 0	//TODO REMOVE FROM HERE
#define TOTEM_VERSION_BUILD 0	//TODO REMOVE FROM HERE
#define TOTEM_VERSION_PATCH 1	//TODO REMOVE FROM HERE

extern const char ReportDescriptor[34];

extern const void * USBDESC_HidDescriptor;

extern const USB_DeviceDescriptor_TypeDef USBDESC_deviceDesc;
extern const uint8_t USBDESC_configDesc[];
extern const void * const USBDESC_strings[4];
extern const uint8_t USBDESC_bufferingMultiplier[];

#ifdef __cplusplus
}
#endif

#endif /* __SILICON_LABS_DESCRIPTORS_H__ */
