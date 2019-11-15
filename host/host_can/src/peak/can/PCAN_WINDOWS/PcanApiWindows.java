package peak.can.PCAN_WINDOWS;

import com.sun.jna.Memory;
import com.sun.jna.Pointer;

import peak.can.CANif;
import peak.can.PCAN_WINDOWS.jna.PCANBasicLibrary;
import peak.can.PCAN_WINDOWS.jna.structures.tagTPCANMsg;
import peak.can.PCAN_WINDOWS.jna.structures.tagTPCANTimestamp;
import peak.can.PCAN_OSX.jna.PCBUSBLibrary;

/**
 *
 * @author Begoña Alvarez, Agustin Tena
 *
 *         JNA utility class to provide access to PCAN low level functions
 *
 */
public class PcanApiWindows extends CANif {
	private final PCANBasicLibrary pcanApiLibrary;
	private static short current_PCANUSB = PCANBasicLibrary.PCAN_NONEBUS;

	private static PcanApiWindows instance = new PcanApiWindows();

	// Default constructor made private to ensure that no external calls are
	// possible (singelton pattern)
	private PcanApiWindows() {
		pcanApiLibrary = PCANBasicLibrary.INSTANCE;
	}

	public static PcanApiWindows getInstance() {
		return instance;
	}

	public boolean write(int ID, byte[] data, int length) {
		byte sendDat[] = new byte[8];
		for (int i = 0; i < data.length && i < sendDat.length; i++)
			sendDat[i] = data[i];

		tagTPCANMsg msg = new tagTPCANMsg(ID, (byte) PCANBasicLibrary.PCAN_MESSAGE_STANDARD, (byte) length, sendDat);

		pcanApiLibrary.CAN_Write(current_PCANUSB, msg);
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
		while (pcanApiLibrary.CAN_Read(current_PCANUSB, msg, ts) == PCBUSBLibrary.PCAN_ERROR_OK && msg.ID != ID)
			;

		if (msg.ID == ID) {
			return msg.DATA;
		} else {
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

		int status;
		short[] array_PCAN_USBBUS = { PCBUSBLibrary.PCAN_USBBUS1, PCBUSBLibrary.PCAN_USBBUS2,
				PCBUSBLibrary.PCAN_USBBUS3, PCBUSBLibrary.PCAN_USBBUS4, PCBUSBLibrary.PCAN_USBBUS5,
				PCBUSBLibrary.PCAN_USBBUS6, PCBUSBLibrary.PCAN_USBBUS7, PCBUSBLibrary.PCAN_USBBUS8 };

		// trying to connect to different PCAN_USBS Ports
		int i = 0;
		for (i = 0; i < array_PCAN_USBBUS.length; i++) {
			current_PCANUSB = (byte) PCBUSBLibrary.PCAN_NONEBUS;
			status = pcanApiLibrary.CAN_Initialize(array_PCAN_USBBUS[i], (short) PCBUSBLibrary.PCAN_BAUD_250K, (byte) 0,
					0, (short) 0);
			current_PCANUSB = array_PCAN_USBBUS[i];
			if (status != PCBUSBLibrary.PCAN_ERROR_OK) {
				pcanApiLibrary.CAN_Reset(current_PCANUSB);
				disconnect();
			} else {
				if (device_number > 0) { // if specified a device_number

					Pointer ptr = new Memory(Long.SIZE);
					status = pcanApiLibrary.CAN_GetValue(current_PCANUSB, (byte) PCBUSBLibrary.PCAN_DEVICE_NUMBER, ptr,
							Long.SIZE);
					while (status != PCBUSBLibrary.PCAN_ERROR_OK) {
						// wait here until properly got the device number
						status = pcanApiLibrary.CAN_GetValue(current_PCANUSB, (byte) PCBUSBLibrary.PCAN_DEVICE_NUMBER,
								ptr, Long.SIZE);
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
		} else { // correctly connected

			return true;
		}

	}

	public boolean disconnect() {
		pcanApiLibrary.CAN_Uninitialize(current_PCANUSB);
		return true;
	}
}