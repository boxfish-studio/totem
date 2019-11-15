package totem.can.demo;

import peak.can.CANif;
import totem.can.TotemCAN;

/**
 * 
 * @author Agustin Tena
 * @mail atena@boxfish.studio
 * @date 11.11.2019
 * 
 *       Demo app to show the interface to totem over CAN
 *
 */
public class Main {

	public static void main(String[] args) throws java.io.IOException, InterruptedException {

		CANif canif = CANif.getCANif();
		canif.connect(-1);

		TotemCAN totem = new TotemCAN(canif);

		while (true) {

			Thread.sleep(1000);
		}
	}
}