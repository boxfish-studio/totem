package usb.xmodem;

import java.io.IOException;
import java.lang.invoke.MethodHandles;
import java.util.ArrayList;
import java.util.concurrent.ConcurrentLinkedDeque;

import org.apache.logging.log4j.Logger;

import usb.UsbHidModel;

import org.apache.logging.log4j.LogManager;

public class XmodemModel implements Runnable {

	private UsbHidModel usbDevice;
	private ConcurrentLinkedDeque<byte[]> rcvQueue;
	private ConcurrentLinkedDeque<byte[]> sendQueue;

	private static final Logger logger = LogManager.getLogger(MethodHandles.lookup().lookupClass().getSimpleName());

	private volatile int threadStatus = 0; // 0 = ready, 1 = running, 2 = stop

	/* Class variable initialization */
	final int MAX_ERRORS = 20; // Maximum amount of re-transmissions if transmission failed ##10

	/* Defines for XMODEM protocol */
	private static final int MAXERRORS = 100;
	private static final int SECSIZE = 32;

	/* Protocol characters used */
	private static final byte SOT = 0x21; /* Start of Transmission */
	private static final byte ACKSOT = 0x22; /* ACK Start of Transmission */
	private static final byte NAKSOT = 0x23; /* NAK Start of Transmission */

	private static final byte NCG = 0x43; /* Initial Character */
	private static final byte SOH = 0x01; /* Start Of Header (signals regular data package) */
	private static final byte EOT = 0x04; /* End Of Transmission */
	private static final byte ACKEOT = 0x25; /* Acknowledge End Of Transmission */
	private static final byte NAKEOT = 0x26; /* NOTacknowledge End of Transmission */

	private static final byte ACK = 0x06; /* ACKnowlege */
	private static final byte NAK = 0x15; /* Negative AcKnowlege */
	private static final byte CAN = 0x18; /* CANcel */

	public XmodemModel(UsbHidModel usbDevice, ConcurrentLinkedDeque<byte[]> rcvQueue,
			ConcurrentLinkedDeque<byte[]> sendQueue) {
		this.usbDevice = usbDevice;

		this.rcvQueue = rcvQueue;
		this.sendQueue = sendQueue;
		// this.connNotifier = connNotifier;
	}

