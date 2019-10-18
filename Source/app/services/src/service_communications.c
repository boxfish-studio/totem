#include "service_communications.h"

#include "hid.h"

/*********************** Local typedefs *****************************************************/
typedef enum
{
    SENDING     = 1,
    RECEIVING   = 2,
    IDLE        = 3
} xModemCommunicatorMode_t;

typedef struct
{
    uint8_t padding; /* Padding to make sure data is 32 bit aligned. */
    uint8_t header;
    uint8_t packetNumber;
    uint8_t packetNumberC;
    uint8_t data[XMODEM_DATA_SIZE];
    uint8_t crcHigh;
    uint8_t crcLow;
} xModem_packet_t;

#define XMODEM_TIMEOUT 25          /* Amount of retries until break */

/*********************** Variable definitions **********************************************/
/* Semaphores */
extern xSemaphoreHandle sem_ISR_USB_transfer_done;
extern xSemaphoreHandle sem_activate_xmodem_communicator;
/* Queues */
extern xQueueHandle queue_xmodem_communicator_in;
extern xQueueHandle queue_xmodem_communicator_out;
extern xQueueHandle queue_usb_in;

/*********************** Local function prototypes ******************************************/
static bool xmodem_read(usbInData_t data, bool first_rcv_run);
static bool xmodem_send(xModemCommunicator_t send_data, bool first_send_run, bool ack_last_package);
static bool usb_send (uint8_t data[], int len);
static int xmodem_verifyPacketChecksum(xModem_packet_t *pkt, uint32_t sequenceNumber);
static uint16_t crc_calc(uint8_t *start, uint8_t *end);
static void receiveCallback(uint8_t **report, uint8_t remaining);
static int transmitCallback(USB_Status_TypeDef status, uint32_t xferred, uint32_t remaining);

/****** Initializations **********/
xModemCommunicatorMode_t    current_task_mode = IDLE;
xModemCommunicator_t        rcv_dispatch_data;
xModemCommunicator_t        send_data;

static xTaskHandle handle_xmodem;

/**
 *
 */
void service_communications_setup(const char * service_name, UBaseType_t service_priority) {
	xTaskCreate(service_communications, service_name, 650, NULL, service_priority,
			&handle_xmodem);
}

