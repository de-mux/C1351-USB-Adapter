/*
             LUFA Library
     Copyright (C) Dean Camera, 2021.

    dean [at] fourwalledcubicle [dot] com
           www.lufa-lib.org
*/

/*
    Copyright 2021  Dean Camera (dean [at] fourwalledcubicle [dot] com)

    Permission to use, copy, modify, distribute, and sell this
    software and its documentation for any purpose is hereby granted
    without fee, provided that the above copyright notice appear in
    all copies and that both that the copyright notice and this
    permission notice and warranty disclaimer appear in supporting
    documentation, and that the name of the author not be used in
    advertising or publicity pertaining to distribution of the
    software without specific, written prior permission.

    The author disclaims all warranties with regard to this
    software, including all implied warranties of merchantability
    and fitness.  In no event shall the author be liable for any
    special, indirect or consequential damages or any damages
    whatsoever resulting from loss of use, data or profits, whether
    in an action of contract, negligence or other tortious action,
    arising out of or in connection with the use or performance of
    this software.
*/

/** \file

    Main source file for the Mouse demo. This file contains the main tasks of
    the demo and is responsible for the initial application hardware configuration.
*/

#include "mouse.h"


volatile Mouse_Report mouse_report_data;
volatile bool needs_update = true;

#ifdef ENABLE_VIRTUAL_SERIAL
/** LUFA CDC Class driver interface configuration and state information. This structure is
    passed to all CDC Class driver functions, so that multiple instances of the same class
    within a device can be differentiated from one another.
*/
USB_ClassInfo_CDC_Device_t VirtualSerial_CDC_Interface = {
    .Config =
    {
        .ControlInterfaceNumber         = INTERFACE_ID_CDC_CCI,
        .DataINEndpoint                 =
        {
            .Address                = CDC_TX_EPADDR,
            .Size                   = CDC_TXRX_EPSIZE,
            .Banks                  = 1,
        },
        .DataOUTEndpoint                =
        {
            .Address                = CDC_RX_EPADDR,
            .Size                   = CDC_TXRX_EPSIZE,
            .Banks                  = 1,
        },
        .NotificationEndpoint           =
        {
            .Address                = CDC_NOTIFICATION_EPADDR,
            .Size                   = CDC_NOTIFICATION_EPSIZE,
            .Banks                  = 1,
        },
    },
};



char serial_out_str[16];
volatile bool serial_data_waiting = false;

void serialPrintNum(int16_t n)
{
    sprintf(serial_out_str, "%d\r\n", n);
    serial_data_waiting = true;
}

#endif

/** Buffer to hold the previously generated Mouse HID report, for comparison purposes inside the HID class driver. */
static uint8_t PrevMouseHIDReportBuffer[sizeof(Mouse_Report)];

/** LUFA HID Class driver interface configuration and state information. This structure is
    passed to all HID Class driver functions, so that multiple instances of the same class
    within a device can be differentiated from one another.
*/
USB_ClassInfo_HID_Device_t Mouse_HID_Interface = {
    .Config =
    {
        .InterfaceNumber          = INTERFACE_ID_Mouse,
        .ReportINEndpoint         =
        {
            .Address              = MOUSE_EPADDR,
            .Size                 = MOUSE_EPSIZE,
            .Banks                = 1,
        },
        .PrevReportINBuffer       = PrevMouseHIDReportBuffer,
        .PrevReportINBufferSize   = sizeof(PrevMouseHIDReportBuffer),
    },
};


/* Set values of USB mouse. */
void setUsbMouse(int8_t x, int8_t y, uint8_t button)
{
    if (x || x != mouse_report_data.X) {
        mouse_report_data.X = x;
        needs_update = true;
    }

    if (y || y != mouse_report_data.Y) {
        mouse_report_data.Y = y;
        needs_update = true;
    }

    if (button != mouse_report_data.Button) {
        mouse_report_data.Button = button;
        needs_update = true;
    }
}


void initMouseReportData(void)
{
    setUsbMouse(0, 0, 0);
}


void setupUsbMouse(void)
{
    initMouseReportData();
    handleUsb();

    SetupHardware();
    LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
    GlobalInterruptEnable();
}


/** Called in a loop to handle USB events. */
inline void handleUsb(void)
{
#ifdef ENABLE_VIRTUAL_SERIAL
    //char report_string[16];
    //sprintf(report_string, "%d", 42);

    if (serial_data_waiting) {
        CDC_Device_SendString(&VirtualSerial_CDC_Interface, serial_out_str);
    }

    /* Must throw away unused bytes from the host, or it will lock up while waiting for the device */
    CDC_Device_ReceiveByte(&VirtualSerial_CDC_Interface);
    CDC_Device_USBTask(&VirtualSerial_CDC_Interface);
#endif
    /*  General management task for a given HID class interface, required for the
        correct operation of the interface. This should be called frequently in
        the main program loop, before the master USB management task USB_USBTask().
    */
    HID_Device_USBTask(&Mouse_HID_Interface);
    /*  This is the main USB management task. The USB driver requires this task to be
        executed continuously when the USB system is active (device attached in host
        mode, or attached to a host in device mode) in order to manage USB communications.
        This task may be executed inside an RTOS, fast timer ISR or the main user
        application loop.
        The USB task must be serviced within 30ms while in device mode, or within
        1ms while in host mode. The task may be serviced at all times, or (for
        minimum CPU consumption):
        In device mode, it may be disabled at start-up, enabled on the firing of
        the EVENT_USB_Device_Connect() event and disabled again on the firing of
        the EVENT_USB_Device_Disconnect() event.
    */
    USB_USBTask();
}


