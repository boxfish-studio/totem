package usb.xmodem;

public class ChecksumCalculator {

	/*************************************************************
	 * Checksum calculation algorithm
	 * (calculates CRC-16-CCIT checksum)
	 * @param data
	 * @param len
	 * @return
	 *************************************************************/
	public static int crcCalc(byte data[], int len)
	{
		int crc = 0x0;

		for (int i = 0; i < len; i ++)
		{
			crc = (((crc & 0x0000ffff) >>> 8) | ((crc & 0x0000ffff) << 8)) & 0x0000ffff;
			crc = ((crc & 0x0000ffff) ^ (data[i] & 0x00ff));
			crc = crc & 0x0000ffff;
			crc = ((crc & 0x0000ffff) ^ (((crc & 0x0000ffff) & 0xff) >>> 4))  & 0x0000ffff;
			crc = ((crc & 0x0000ffff) ^ ((crc & 0x0000ffff) << 12));
			crc = ((crc & 0x0000ffff) ^ (((crc & 0x0000ffff) & 0xff) << 5))  & 0x0000ffff;
		}
		return crc;
	}
}