void service_communications(void *args)
{
	traceString stack_trace = INIT_STACKTRACE(COMMUNICATIONS_SERVICE_NAME);

    /********************** Local task variables ****************************/
    xModemCommunicator_t in_data;
    xModemCommunicator_t peek_in_data;
    usbInData_t usb_in_data;
    usbInData_t peek_usb_in_data;

    /* Status variables */
    bool first_rcv_run  = false;
    bool first_send_run = false;
    int send_state      = 0;
    int timeout_counter = 0;

    /* Enable USB peripheral */
    HID_Init(receiveCallback);

    /* Task main loop */
    while (1)
    {

        /**************************** Queue Handling *******************************/
        if (current_task_mode == IDLE)
        {
            // Wait for semaphore to activate Task
            xSemaphoreTake(sem_activate_xmodem_communicator, portMAX_DELAY);

            /* Check which queue has data in order to select the right mode */
            if (xQueuePeek(queue_xmodem_communicator_in, &peek_in_data, 50/portTICK_RATE_MS) == pdPASS)
            {
                current_task_mode = SENDING;
                xQueueReceive(queue_xmodem_communicator_in, &in_data, 0);
            }
            else if (xQueuePeek(queue_usb_in, &peek_usb_in_data, 50/portTICK_RATE_MS) == pdPASS)
            {
                current_task_mode = RECEIVING;
                xQueueReceive(queue_usb_in, &usb_in_data, 0);
            }
            else
                continue;
        }
        else
        {

            if (xQueueReceive(queue_usb_in, &usb_in_data, 50/portTICK_RATE_MS) == errQUEUE_EMPTY)
            {
                PRINT("task_xmodem_communicator(): Expected data in receive queue (queue_usb_in), but queue is empty!\n");
                timeout_counter ++;

                if (timeout_counter > XMODEM_TIMEOUT)
                {
                    /* Reset task */
                    PRINT ("task_xmodem_communicator(): Timout occured in Queue handler (receiving) - reset state of task! \n");
                    current_task_mode = IDLE;
                    timeout_counter = 0;
                    send_state = 0;
                    continue;
                }
            }
        }

        /*************************************** USB READY CHECK *********************************************/
        /* If USB not ready -> Reset task right away */
        if (USBD_GetUsbState() == USBD_STATE_NONE)
        {
            PRINT ("task_xmodem_communicator(): No USB host attached -> Reset task state \n");
            current_task_mode = IDLE;
            timeout_counter = 0;
            send_state = 0;

            //Clean up queues
            xQueueReset(queue_usb_in);
            xQueueReset(queue_xmodem_communicator_in);
            xQueueReset(queue_xmodem_communicator_out);
        }

        /* ********************************************** STATE MACHINE ******************************************** */
        if (current_task_mode == RECEIVING)
        {
            if (usb_in_data.data[0] == XMODEM_SOT)
            {
                /* Acknowledge that the SOT was recognised */
                uint8_t data[] = {XMODEM_ACKSOT};
                usb_send(data, 1);

                first_rcv_run = true;
                continue;

            }
            else if (usb_in_data.data[0] == XMODEM_EOT)
            {
                /* Acknowledge EOT */
                uint8_t data[] = {XMODEM_ACKEOT};
                usb_send(data, 1);

                // Send received data to the dispatcher; clear buffer
                xQueueSendToBack(queue_xmodem_communicator_out, &rcv_dispatch_data, 0);
                first_rcv_run = false;
                timeout_counter = 0;
                current_task_mode = IDLE;
                continue;
            }

            if (current_task_mode == RECEIVING)
            {
                if (!xmodem_read(usb_in_data, first_rcv_run))
                {
                    PRINT("task_xmodem_communicator(): Receiving FAILED! Set mode to IDLE\n");

                    /* Reset task state */
                    current_task_mode = IDLE;
                    timeout_counter = 0;
                    first_rcv_run = false;
                    continue;
                }

                first_rcv_run = false;
            }
        }
        /* Sending data to PC */
        else if (current_task_mode == SENDING)
        {
            switch (send_state)
            {
                /* SOT */
                case 0:
                {
                    /* Reset timeout counter (new transmission) */
                    timeout_counter = 0;

                    uint8_t data[] = {XMODEM_SOT};
                    usb_send(data, 1);

                    // Keep data to be sent in new data structure
                    send_data = in_data;

                    // Move to next step
                    send_state ++;

                    break;
                }
                /* ACKSOT / NAKSOT */
                case 1:
                {
                    if (usb_in_data.data[0] != XMODEM_ACKSOT)
                    {
                        PRINT ("task_xmodem_communicator(): send_state 1: Received NAKSOT or no data from PC\n");

                        timeout_counter ++;
                        if (timeout_counter > XMODEM_TIMEOUT)
                        {
                            PRINT ("task_xmodem_communicator(): Timout occured in sending switch (state 1)! Reset task to initial state... \n");
                            current_task_mode = IDLE;
                            send_state = 0;
                            continue;
                        }

                        // Send SOT again
                        uint8_t data[] = {XMODEM_SOT};
                        usb_send(data, 1);
                        break;
                    }
                    else
                    {
                        first_send_run = true;
                        send_state ++;
                    }
                }
                /* Communication established --> actual sending */
                case 2:
                {

                    if (!first_send_run && usb_in_data.data[0] != XMODEM_ACK)
                    {
                        timeout_counter ++;
                    }

                    if (!first_send_run && usb_in_data.data[0] == XMODEM_CAN)
                    {
                        current_task_mode = IDLE;
                        timeout_counter = 0;
                        send_state = 0;
                        continue;
                    }

                    if (timeout_counter > XMODEM_TIMEOUT)
                    {
                        PRINT ("task_xmodem_communicator(): Timout occured in sending switch (state 2)! Reset task to initial state... \n");
                        current_task_mode = IDLE;
                        send_state = 0;
                        timeout_counter = 0;
                        continue;
                    }

                    if (xmodem_send(send_data, first_send_run, usb_in_data.data[0] == XMODEM_ACK))
                        send_state ++;
                    else
                    {
                        first_send_run = false;
                        break;
                    }
                }
                /* End Communication */
                case 3:
                {
                    uint8_t data[] = {XMODEM_EOT};
                    usb_send(data, 1);

                    send_state ++;
                    break;
                }
                /* ACKEOT / NAKEOT */
                case 4:
                {

                    if (usb_in_data.data[0] != XMODEM_ACKEOT)
                    {
                        PRINT ("task_xmodem_communicator(): send_state 4: Didn't receive ACKEOT from PC\n");
                        //TODO: Make timer local
                        timeout_counter ++;

                        if (timeout_counter > XMODEM_TIMEOUT)
                        {
                            PRINT ("task_xmodem_communicator(): send_state 4: Timout occured while waiting for ACKEOT from PC! Reset task to initial state... \n");
                            current_task_mode = IDLE;
                            send_state = 0;
                            timeout_counter = 0;
                            continue;
                        }

                        uint8_t data[] = {XMODEM_EOT};
                        usb_send(data, 1);
                    }
                    else
                    {

                        first_send_run = false;
                        current_task_mode = IDLE;
                        send_state = 0;
                        timeout_counter = 0;
                    }
                    break;
                }
                /* Should never get here */
                default:
                {
                    send_state = 0;
                    timeout_counter = 0;
                    current_task_mode = IDLE;
                    break;
                }
            } //END switch
        }

        PRINT_STACKTRACE(stack_trace);
    }
}