/** Configures the board hardware and chip peripherals */
void SetupHardware(void)
{
#if (ARCH == ARCH_AVR8)
    /* Disable watchdog if enabled by bootloader/fuses */
    MCUSR &= ~(1 << WDRF);
    wdt_disable();

    /* Disable clock division */
    clock_prescale_set(clock_div_1);
#elif (ARCH == ARCH_XMEGA)
    /* Start the PLL to multiply the 2MHz RC oscillator to 32MHz and switch the CPU core to run from it */
    XMEGACLK_StartPLL(CLOCK_SRC_INT_RC2MHZ, 2000000, F_CPU);
    XMEGACLK_SetCPUClockSource(CLOCK_SRC_PLL);

    /* Start the 32MHz internal RC oscillator and start the DFLL to increase it to 48MHz using the USB SOF as a reference */
    XMEGACLK_StartInternalOscillator(CLOCK_SRC_INT_RC32MHZ);
    XMEGACLK_StartDFLL(CLOCK_SRC_INT_RC32MHZ, DFLL_REF_INT_USBSOF, F_USB);

    PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm | PMIC_HILVLEN_bm;
#endif

    /* Hardware Initialization */
    LEDs_Init();
    USB_Init();
}

/** Event handler for the library USB Connection event. */
void EVENT_USB_Device_Connect(void)
{
    LEDs_SetAllLEDs(LEDMASK_USB_ENUMERATING);
}

/** Event handler for the library USB Disconnection event. */
void EVENT_USB_Device_Disconnect(void)
{
    LEDs_SetAllLEDs(LEDMASK_USB_NOTREADY);
}

/** Event handler for the library USB Configuration Changed event. */
void EVENT_USB_Device_ConfigurationChanged(void)
{
    bool ConfigSuccess = true;

    ConfigSuccess &= HID_Device_ConfigureEndpoints(&Mouse_HID_Interface);
#ifdef ENABLE_VIRTUAL_SERIAL
    ConfigSuccess &= CDC_Device_ConfigureEndpoints(&VirtualSerial_CDC_Interface);
#endif

    USB_Device_EnableSOFEvents();

    LEDs_SetAllLEDs(ConfigSuccess ? LEDMASK_USB_READY : LEDMASK_USB_ERROR);
}

/** Event handler for the library USB Control Request reception event. */
void EVENT_USB_Device_ControlRequest(void)
{
#ifdef ENABLE_VIRTUAL_SERIAL
    CDC_Device_ProcessControlRequest(&VirtualSerial_CDC_Interface);
#endif
    HID_Device_ProcessControlRequest(&Mouse_HID_Interface);
}

/** Event handler for the USB device Start Of Frame event. */
void EVENT_USB_Device_StartOfFrame(void)
{
    HID_Device_MillisecondElapsed(&Mouse_HID_Interface);
}

/** HID class driver callback function for the creation of HID reports to the host.

    \param[in]     HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
    \param[in,out] ReportID    Report ID requested by the host if non-zero, otherwise callback should set to the generated report ID
    \param[in]     ReportType  Type of the report to create, either HID_REPORT_ITEM_In or HID_REPORT_ITEM_Feature
    \param[out]    ReportData  Pointer to a buffer where the created report should be stored
    \param[out]    ReportSize  Number of bytes written in the report (or zero if no report is to be sent)

    \return Boolean \c true to force the sending of the report(useful for devices
        that report relative movement), \c false to let the library determine
        if it needs to be sent
*/
bool CALLBACK_HID_Device_CreateHIDReport(USB_ClassInfo_HID_Device_t* const
        HIDInterfaceInfo,
        uint8_t* const ReportID,
        const uint8_t ReportType,
        void* ReportData,
        uint16_t* const ReportSize)
{
    *(Mouse_Report*)ReportData = mouse_report_data;
    *ReportSize = sizeof(Mouse_Report);

    if (needs_update) {
        needs_update = false;
        return true;
    }

    return false;
}

/** HID class driver callback function for the processing of HID reports from the host.

    \param[in] HIDInterfaceInfo  Pointer to the HID class interface configuration structure being referenced
    \param[in] ReportID    Report ID of the received report from the host
    \param[in] ReportType  The type of report that the host has sent, either HID_REPORT_ITEM_Out or HID_REPORT_ITEM_Feature
    \param[in] ReportData  Pointer to a buffer where the received report has been stored
    \param[in] ReportSize  Size in bytes of the received HID report
*/
void CALLBACK_HID_Device_ProcessHIDReport(USB_ClassInfo_HID_Device_t* const
        HIDInterfaceInfo,
        const uint8_t ReportID,
        const uint8_t ReportType,
        const void* ReportData,
        const uint16_t ReportSize)
{
    // Unused (but mandatory for the HID class driver) in this demo, since there are no Host->Device reports
}

#ifdef ENABLE_VIRTUAL_SERIAL
/** CDC class driver callback function the processing of changes to the virtual
    control lines sent from the host..

    \param[in] CDCInterfaceInfo  Pointer to the CDC class interface configuration structure being referenced
*/
void EVENT_CDC_Device_ControLineStateChanged(USB_ClassInfo_CDC_Device_t* const
        CDCInterfaceInfo)
{
    /*  You can get changes to the virtual CDC lines in this callback; a common
        use-case is to use the Data Terminal Ready (DTR) flag to enable and
        disable CDC communications in your application when set to avoid the
        application blocking while waiting for a host to become ready and read
        in the pending data from the USB endpoints.
    */
    bool HostReady = (CDCInterfaceInfo->State.ControlLineStates.HostToDevice &
                      CDC_CONTROL_LINE_OUT_DTR) != 0;

    (void)HostReady;
}
#endif
