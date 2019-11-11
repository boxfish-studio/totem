package peak.can;

import peak.can.PCAN_OSX.*;
import peak.can.PCAN_WINDOWS.*;
import peak.can.PCAN_LINUX.*;

/**
 * 
 * @author Agustin Tena
 * @mail atena@boxfish.studio
 * @date 11.11.2019
 * 
 *       Abstract class for different architectures of PcanApi
 *
 */
public abstract class CANif {
	// Define abstract methods
	public abstract boolean connect(int device_number);

	public abstract byte[] read(int ID);

	public abstract boolean write(int ID, byte[] data, int length);

	public abstract boolean disconnect();

	public static CANif getCANif() {
		String osName = System.getProperty("os.name").toLowerCase();
		System.out.println("Operating System:" + osName);

		// Check platform to load right driver
		if (osName.contains("mac os")) {
			return PcanApiOsx.getInstance();
		} else if (osName.contains("windows")) {
			return PcanApiWindows.getInstance();
		} else if (osName.contains("linux")) {
			return PcanApiLinux.getInstance();
		}

		return null;
	}

	public void flush(int id) {
		byte[] rx = new byte[8];

		// Flush messages
		do {
			rx = this.read(id);
		} while (rx.length != 0);
	}
}
