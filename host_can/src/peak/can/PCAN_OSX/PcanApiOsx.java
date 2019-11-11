package peak.can.PCAN_OSX;

import peak.can.CANif;

import peak.can.PCAN_OSX.jna.PCBUSBLibrary;
import peak.can.PCAN_OSX.jna.structures.tagTPCANMsg;
import peak.can.PCAN_OSX.jna.structures.tagTPCANTimestamp;

import com.sun.jna.Memory;
import com.sun.jna.NativeLong;
import com.sun.jna.Pointer;

/**
 * 
 * @author Begoña Alvarez, Agustin Tena
 * 
 *         JNA utility class to provide access to PCAN low level functions
 *
 */
public class PcanApiOsx extends CANif {
	// PCAN API Lib
	private final PCBUSBLibrary pcanApiLibrary;
	private static byte current_PCANUSB = PCBUSBLibrary.PCAN_NONEBUS;

	// Thread-safe implementation of Singelton Pattern
	private static PcanApiOsx instance = new PcanApiOsx();

	// Default constructor made private to ensure that no external calls are
	// possible (singelton pattern)
	private PcanApiOsx() {
		pcanApiLibrary = PCBUSBLibrary.INSTANCE;
	}

	public static PcanApiOsx getInstance() {
		return instance;
	}

	public boolean write(int ID, byte[] data, int length) {
		byte sendDat[] = new byte[8];
		for (int i = 0; i < data.length && i < sendDat.length; i++)
			sendDat[i] = data[i];

		tagTPCANMsg msg = new tagTPCANMsg(new NativeLong(ID), (byte) PCBUSBLibrary.PCAN_MESSAGE_STANDARD, (byte) length,
				sendDat);
		pcanApiLibrary.CAN_Write((byte) PCBUSBLibrary.PCAN_USBBUS1, msg);
		return true;
	}

	public byte[] read(int ID) {
		tagTPCANMsg msg = new tagTPCANMsg();
		tagTPCANTimestamp ts = new tagTPCANTimestamp();

		for (int i = 0; i < 8; i++)
			msg.DATA[i] = 0x00;

		/*
		 * Read from bus until Bootloader message is found (message filtering is not
		 * implemented in OSX version of PCAN API so this is essentially used to filter
		 * messages
		 */
		while (pcanApiLibrary.CAN_Read((byte) PCBUSBLibrary.PCAN_USBBUS1, msg, ts)
				.intValue() == PCBUSBLibrary.PCAN_ERROR_OK && msg.ID.intValue() != ID)
			;

		// Return found message
		if (msg.ID.intValue() == ID) {
			return msg.DATA;
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

	public boolean connect(int device_number) {

		NativeLong status = null;

		byte[] array_PCAN_USBBUS = { PCBUSBLibrary.PCAN_USBBUS1, PCBUSBLibrary.PCAN_USBBUS2, PCBUSBLibrary.PCAN_USBBUS3,
				PCBUSBLibrary.PCAN_USBBUS4, PCBUSBLibrary.PCAN_USBBUS5, PCBUSBLibrary.PCAN_USBBUS6,
				PCBUSBLibrary.PCAN_USBBUS7, PCBUSBLibrary.PCAN_USBBUS8 };

		// trying to connect to different PCAN_USBS Ports
		int i = 0;
		for (i = 0; i < array_PCAN_USBBUS.length; i++) {
			current_PCANUSB = (byte) PCBUSBLibrary.PCAN_NONEBUS;
			status = pcanApiLibrary.CAN_Initialize(array_PCAN_USBBUS[i], (short) PCBUSBLibrary.PCAN_BAUD_250K, (byte) 0,
					new NativeLong(0), (byte) 0);
			current_PCANUSB = array_PCAN_USBBUS[i];
			if (!status.equals(PCBUSBLibrary.PCAN_ERROR_OK)) {
				pcanApiLibrary.CAN_Reset(current_PCANUSB);
				disconnect();
			} else {
				if (device_number > 0) { // if specified a device_number

					Pointer ptr = new Memory(Long.SIZE);
					NativeLong nativelong = ptr.getNativeLong(0);
					status = pcanApiLibrary.CAN_GetValue(current_PCANUSB, (byte) PCBUSBLibrary.PCAN_DEVICE_NUMBER, ptr,
							nativelong);
					while (!status.equals(PCBUSBLibrary.PCAN_ERROR_OK)) {
						status = pcanApiLibrary.CAN_GetValue(current_PCANUSB, (byte) PCBUSBLibrary.PCAN_DEVICE_NUMBER,
								ptr, nativelong);
						// wait here until properly got the device number
					}
					System.out.println("Connected PCAN_DEVICE_NUMBER = " + ptr.getInt(0));
					if (ptr.getInt(0) == device_number) {
						break;
					} else { // not the one needed
						continue;
					}
				} else {
					break;
				}
			}
		}
		if (i == array_PCAN_USBBUS.length) {
			current_PCANUSB = (byte) PCBUSBLibrary.PCAN_NONEBUS;
			return false;
		} else {
			return true;
		}
	}

	public boolean disconnect() {
		pcanApiLibrary.CAN_Uninitialize((byte) PCBUSBLibrary.PCAN_NONEBUS);
		return true;
	}
}
