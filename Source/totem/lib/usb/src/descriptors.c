/***************************************************************************//**
 * @file descriptors.c
 * @brief USB descriptors for HID.
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
#include "descriptors.h"

// From Book "USB Complete"
SL_ALIGN(4)
const char ReportDescriptor[34] __attribute__ ((aligned(4)))= {
        0x06,  0xFF,
		0xA0,          				//Usage Page Vendor defined
        0x09,  0x01,                //Usage (vendor-defined)
        0xA1,  0x01,                //Collection (Application)

        0x09,  0x03,                //Usage (vendor-defined)
        0x15,  0x00,                //Logical Minimum (0)
        0x26,  0x00,
		0xFF,           			//Logical Maximum (255)
        0x75,  0x08,                //Report Size (8 bits)
        0x95,  XMODEM_PACKET_SIZE,  //Report Count (size of XMODEM Packet)
        0x81,  0x02,                //Input (Data, Variable, Absolute)

        0x09,  0x04,                //Usage (vendor-defined)
        0x15,  0x00,                //Logical Minimum (0)
        0x26,  0x00,
		0xFF,          				//Logical Maximum (255)
        0x75,  0x08,                //Report Size (8 bits)
        0x95,  XMODEM_PACKET_SIZE,  //Report Count (size of XMODEM Packet)
        0x91,  0x02,                //Output (Data, Variable, Absolute)

        0xC0                        // END_COLLECTION
};

SL_ALIGN(4)
const USB_DeviceDescriptor_TypeDef USBDESC_deviceDesc SL_ATTRIBUTE_ALIGN(4)=
{
  .bLength            = USB_DEVICE_DESCSIZE,
  .bDescriptorType    = USB_DEVICE_DESCRIPTOR,
  .bcdUSB             = 0x0200,
  .bDeviceClass       = 0,
  .bDeviceSubClass    = 0,
  .bDeviceProtocol    = 0,
  .bMaxPacketSize0    = USB_FS_CTRL_EP_MAXSIZE,
  .idVendor           = 0x10C4,
  .idProduct          = 0x1002,
  .bcdDevice          = 0x0000,
  .iManufacturer      = 1,
  .iProduct           = 2,
  .iSerialNumber      = 3,
  .bNumConfigurations = 1
};

#define CONFIG_DESCSIZE ( USB_CONFIG_DESCSIZE                   + \
                          USB_INTERFACE_DESCSIZE                + \
                          USB_HID_DESCSIZE                      + \
                         (USB_ENDPOINT_DESCSIZE * NUM_EP_USED))  /*HID Functional descriptor length*/

SL_ALIGN(4)
const uint8_t USBDESC_configDesc[] SL_ATTRIBUTE_ALIGN(4)=
{
  /*** Configuration descriptor ***/
  USB_CONFIG_DESCSIZE,    /* bLength                                   */
  USB_CONFIG_DESCRIPTOR,  /* bDescriptorType                           */
  CONFIG_DESCSIZE,        /* wTotalLength (LSB)                        */
  CONFIG_DESCSIZE>>8,     /* wTotalLength (MSB)                        */
  1,                      /* bNumInterfaces                            */
  1,                      /* bConfigurationValue                       */
  0,                      /* iConfiguration                            */
#if defined(BUSPOWERED)
  CONFIG_DESC_BM_RESERVED_D7,    /* bmAttrib: Bus powered              */
#else
  CONFIG_DESC_BM_RESERVED_D7 |   /* bmAttrib: Self powered             */
  CONFIG_DESC_BM_SELFPOWERED,
#endif

  CONFIG_DESC_MAXPOWER_mA( 100 ),/* bMaxPower: 100 mA                  */

  /*** Interface descriptor ***/
  USB_INTERFACE_DESCSIZE, 	/* bLength               */
  USB_INTERFACE_DESCRIPTOR,	/* bDescriptorType       */
  0,                      	/* bInterfaceNumber      */
  0,                      	/* bAlternateSetting     */
  NUM_EP_USED,              /* bNumEndpoints         */
  0x03,          			/* bInterfaceClass hid)  */
  0,      					/* bInterfaceSubClass    */
  0,                      	/* bInterfaceProtocol    */
  0,                      	/* iInterface            */

  /*** HID descriptor ***/
  USB_HID_DESCSIZE,       			/* bLength               */
  USB_HID_DESCRIPTOR,     			/* bDescriptorType       */
  0x11,                   			/* bcdHID (LSB)          */
  0x01,                   			/* bcdHID (MSB)          */
  0,                      			/* bCountryCode          */
  1,                      			/* bNumDescriptors       */
  USB_HID_REPORT_DESCRIPTOR,        /* bDescriptorType       */
  sizeof( ReportDescriptor ),    	/* wDescriptorLength(LSB)*/
  sizeof( ReportDescriptor )>>8, 	/* wDescriptorLength(MSB)*/

  /*** Endpoint descriptor ***/
  USB_ENDPOINT_DESCSIZE,  	/* bLength               */
  USB_ENDPOINT_DESCRIPTOR,	/* bDescriptorType       */
  0x81, 					/* bEndpointAddress (IN) */
  USB_EPTYPE_INTR,        	/* bmAttributes          */
  USB_FS_INTR_EP_MAXSIZE, 	/* wMaxPacketSize (LSB)  */
  0,                      	/* wMaxPacketSize (MSB)  */
  1,       					/* bInterval             */
};

//const void *USBDESC_HidDescriptor = (void*)
//  &USBDESC_configDesc[ USB_CONFIG_DESCSIZE + USB_INTERFACE_DESCSIZE ];

STATIC_CONST_STRING_DESC_LANGID( langID, 0x04, 0x09 );

STATIC_CONST_STRING_DESC( iManufacturer, 'B','o','x','f','i','s','h',' ', \
                                         'S','t','u','d','i','o' );

STATIC_CONST_STRING_DESC( iProduct     , 'T','o','t','e','m',' ','v', 	TOTEM_VERSION_RELEASE+0x30,   	\
																	'.',                   			\
																	TOTEM_VERSION_BUILD+0x30,   		\
																	'.', 							\
																	TOTEM_VERSION_PATCH+0x30);

STATIC_CONST_STRING_DESC( iSerialNumber, '0','0','0','0','0','0',             \
                                         '0','0','1','2','3','4' );

const void * const USBDESC_strings[] =
{
  &langID,
  &iManufacturer,
  &iProduct,
  &iSerialNumber
};


/* Endpoint buffer sizes */
/* 1 = single buffer, 2 = double buffering, 3 = triple buffering ... */
const uint8_t USBDESC_bufferingMultiplier[ NUM_EP_USED + 1 ] = { 1, 1 };
