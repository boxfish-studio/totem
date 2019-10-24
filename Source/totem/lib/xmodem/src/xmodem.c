/*
 * 	xmodem.c
 *
 *  Created on: Oct 18, 2019
 *      Author: Agustin Tena <atena@boxfish.studio>
 */

#include "xmodem.h"

// Queues
extern xQueueHandle q_xmodem_stack_in;

// Semaphores
extern xSemaphoreHandle sem_xmodem_data_ready;

bool xmodem_send_data(xmodem_message_sender_t msg_type, uint8_t subcommand,
		uint8_t data[], int len) {
	// Make sure data length is within boundaries. +1 because of message type which needs to be added to package
	if (len + 2 > XMODEM_MAX_DATA_SIZE || len < 1)
		return false;

	// Prepare package for sending
	xmodem_message_t send_data;
	send_data.type = XMODEM_SEND;
	send_data.dat_len = len + 2;

	send_data.data[0] = msg_type;
	send_data.data[1] = subcommand;

	// Copy data into send structure
	memmove(&send_data.data[2], data, len);

	// Send package into queue to be handled by queue_xmodem_communicator_in
	xQueueSend(q_xmodem_stack_in, &send_data, portMAX_DELAY);
	xSemaphoreGive(sem_xmodem_data_ready);

	return true;
}
