package totem.can;

import peak.can.CANif;

/**
 * 
 * @author Agustin Tena
 * @mail atena@boxfish.studio
 * @date 11.11.2019
 * 
 *       Hardware Abstraction Layer for Totem over CAN
 *
 */
public class TotemCAN implements Runnable {

	CANif canif;
	Thread t;

	public TotemCAN(CANif cif) {
		canif = cif;

		t = new Thread(this);
		t.start();
	}

	public void run() {

		byte[] data = new byte[Defs.BASE_COMMAND_SIZE];

		while (true) {

			try {
				Thread.sleep(Defs.SERVO_MSG_PERIOD_MS);
			} catch (InterruptedException ex) {
				Thread.currentThread().interrupt();
			}

			data[0] = (byte) 111;

			System.out.println("CAN Message sent");
			canif.write(Defs.BASE_ADDRESS, data, Defs.BASE_COMMAND_SIZE);
		}
	}

	public class Defs {

		public static final int SERVO_MSG_PERIOD_MS = 2;

		public static final int BASE_ADDRESS = 0x150;
		public static final int BASE_COMMAND_SIZE = 1;
	}
}