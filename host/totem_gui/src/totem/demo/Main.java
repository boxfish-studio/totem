package totem.demo;

import totem.Totem;

public class Main {

	public static void main(String[] args) throws java.io.IOException, InterruptedException {

		Totem totem = new Totem();
		totem.getUsbHidModel().findDeviceAttached();
		totem.get_eol_shield().get_tx().set(1, 1);

		while (true) {
			//totem.get_eol_shield().print();
			Thread.sleep(1000);
		}
	}
}
