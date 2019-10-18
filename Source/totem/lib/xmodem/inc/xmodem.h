/*
 * xmodem.h
 *
 *  Created on: Jul 20, 2016
 *      Author: Agustin Tena
 *      Mail:   tena@fazua.com
 */

#ifndef XMODEM_H_
#define XMODEM_H_

#include "xmodem_dispatcher.h"

// XMODEM CONFIG
#define XMODEM_DATA_SIZE            32                      // Bytes of data in XMODEM message
#define XMODEM_PACKET_SIZE          XMODEM_DATA_SIZE + 5    // Total Bytes of XMODEM message (inc. header + CRC)
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

typedef enum
{
    USB_RCV         = 1,
    XMODEM_SEND     = 2,
    XMODEM_RCV      = 3
} xModemCommunicatorTypes_t;

typedef struct
{
    xModemCommunicatorTypes_t type;         // Tells the receiving task what the event is.
    int dat_len;                            // The amounts of data bytes
    uint8_t data[XMODEM_MAX_DATA_SIZE];     // Holds or points to any data associated with the event.
} xModemCommunicator_t;

// Function to send data via USB (XMODEM) to PC
bool xmodem_send_data(xModemMessageTypes_t msg_type, uint8_t subcommand, uint8_t data[], int len);

#endif // XMODEM_H_
