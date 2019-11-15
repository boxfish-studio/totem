package totem.demo;

import totem.Totem;

public class Main {

	public static void main(String[] args) throws java.io.IOException, InterruptedException {

		Totem totem = new Totem();
		totem.getUsbHidModel().findDeviceAttached();
		totem.get_data().get_tx().set(1, 1);

		while (true) {
			Thread.sleep(1000);
		}
	}
}