/*********************** Local helper functions **********************************************/
/* Returns true when everything (whole data) is sent */
static bool xmodem_send(xModemCommunicator_t send_data, bool first_send_run, bool ack_last_package)
{
    /* Define local variables */
    static int bytes_sent;
    static uint8_t sequenceNumber;
    uint8_t out_data[XMODEM_PACKET_SIZE];

    /* Init variables for first run
     * Check whether last package was transmitted successfully
     * Failed check will automatically result in sending the last package again */
    if (first_send_run)
    {
        sequenceNumber = 1;
        bytes_sent = 0;
    }
    else
    {
        if (ack_last_package)
        {
            bytes_sent += XMODEM_DATA_SIZE;

            /* Check if the last sent package was the last one for the current transmission */
            if (bytes_sent >= send_data.dat_len)
                return true;

            /* Calculate new sequence number (see XMODEM standard) */
            if (sequenceNumber == 255)
                sequenceNumber = 0;
            else
                sequenceNumber++;
        }
        else
        {
            PRINT("xmodem_send(): Didn't receive ACK, send package again!\n");
        }

    }


    /* Build up data frame to be sent */
    out_data[0] = XMODEM_SOH;
    out_data[1] = sequenceNumber;
    out_data[2] = (~sequenceNumber);

    /* Copy data into frame */
    if (bytes_sent + XMODEM_DATA_SIZE <= send_data.dat_len)
    {
        for (int i = 0; i < XMODEM_DATA_SIZE; i ++)
            out_data[i + 3] = send_data.data[bytes_sent + i];
    }
    else //Last package -> add padding
    {
        for (int i = 0; i < send_data.dat_len - bytes_sent; i ++)
            out_data[i + 3] = send_data.data[bytes_sent + i];

        /* Pad rest of package with 0xff */
        for (int i = send_data.dat_len - bytes_sent; i < XMODEM_DATA_SIZE; i ++)
            out_data[i + 3] = 0xff;
    }

    /* Calculate and add CRC to package */
    uint16_t crc = crc_calc(&out_data[3], &out_data[XMODEM_DATA_SIZE + 3]);
    out_data[XMODEM_DATA_SIZE + 3] = crc >> 8;
    out_data[XMODEM_DATA_SIZE + 4] = crc & 0x00FF;

    usb_send(out_data, XMODEM_PACKET_SIZE);

    return false;
}

static bool xmodem_read(usbInData_t data, bool first_rcv_run)
{
    static uint32_t sequenceNumber;
    static int rcv_dispatch_data_position;
    xModem_packet_t pkt;

    /* If it is the first packet of a new receive sequence --> init variables */
    if (first_rcv_run)
    {
        sequenceNumber = 1;
        rcv_dispatch_data_position = 0;
        rcv_dispatch_data.type = XMODEM_RCV;
        rcv_dispatch_data.dat_len = 0;
    }

    /* Fill packet structure with received data */
    pkt.header         = data.data[0];
    pkt.packetNumber   = data.data[1];
    pkt.packetNumberC  = data.data[2];

    // Read the payload of the packet and assign to packet structure
    for (int i = 0; i < XMODEM_DATA_SIZE; i++)
        pkt.data[i] = data.data[i + 3];

    // Read CRC
    pkt.crcHigh = data.data[XMODEM_DATA_SIZE + 3];
    pkt.crcLow  = data.data[XMODEM_DATA_SIZE + 4];

    /* If header is cancel message, then cancel the transfer */
    if (pkt.header == XMODEM_CAN)
    {
        PRINT ("xmodem_read(): CAN (cancel) received from PC\n");
        return false;
    }

    /* If the header is not a start of header (SOH), then cancel *
     * the transfer. */
    if (pkt.header != XMODEM_SOH)
    {
        PRINT("xmodem_read(): Header not valid! Abort!\n");
        uint8_t dat[1] = {XMODEM_CAN};
        usb_send(dat, 1);

        return false;
    }

    /* Verify that the packet is valid */
    if (xmodem_verifyPacketChecksum(&pkt, sequenceNumber) != 0)
    {
        // On a malformed packet, send a NAK and start over */
        PRINT("xmodem_read(): Broken packet received . Start over\n");
        uint8_t dat[1] = {XMODEM_NAK};
        usb_send(dat, 1);

        return true;
    }


    /* Calculate new sequence number (see XMODEM standard) */
    if (sequenceNumber == 255)
        sequenceNumber = 0;
    else
        sequenceNumber++;


    /* Check if total rcv size is still within XMODEM_MAX_DATA_SIZE */
    if (rcv_dispatch_data_position + XMODEM_DATA_SIZE > XMODEM_MAX_DATA_SIZE)
    {
        PRINT("xmodem_read(): Received data too big for input buffer! CANCEL!\n");
        uint8_t dat[1] = {XMODEM_CAN};
        usb_send(dat, 1);
        return true;
    }

    /* Send ACK */
    uint8_t dat[1] = {XMODEM_ACK};
    usb_send(dat, 1);

    /* Add package recieved to total data */
    for (int i = 0; i < XMODEM_DATA_SIZE; i ++)
        rcv_dispatch_data.data[rcv_dispatch_data_position + i] = pkt.data[i];

    /* Add amount of data written to absolute position */
    rcv_dispatch_data_position += XMODEM_DATA_SIZE;
    rcv_dispatch_data.dat_len += XMODEM_DATA_SIZE;

    /* Return success */
    PRINT ("xmodem_read(): Packet-Transfer completed!\n");
    return true;
}

