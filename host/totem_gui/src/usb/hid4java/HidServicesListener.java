package usb.hid4java;

import java.util.EventListener;

import usb.hid4java.event.HidServicesEvent;

/**
 * <p>Interface to provide the following to API consumers:</p>
 * <ul>
 * <li>Notification of a HID event</li>
 * </ul>
 *
 * @since 0.0.1
 *  
 */
public interface HidServicesListener extends EventListener {
  /**
   * A HID device was attached
   *
   * @param event The event
   */
  public void hidDeviceAttached(HidServicesEvent event);

  /**
   * A HID device was detached
   *
   * @param event The event
   */
  public void hidDeviceDetached(HidServicesEvent event);

  /**
   * A HID failure occurred during scanning
   *
   * @param event The event
   */
  public void hidFailure(HidServicesEvent event);

}