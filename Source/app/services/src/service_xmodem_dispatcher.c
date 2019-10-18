/*
 * task_xmodem_dispatcher.c
 *
 *      Created on: Jul 20, 2016
 *      Author: Agustin Tena
 *      Mail:   tena@fazua.com
 */
#include "service_xmodem_dispatcher.h"

#include "xmodem.h"

// RTOS Queues
extern xQueueHandle queue_xmodem_communicator_out;

static xTaskHandle handle_dispatcher;

/**
 *
 */
void service_xmodem_dispatcher_setup(const char * service_name, UBaseType_t service_priority) {
	xTaskCreate(service_xmodem_dispatcher, service_name, 650, NULL, service_priority,
			&handle_dispatcher);
}

/**************************************************************************
 * @brief Receives packages from xmodem_communicator and dispatches them
 * to the right place in the application
 * @param *pParameters pointer to parameters passed to the function
 *****************************************************************************/
void service_xmodem_dispatcher(void *args)
{
    INIT_STACKTRACE("task_xmodem_dispatcher");

    xModemCommunicator_t in_data;
    uint8_t rx_counter = 0;

    while (1)
    {
        // Wait for data to come in from xmodem_communicator
        xQueueReceive(queue_xmodem_communicator_out, &in_data, portMAX_DELAY);
        PRINT("-------------------- Task task_xmodem_dispatcher running -----------------------------\n");

        // Check whether data is meant for this task
        if (in_data.type != XMODEM_RCV)
            continue;

        // Check type of data and take appropriate action
        switch (in_data.data[0])
        {
            case XMODEM_MASTER_MSG:
                PRINT ("task_xmodem_dispatcher(): XMODEM_MASTER_MSG received\n");
                xmodem_dispatch_master_msg(in_data.data, in_data.dat_len);

                // Slave msgs half frequency that master msgs
                rx_counter++;
                if(rx_counter>=2){
                	rx_counter = 0;
                	xmodem_send_slave_status_msg();
                }
                break;
            default:
                break;
        }

        PRINT_STACKTRACE("task_xmodem_dispatcher");
    }
}