	public void stopThread() {
		// Thread isn't running
		if (threadStatus == 0)
			return;

		threadStatus = 3;

		/* Wait until thread stopped */
		while (threadStatus != 0) {
			try {
				Thread.sleep(250);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
		}

	}

	/* Endlessly running thread to communicate with controller */
	private void xModemCommunicator() {
		/* Set status to "running" */
		threadStatus = 1;

		System.out.println("Xmodem: START");
		while (threadStatus == 1) {
			/* Nothing to send -> wait for data from controller */
			if (sendQueue.isEmpty()) {
				try {
					xModemRead();
				} catch (IOException e) {
					logger.info("Xmodem: Lost connection to controller!");
					threadStatus = 0;
					// connNotifier.connectionLost();
					return; // Leave Thread
				}
			} else {
				try {
					xModemWrite();
				} catch (IOException e) {
					logger.info("Xmodem:Lost connection to controller!");
					threadStatus = 0;
					// connNotifier.connectionLost();
					return; // Leave Thread
				}
			}
			// System.out.println("xmodemmodel running");
		}

		/* Set status to "Ready" */
		threadStatus = 0;
		System.out.println("Xmodem: STOPPED");
	}

	/* Handling all xModem read operations */
	private boolean xModemRead() throws IOException {
		/* Init method variables */
		ArrayList<Byte> completeDate = new ArrayList<Byte>();
		int errorCnt = 0;

		byte data[] = usbDevice.deviceRead();

		if (data.length == 0) {
			// System.out.println("No data from controller coming");
			return false;
		} else {
			// ==============================================
			// HARD PATCH TO PREVENT COMMUNICATION ERROR LOOP
			if (data[0] == EOT) {
				System.out.println("Data 0: EOT - Nothing to do");
				return false;
			}

			else if (data[0] == SOH) {
				System.out.println("Data 0: SOH - Nothing to do");
				return false;
			}

			else if (data[0] == CAN) {
				System.out.println("Data 0: CAN - Nothing to do");
				return false;
			}
			// ==============================================

			else if (data[0] != SOT) {
				System.err.println("Data 0: " + data[0]);
				usbDevice.deviceWrite(new byte[] { NAKSOT });
				System.err.println("NAKSOT");
				return false;
			}

			else {
				// System.out.println("Data 0: SOT");
				usbDevice.deviceWrite(new byte[] { ACKSOT });
				// System.err.println("ACKSOT");
			}
		}

		/************* Start receiving data **************************/
		int sequenceNumber = 1;

		/* Start receiving XMODEM packets */
		while (true) {
			/* Packet variables */
			byte header = 0x00;
			byte packetNumber = 0x00;
			byte packetNumberC = 0x00;
			byte crcHigh = 0x00;
			byte crcLow = 0x00;
			byte packetData[] = new byte[SECSIZE];

			/* Receive XMODEM packet */
			errorCnt = 0;
			while (true) {
				data = usbDevice.deviceRead();

				if (data.length == 0)
					errorCnt++;
				else
					break;

				if (errorCnt == MAXERRORS) {
					System.err.println("xModemRead(): Timeout, didn't receive data from controller!");
					return false;
				}
			}

			/* Fill packet structure with received data */
			header = data[0];
			packetNumber = data[1];
			packetNumberC = data[2];

			// Read the payload of the packet and assign to packet structure
			for (int i = 0; i < SECSIZE; i++)
				packetData[i] = data[i + 3];

			// Read CRC
			crcHigh = data[SECSIZE + 3];
			crcLow = data[SECSIZE + 4];

			/* Check for end of transfer */
			if (header == EOT) {
				// System.out.println ("xmodem_download(): EOT received from PC\n");

				/* Acknowledge End of transfer */
				usbDevice.deviceWrite(new byte[] { ACKEOT });
				break;
			}

			/* If header is cancel message, then cancel the transfer */
			if (header == CAN) {
				System.out.println("xmodem_download(): CAN (cancel) received from PC\n");
				return false;
			}

			/*
			 * If the header is not a start of header (SOH), then cancel * the transfer.
			 */
			// TODO: Think about sending CANcel message
			if (header != SOH) {
				/* Send CAN */
				System.out.println("Header: " + Integer.toHexString(header));
				usbDevice.deviceWrite(new byte[] { CAN });
				System.out.println("xmodem_download(): Header not valid! Abort!\n");
				return false;
			}

			/* Verify that the packet is valid */
			if (!verifyPacketChecksum(packetNumber, packetNumberC, sequenceNumber, data, crcHigh, crcLow)) {
				// On a malformed packet, send a NAK and start over */
				System.err.println("xmodem_download(): Broken packet received -> Start over\n");

				usbDevice.deviceWrite(new byte[] { NAK });
				continue;
			}

			/*******************************************************
			 * Copy Data to data-object for all data received as part of this transmission
			 *******************************************************/
			for (int i = 0; i < packetData.length; i++)
				completeDate.add(packetData[i]);

			/* Calculate new sequence number (see XMODEM standard) */
			if (sequenceNumber == 255)
				sequenceNumber = 0;
			else
				sequenceNumber++;

			/* Send ACK */
			usbDevice.deviceWrite(new byte[] { ACK });
		}

		/* Return success */
		// System.out.println ("xModemRead(): Transfer completed!\n");

		/*
		 * Send data to queue for further processing in different thread (Unboxing to
		 * byte required)
		 */
		byte[] queueData = new byte[completeDate.size()];

		for (int i = 0; i < completeDate.size(); i++)
			queueData[i] = completeDate.get(i);

		rcvQueue.add(queueData);

		return true;
	}

	/* Handling all xModem write operations */
	private Boolean xModemWrite() throws IOException {
		if (sendQueue.isEmpty())
			return false;

		byte data[] = sendQueue.poll();
		// System.out.println("xModemWrite(): Send something to controller!"); // TODO
		// DO SOMETING WITH THE PRINTS

		char errorCount = 0;
		char blockNumber = 1;
		int byteCnt = 0;
		byte character[] = {};

		/***********************************************************************************
		 * Send start of transmission and wait for controller to ACKSOT the receiving
		 ***********************************************************************************/
		while (true) {
			usbDevice.deviceWrite(new byte[] { SOT });
			character = usbDevice.deviceRead();

			if ((character.length == 0 && errorCount < MAXERRORS)
					|| (character[0] != ACKSOT && errorCount < MAXERRORS)) {
				errorCount++;
				if (errorCount == MAXERRORS) {
					// System.err.println("Received: " + Integer.toHexString(character[0]));
					System.err.println(usbDevice.getDevice().getProduct()
							+ ": xModemWrite(): Didn't receive ACK for SOT from controller!");
					return false;
				}
			} else
				break;

		}

		// System.out.println("xModemWrite(): Received ACKnowledge (ACKSOT) from
		// Controller to start transmission"); // TODO THINK WHAT TO DO WITH THIS

		/* Initialize variables for send procedure */
		int crcCheckSum = 0x00;
		byte[] sendSector = new byte[SECSIZE];
		byte[] sendPackage = new byte[SECSIZE + 5];

		while (byteCnt <= data.length) {
			/*
			 * Read sector from array
			 * 
			 * Check if package is the last one (sec size too small) Pad package with 0xffff
			 * Else just copy array to sendSector
			 */

			if (byteCnt + SECSIZE <= data.length)
				System.arraycopy(data, byteCnt, sendSector, 0, SECSIZE);
			else {
				int i = 0;
				for (i = 0; i < (data.length - byteCnt); i++) {
					sendSector[i] = data[i + byteCnt];
				}

				while (i < SECSIZE) {
					sendSector[i] = (byte) 0xFF;
					i++;
				}
			}

			byteCnt += SECSIZE;

			// Reset error count for every new transmission
			errorCount = 0;

			/**********************************************
			 * Assemble package to send
			 **********************************************/
			sendPackage[0] = SOH;
			sendPackage[1] = (byte) blockNumber;
			sendPackage[2] = (byte) (~blockNumber);

			System.arraycopy(sendSector, 0, sendPackage, 3, SECSIZE);

			crcCheckSum = ChecksumCalculator.crcCalc(sendSector, SECSIZE);
			sendPackage[SECSIZE + 3] = (byte) (crcCheckSum >>> 8);
			sendPackage[SECSIZE + 4] = (byte) (crcCheckSum & 0x00FF);

			/*******************************************************************
			 * Send assembled package to controller (see XMODEM protocol spec)
			 *******************************************************************/
			while (errorCount < MAX_ERRORS) {
				/* Send package via USB to Controller */
				// System.out.println("xModemWrite(): Sending package " + (int)blockNumber + "
				// to controller...");
				usbDevice.deviceWrite(sendPackage);

				/* Wait for ACK or NAK from Controller */
				while (true) {
					character = usbDevice.deviceRead();

					/**************************************************************************
					 * After the start it is possible that NCG(0x43) is still buffered Therefore if
					 * NCG received, receive again until valid value is received
					 *************************************************************************/
					if (character.length > 0 && character[0] == NCG) {
						System.out.println("Clearing input buffer...");
						continue;
					}

					if (character.length > 0)
						break;
					else
						errorCount++;

					if (errorCount == MAXERRORS) {
						System.err.println(
								"xModemWrite(): Too many errors caught. Transmission canceled! Controller didn't send ACK/NAK");
						return false;
					}
				}

				/* Check response */
				if (character[0] == ACK)
					break;
				else if (character[0] == NAK)
					errorCount++;
				else if (character[0] == CAN) {
					System.err.println("xModemWrite(): Receiver canceled transmission!");
					return false;
				}
			}

			if (errorCount == MAXERRORS) {
				System.err.println("xModemWrite(): Too many errors caught. Transmission canceled!");
				return false;
			}

			if (blockNumber == 255)
				blockNumber = 0;
			else
				blockNumber++;
		}

		/***********************************************************
		 * All data sent -> Finish transmission accordingly to protocol
		 ***********************************************************/
		errorCount = 0;

		while (errorCount < MAX_ERRORS) {
			usbDevice.deviceWrite(new byte[] { EOT });
			// System.out.println("xModemWrite(): Sent EOT to controller!"); //TODO DO
			// SOMETHING WITH

			/* Wait for ACK, NAK, CAN */
			while (true) {
				character = usbDevice.deviceRead();

				if (character.length > 0 && character[0] == NAKEOT) {
					errorCount++;
					break;
				} else if (character.length > 0 && character[0] == CAN) {
					System.err.println("xModemWrite(): Receiver CANCELED transmission, something went wrong!");
					return false;
				} else if (character.length > 0 && character[0] == ACKEOT)
					break;
				else
					errorCount++;

				if (errorCount == MAX_ERRORS) {
					System.err.println("xModemWrite(): Too many errors caught. Couldn't end transmission!");
					return false;
				}
			}

			// Leave loop
			if (character.length > 0 && character[0] == ACKEOT)
				break;
		}

		// System.out.println("xModemWrite(): Send process completed!"); // TODO DO
		// SOMETHING WITH THE PRINTS
		return true;
	}

	private Boolean verifyPacketChecksum(byte packetNumber, byte packetNumberC, int sequenceNumber, byte[] data,
			byte crcHigh, byte crcLow) {
		int packetCRC;
		int calculatedCRC;

		/* Check the packet number integrity */
		if (packetNumber + packetNumberC != (byte) 0xff)
			return false;

		/* Check that the packet number matches the expected number */
		if (packetNumber != (sequenceNumber % 256))
			return false;

		/* Copy plain data to new array for checksum calculation */
		byte plainData[] = new byte[SECSIZE];
		System.arraycopy(data, 3, plainData, 0, SECSIZE);

		calculatedCRC = ChecksumCalculator.crcCalc(plainData, plainData.length) & 0x0000FFFF;
		packetCRC = (crcHigh << 8 | (crcLow & 0x000000ff)) & 0x0000FFFF;

		/* Check the CRC value */
		if (calculatedCRC != packetCRC)
			return false;

		return true;
	}

	/* Thread start method */
	public void run() {
		/* Make sure only one instance of thread can run */
		if (threadStatus != 0) {
			System.err.println("XMODEMMODEL: TRIED TO START RUNNING THREAD AGAIN!");
			return;

		}

		xModemCommunicator();
	}
}
