/*
 * xmodem.c
 *
 *  Created on: Mar 6, 2015
 *      Author: Simon Fischinger
 *      Mail:   sfischinger@synapticon.com
 */

#include "xmodem.h"

#include "totem_common.h"
#include <string.h>

/* Queues */
extern xQueueHandle queue_xmodem_communicator_in;

/* Semaphores */
extern xSemaphoreHandle sem_activate_xmodem_communicator;

bool xmodem_send_data(xModemMessageTypes_t msg_type, uint8_t subcommand, uint8_t data[], int len)
{
    /* Make sure data length is within boundaries
     * (+1 because of message type which needs to be added to package
     * */
    if (len + 2 > XMODEM_MAX_DATA_SIZE || len < 1)
        return false;

    /* Prepare package for sending */
    xModemCommunicator_t send_data;
    send_data.type = XMODEM_SEND;
    send_data.dat_len = len + 2;

    send_data.data[0] = msg_type;
    send_data.data[1] = subcommand;

    /* Copy data into send structure */
    memmove(&send_data.data[2], data, len);

    /* Send package into queue to be handled by queue_xmodem_communicator_in */
    xQueueSend(queue_xmodem_communicator_in, &send_data, portMAX_DELAY);
    xSemaphoreGive(sem_activate_xmodem_communicator);

    return true;
}
