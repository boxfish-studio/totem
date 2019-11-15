package peak.can.PCAN_WINDOWS.jna.structures;
import com.sun.jna.Pointer;
import com.sun.jna.Structure;
import java.util.Arrays;
import java.util.List;
/**
 * This file was autogenerated by <a href="http://jnaerator.googlecode.com/">JNAerator</a>,<br>
 * a tool written by <a href="http://ochafik.com/">Olivier Chafik</a> that <a href="http://code.google.com/p/jnaerator/wiki/CreditsAndLicense">uses a few opensource projects.</a>.<br>
 * For help, please visit <a href="http://nativelibs4java.googlecode.com/">NativeLibs4Java</a> , <a href="http://rococoa.dev.java.net/">Rococoa</a>, or <a href="http://jna.dev.java.net/">JNA</a>.
 */
public class tagTPCANTimestamp extends Structure {
	/** Base-value: milliseconds: 0.. 2^32-1 */
	public int millis;
	/** Roll-arounds of millis */
	public short millis_overflow;
	/** Microseconds: 0..999 */
	public short micros;
	public tagTPCANTimestamp() {
		super();
	}
	protected List<? > getFieldOrder() {
		return Arrays.asList("millis", "millis_overflow", "micros");
	}
	/**
	 * @param millis Base-value: milliseconds: 0.. 2^32-1<br>
	 * @param millis_overflow Roll-arounds of millis<br>
	 * @param micros Microseconds: 0..999
	 */
	public tagTPCANTimestamp(int millis, short millis_overflow, short micros) {
		super();
		this.millis = millis;
		this.millis_overflow = millis_overflow;
		this.micros = micros;
	}
	public tagTPCANTimestamp(Pointer peer) {
		super(peer);
	}
	public static class ByReference extends tagTPCANTimestamp implements Structure.ByReference {

	};
	public static class ByValue extends tagTPCANTimestamp implements Structure.ByValue {

	};
}