static bool usb_send (uint8_t data[], int len)
{

    if (! USBD_EpIsBusy(0x81))
    {
        if (len > XMODEM_PACKET_SIZE)
        {
            PRINT ("usb_send(): Sending failed! Data length doesn't match!!\n");
            return false;
        }

        vTaskSuspendAll();
        USBD_Write(0x81, (void*) data, XMODEM_PACKET_SIZE, transmitCallback);
        xTaskResumeAll();
        // Wait for completion of transmission
        if (xSemaphoreTake(sem_ISR_USB_transfer_done, 100 / portTICK_RATE_MS) != pdPASS)
        {
            PRINT ("usb_send(): ERROR!: Sending data failed!\n");
            return false;
        }


        return true;
    }
    else
    {
        return false;
    }
}

static int xmodem_verifyPacketChecksum(xModem_packet_t *pkt, uint32_t sequenceNumber)
{
    uint16_t packetCRC;
    uint16_t calculatedCRC;

    /* Check the packet number integrity */
    if (pkt->packetNumber + pkt->packetNumberC != 255)
    {
        PRINT ("xmodem_verifyPacketChecksum(): Packet number integrity fault!\n");
        return -1;
    }

    /* Check that the packet number matches the excpected number */
    if (pkt->packetNumber != (sequenceNumber % 256))
    {
        PRINT ("xmodem_verifyPacketChecksum(): Expected sequence number doesn't match!\n");
        return -1;
    }

    calculatedCRC = crc_calc((uint8_t *) pkt->data, (uint8_t *) &(pkt->crcHigh));
    packetCRC     = pkt->crcHigh << 8 | pkt->crcLow;

    /* Check the CRC value */
    if (calculatedCRC != packetCRC)
    {
        PRINT ("xmodem_verifyPacketChecksum(): CRC doesn't match!\n");
        return -1;
    }

    return 0;
}

static uint16_t crc_calc(uint8_t *start, uint8_t *end)
{
    uint16_t crc = 0x0;
    uint8_t  *data;

    int cnt = 0;

    for (data = start; data < end; data++)
    {
        crc  = (crc >> 8) | (crc << 8);
        crc ^= *data;
        crc ^= (crc & 0xff) >> 4;
        crc ^= crc << 12;
        crc ^= (crc & 0xff) << 5;

        cnt ++;
    }

    return crc;
}

static void receiveCallback(uint8_t **report, uint8_t remaining)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    if (!remaining) {
		usbInData_t usbData;
		/* Copy received data into queue packet */
		memcpy (&usbData.data, report, XMODEM_PACKET_SIZE);

		/* Send received data to queue */
		xQueueSendFromISR(queue_usb_in, &usbData, &xHigherPriorityTaskWoken);
		xSemaphoreGiveFromISR(sem_activate_xmodem_communicator, &xHigherPriorityTaskWoken);
    }

    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}

static int transmitCallback(USB_Status_TypeDef status, uint32_t xferred, uint32_t remaining)
{
    portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

    if (remaining == 0)
    {
        xSemaphoreGiveFromISR(sem_ISR_USB_transfer_done, &xHigherPriorityTaskWoken);
    }

    portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
    return USB_STATUS_OK;
}
