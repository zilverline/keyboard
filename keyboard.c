/*
   The MIT License (MIT)

   Copyright (c) 2013 Peter Rebholz

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
   */

#include "lufa_config.h"
#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/power.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "LUFA/Drivers/USB/USB.h"
#include "keyboard.h"

/** Indicates what report mode the host has requested, true for normal HID reporting mode, \c false for special boot
 *  *  protocol reporting mode.
 *   */
static bool UsingReportProtocol = true;

/** Current Idle period. This is set by the host via a Set Idle HID class request to silence the device's reports
 *  *  for either the entire idle duration, or until the report status changes (e.g. the user presses a key).
 *   */
static uint16_t IdleCount = 500;

/** Current Idle period remaining. When the IdleCount value is set, this tracks the remaining number of idle
 *  *  milliseconds. This is separate to the IdleCount timer and is incremented and compared as the host may request
 *   *  the current idle period via a Get Idle HID class request, thus its value must be preserved.
 *    */
static uint16_t IdleMSRemaining = 0;

int main (void) {
  SetupHardware();
  GlobalInterruptEnable();

  int x = 0;
  for(;;) {
    /* HID_Task(); */
    /* USB_USBTask(); */
    x++;
    if(x % (int) 5e8 < 2.5e8) PORTD ^= (1 << 7);
  }

  return 0;
}

void SetupHardware () {
  /* Disable watchdog if enabled by bootloader/fuses */
  MCUSR &= ~(1 << WDRF);
  wdt_disable();

  /* Disable clock division */
  clock_prescale_set(clock_div_1);

  USB_Init();
  Buttons_Init();
}

void Buttons_Init() {
}

void HID_Task() {
  if(USB_DeviceState != DEVICE_STATE_Configured) { return; }
  static USB_KeyboardReport_Data_t status;

  memset(&status, 0, sizeof(USB_KeyboardReport_Data_t));

  status.Modifier = HID_KEYBOARD_MODIFIER_LEFTSHIFT;
  status.KeyCode[0] = HID_KEYBOARD_SC_Z;

  Endpoint_SelectEndpoint(KEYBOARD_IN_EPADDR);

  if (Endpoint_IsReadWriteAllowed()) {
    /* Write Keyboard Report Data */
    Endpoint_Write_Stream_LE(&status, sizeof(status), NULL);

    /* Finalize the stream transfer to send the last packet */
    Endpoint_ClearIN();
  }
}

/** Event handler for the USB_Connect event. This indicates that the device is enumerating via the status LEDs and
 *  *  starts the library USB task to begin the enumeration and USB management process.
 *   */
void EVENT_USB_Device_Connect(void)
{
  /* Default to report protocol on connect */
  UsingReportProtocol = true;
}

/** Event handler for the USB_Disconnect event. This indicates that the device is no longer connected to a host via
 *  *  the status LEDs.
 *   */
void EVENT_USB_Device_Disconnect(void)
{
}

/** Event handler for the USB_ConfigurationChanged event. This is fired when the host sets the current configuration
 *  *  of the USB device after enumeration, and configures the keyboard device endpoints.
 *   */
void EVENT_USB_Device_ConfigurationChanged(void)
{
  /* Setup HID Report Endpoints */
  Endpoint_ConfigureEndpoint(KEYBOARD_IN_EPADDR, EP_TYPE_INTERRUPT, KEYBOARD_EPSIZE, 1);

  /* Turn on Start-of-Frame events for tracking HID report period expiry */
  USB_Device_EnableSOFEvents();
}

/** Event handler for the USB_ControlRequest event. This is used to catch and process control requests sent to
 *  *  the device from the USB host before passing along unhandled control requests to the library for processing
 *   *  internally.
 *    */
void EVENT_USB_Device_ControlRequest(void)
{
  /* Handle HID Class specific requests */
  switch (USB_ControlRequest.bRequest)
  {
    case HID_REQ_GetReport:
      /* if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE)) */
      /* { */
      /*   USB_KeyboardReport_Data_t KeyboardReportData; */

      /*   /1* Create the next keyboard report for transmission to the host *1/ */
      /*   CreateKeyboardReport(&KeyboardReportData); */

      /*   Endpoint_ClearSETUP(); */

      /*   /1* Write the report data to the control endpoint *1/ */
      /*   Endpoint_Write_Control_Stream_LE(&KeyboardReportData, sizeof(KeyboardReportData)); */
      /*   Endpoint_ClearOUT(); */
      /* } */

      break;
    case HID_REQ_SetReport:
      break;
    case HID_REQ_GetProtocol:
      if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
      {
        Endpoint_ClearSETUP();

        /* Write the current protocol flag to the host */
        Endpoint_Write_8(UsingReportProtocol);

        Endpoint_ClearIN();
        Endpoint_ClearStatusStage();
      }

      break;
    case HID_REQ_SetProtocol:
      if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
      {
        Endpoint_ClearSETUP();
        Endpoint_ClearStatusStage();

        /* Set or clear the flag depending on what the host indicates that the current Protocol should be */
        UsingReportProtocol = (USB_ControlRequest.wValue != 0);
      }

      break;
    case HID_REQ_SetIdle:
      if (USB_ControlRequest.bmRequestType == (REQDIR_HOSTTODEVICE | REQTYPE_CLASS | REQREC_INTERFACE))
      {
        Endpoint_ClearSETUP();
        Endpoint_ClearStatusStage();

        /* Get idle period in MSB, IdleCount must be multiplied by 4 to get number of milliseconds */
        IdleCount = ((USB_ControlRequest.wValue & 0xFF00) >> 6);
      }

      break;
    case HID_REQ_GetIdle:
      if (USB_ControlRequest.bmRequestType == (REQDIR_DEVICETOHOST | REQTYPE_CLASS | REQREC_INTERFACE))
      {
        Endpoint_ClearSETUP();

        /* Write the current idle duration to the host, must be divided by 4 before sent to host */
        Endpoint_Write_8(IdleCount >> 2);

        Endpoint_ClearIN();
        Endpoint_ClearStatusStage();
      }

      break;
  }
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
  /* One millisecond has elapsed, decrement the idle time remaining counter if it has not already elapsed */
  if (IdleMSRemaining)
    IdleMSRemaining--;
}

bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
    uint8_t* const ReportID,
    const uint8_t ReportType,
    void* ReportData,
    uint16_t* const ReportSize) { return true; }

void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const HIDInterfaceInfo,
    const uint8_t ReportID,
    const uint8_t ReportType,
    const void* ReportData,
    const uint16_t ReportSize) {}

bool CALLBACK_HIDParser_FilterHIDReportItem(HID_ReportItem_t* const CurrentItem) { return true; }
