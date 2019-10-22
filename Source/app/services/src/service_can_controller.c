/*
 * service_can_controller.c
 *
 *  Created on: Oct 21, 2019
 *      Author: Miguel Villalba <mvillalba@boxfish.studio>
 */

#include "service_can_controller.h"

static xTaskHandle handle_can_controller;

static MCP2515_T *mcp2515;

extern xQueueHandle q_can_handle;
extern xSemaphoreHandle sem_can;

static void fill_param_struct();

/**
 *
 */
void service_can_controller_setup(const char * service_name, UBaseType_t service_priority) {

	xTaskCreate(service_can_controller, service_name, 450, NULL, service_priority,
			&handle_can_controller);
}

/**
 *
 */
void service_can_controller(void *args) {

	traceString stackTrace = INIT_STACKTRACE(CAN_CONTROLLER_SERVICE_NAME);

    CAN_Queue_t mssg_queue;
    CAN_Frame_t rxcan, *txcan;
    enum eCANErrorState errorstate = CAN_NO_ERROR;

    /* wait till the device is settled and configured */
    portTickType xLastWakeTime = xTaskGetTickCount();
    vTaskDelayUntil(&xLastWakeTime, 500 / portTICK_RATE_MS);

    mcp2515 = mcp2515_init(CAN_BAUD_250KHZ); // filter,

    if (mcp2515 != NULL)
    {
        PRINT("[CAN] successfull initialized\n");
    }
    else
    {
        PRINT("[CAN] NOT initialized\n");
    }

	for (;;) {

		/* Wait for data in queue */
		xQueueReceive(q_can_handle, (void *)&mssg_queue, portMAX_DELAY);

		/* Receive */
		if (mssg_queue.dir == CAN_QUEUE_IN)
		{

			/* Wait for data received by /INT signal from MCP2515 */
			mcp2515_inthandler(&rxcan);
			fill_param_struct();
			PRINT("Message received through CAN\n");
		}

		/* Send */
		else
		{

			txcan = &mssg_queue.canframe;
			if (errorstate != CAN_BUS_OFF)
			{
				mcp2515_txcan(txcan);
			}
			else
			{
				PRINT("[CAN ERROR] CAN in bus error: ");
				switch (errorstate)
				{
					case CAN_ERROR_PASSIVE:
						PRINT("CAN_ERROR_PASSIVE\n");
						break;
					case CAN_BUS_OFF:
						PRINT("CAN_BUS_OFF\n");
						break;
					default:
						PRINT("CAN_NO_ERROR\n");
						break;
				}
			}

			PRINT("Message sent through CAN\n");
		}

		PRINT_STACKTRACE(stackTrace);
	}
}

static void fill_param_struct()
{
	// Take semaphore only once to reduce CPU load
	xSemaphoreTake(sem_can, portMAX_DELAY);

	// Fill global status with the received data

	xSemaphoreGive(sem_can);
}
