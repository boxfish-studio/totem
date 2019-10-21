/*
 * 	xmodem.h
 *
 *  Created on: Oct 18, 2019
 *      Author: Agustin Tena <atena@boxfish.studio>
 */

#ifndef XMODEM_H_
#define XMODEM_H_

#include "totem_mcu_common.h"
#include "totem_freertos_common.h"

// XMODEM CONFIG
#define XMODEM_PACKET_SIZE          USB_PACKET_SIZE    		// Total Bytes of XMODEM message (inc. header + CRC)
#define XMODEM_DATA_SIZE            XMODEM_PACKET_SIZE - 5  // Bytes of data in XMODEM message
#define XMODEM_MAX_DATA_SIZE        512                     // Maximum size of one transmission

/* XMODEM protocol states used */
#define XMODEM_SOT     0x21        /* Start of Transmission */
#define XMODEM_ACKSOT  0x22        /* ACK Start of Transmission */
#define XMODEM_NAKSOT  0x23        /* NAK Start of Transmission */

#define XMODEM_NCG     0x43        /* Initial Character */
#define XMODEM_SOH     0x01        /* Start Of Header (signals regular data package) */
#define XMODEM_EOT     0x04        /* End Of Transmission */
#define XMODEM_ACKEOT  0x25        /* End Of Transmission */
#define XMODEM_NAKEOT  0x26        /* End Of Transmission */

#define XMODEM_ACK     0x06        /* ACKnowlege */
#define XMODEM_NAK     0x15        /* Negative AcKnowlege */
#define XMODEM_CAN     0x18        /* CANcel */

// Message types
typedef enum {
	XMODEM_MASTER_MSG = 1, XMODEM_SLAVE_MSG = 2
} xmodem_message_sender_t;

typedef enum {
	USB_RCV = 1, XMODEM_SEND = 2, XMODEM_RCV = 3
} xmodem_message_type_t;

typedef struct {
	xmodem_message_type_t type;   // Tells the receiving task what the event is.
	int dat_len;                            // The amounts of data bytes
	uint8_t data[XMODEM_MAX_DATA_SIZE]; // Holds or points to any data associated with the event.
} xmodem_message_t;

// Function to send data via USB (XMODEM) to PC
bool xmodem_send_data(xmodem_message_sender_t msg_type, uint8_t subcommand,
		uint8_t data[], int len);

#endif // XMODEM_H_
