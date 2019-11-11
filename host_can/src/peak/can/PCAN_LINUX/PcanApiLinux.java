package peak.can.PCAN_LINUX;

import peak.can.CANif;
import peak.can.MutableInteger;
import peak.can.basic.PCANBasic;
import peak.can.basic.TPCANBaudrate;
import peak.can.basic.TPCANHandle;
import peak.can.basic.TPCANMsg;
import peak.can.basic.TPCANParameter;
import peak.can.basic.TPCANStatus;
import peak.can.basic.TPCANTimestamp;
import peak.can.basic.TPCANType;

/**
 * 
 * @author Begona Alvarez, Agustin Tena
 * 
 *         JNA utility class to provide access to PCAN low level functions
 *
 */
public class PcanApiLinux extends CANif {

	PCANBasic can = new PCANBasic();
	private static TPCANHandle current_PCANUSB = null;

	private static PcanApiLinux instance = new PcanApiLinux();

	private PcanApiLinux() {

	}

	public static PcanApiLinux getInstance() {
		return instance;
	}

	public boolean write(int ID, byte[] data, int length) {
		byte sendDat[] = new byte[8];
		for (int i = 0; i < data.length && i < sendDat.length; i++)
			sendDat[i] = data[i];

		TPCANMsg msg = new TPCANMsg();
		msg.setID(ID);
		msg.setType(TPCANMsg.MSGTYPE_STANDARD);
		msg.setData(sendDat, (byte) length);

		can.Write(current_PCANUSB, msg);
		return true;
	}

	public byte[] read(int ID) {
		TPCANMsg msg = new TPCANMsg();
		TPCANTimestamp ts = new TPCANTimestamp();

		for (int i = 0; i < 8; i++)
			msg.getData()[i] = 0x00;

		// Read from bus until Bootloader message is found
		while (can.Read(current_PCANUSB, msg, ts) == TPCANStatus.PCAN_ERROR_OK && msg.getID() != ID)
			;
		// Return found message
		if (msg.getID() == ID) {
			return msg.getData();
		} else // No message found on bus
		{
			try {
				Thread.sleep(0, 100000); // To reduce CPU load (100µs)
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			return new byte[0];
		}
	}

	/*
	 * @param int device_number If specified device_number > 0, connect() will try
	 * to find the CAN Port that matches the specified device number
	 */
	public boolean connect(int device_number) {

		TPCANStatus status = null;

		if (!can.initializeAPI()) {
			System.out.println("Unable to initialize the API");
			System.exit(0);
		}

		TPCANHandle[] array_PCAN_USBBUS = { TPCANHandle.PCAN_USBBUS1, TPCANHandle.PCAN_USBBUS2,
				TPCANHandle.PCAN_USBBUS3, TPCANHandle.PCAN_USBBUS4, TPCANHandle.PCAN_USBBUS5, TPCANHandle.PCAN_USBBUS6,
				TPCANHandle.PCAN_USBBUS7, TPCANHandle.PCAN_USBBUS8 };

		// trying to connect to different PCAN_USBS Ports
		int i = 0;
		for (i = 0; i < array_PCAN_USBBUS.length; i++) {
			status = null;
			status = can.Initialize(array_PCAN_USBBUS[i], TPCANBaudrate.PCAN_BAUD_250K, TPCANType.PCAN_TYPE_NONE, 0,
					(short) 0);
			current_PCANUSB = array_PCAN_USBBUS[i];
			if (status != TPCANStatus.PCAN_ERROR_OK) {
				disconnect();
			} else {
				if (device_number > 0) { // if specified a device_number
					/////////////////////////////////////////
					MutableInteger integerBuffer = new MutableInteger(0);
					status = can.GetValue(current_PCANUSB, TPCANParameter.PCAN_DEVICE_NUMBER, integerBuffer,
							Integer.SIZE);
					while (status != TPCANStatus.PCAN_ERROR_OK) {
						status = can.GetValue(current_PCANUSB, TPCANParameter.PCAN_DEVICE_NUMBER, integerBuffer,
								Integer.SIZE);
					}
					System.out.println("Connected PCAN_DEVICE_NUMBER = " + integerBuffer.getValue());
					if (integerBuffer.getValue() == device_number) {
						break;
					} else { // not the one needed
						continue;
					}
					//////////////////////////////////////////
				} else {
					break;
				}
			}
		}
		if (i == array_PCAN_USBBUS.length)
			return false;
		else { // correctly connected
			return true;
		}
	}

	public boolean disconnect() {
		can.Uninitialize(current_PCANUSB);
		return true;
	}
}
