/*******************************************************************************
Copyright 2016 Microchip Technology Inc. (www.microchip.com)

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

To request to license the code under the MLA license (www.microchip.com/mla_license), 
please contact mla_licensing@microchip.com
*******************************************************************************/

#include "system.h"

#include "usb.h"
#include "fileio.h"
#include "sd_spi.h"

/** CONFIGURATION Bits **********************************************/
#pragma config WDTPS = PS1
#pragma config FWPSA = PR32
#pragma config WINDIS = OFF
#pragma config FWDTEN = OFF
#pragma config ICS = PGx1
#pragma config GWRP = OFF
#pragma config GCP = OFF
#pragma config JTAGEN = OFF

#pragma config POSCMOD = HS
#pragma config IOL1WAY = OFF
#pragma config OSCIOFNC = ON
#pragma config FCKSM = CSDCMD
#pragma config FNOSC = PRIPLL
#pragma config PLL96MHZ = ON
#pragma config PLLDIV = DIV2
#pragma config IESO = OFF

#pragma config WPFP = WPFP0
#pragma config SOSCSEL = SOSC
#pragma config WUTSEL = LEG
#pragma config WPDIS = WPDIS
#pragma config WPCFG = WPCFGDIS
#pragma config WPEND = WPENDMEM

#pragma config DSWDTPS = DSWDTPS3
#pragma config DSWDTOSC = LPRC
#pragma config RTCOSC = SOSC
#pragma config DSBOREN = OFF
#pragma config DSWDTEN = OFF

extern FILEIO_SD_DRIVE_CONFIG sdCardMediaParameters;

/*********************************************************************
* Function: void SYSTEM_Initialize(void)
*
* Overview: Initializes the system.
*
* PreCondition: None
*
* Input:  SYSTEM_STATE - the state to initialize the system into
*
* Output: None
*
********************************************************************/
void SYSTEM_Initialize(void)
{
    unsigned int pll_startup_counter = 600;

    AD1PCFGL = 0xFFFF;

    //On the PIC24FJ64GB004 Family of USB microcontrollers, the PLL will not power up and be enabled
    //by default, even if a PLL enabled oscillator configuration is selected (such as HS+PLL).
    //This allows the device to power up at a lower initial operating frequency, which can be
    //advantageous when powered from a source which is not gauranteed to be adequate for 32MHz
    //operation.  On these devices, user firmware needs to manually set the CLKDIV<PLLEN> bit to
    //power up the PLL.
    CLKDIVbits.PLLEN = 1;
    while(pll_startup_counter--);

    //Device switches over automatically to PLL output after PLL is locked and ready.


//	The USB specifications require that USB peripheral devices must never source
//	current onto the Vbus pin.  Additionally, USB peripherals should not source
//	current on D+ or D- when the host/hub is not actively powering the Vbus line.
//	When designing a self powered (as opposed to bus powered) USB peripheral
//	device, the firmware should make sure not to turn on the USB module and D+
//	or D- pull up resistor unless Vbus is actively powered.  Therefore, the
//	firmware needs some means to detect when Vbus is being powered by the host.
//	A 5V tolerant I/O pin can be connected to Vbus (through a resistor), and
// 	can be used to detect when Vbus is high (host actively powering), or low
//	(host is shut down or otherwise not supplying power).  The USB firmware
// 	can then periodically poll this I/O pin to know when it is okay to turn on
//	the USB module/D+/D- pull up resistor.  When designing a purely bus powered
//	peripheral device, it is not possible to source current on D+ or D- when the
//	host is not actively providing power on Vbus. Therefore, implementing this
//	bus sense feature is optional.  This firmware can be made to use this bus
//	sense feature by making sure "USE_USB_BUS_SENSE_IO" has been defined in the
//	HardwareProfile.h file.
    #if defined(USE_USB_BUS_SENSE_IO)
    tris_usb_bus_sense = INPUT_PIN;
    #endif

//	If the host PC sends a GetStatus (device) request, the firmware must respond
//	and let the host know if the USB peripheral device is currently bus powered
//	or self powered.  See chapter 9 in the official USB specifications for details
//	regarding this request.  If the peripheral device is capable of being both
//	self and bus powered, it should not return a hard coded value for this request.
//	Instead, firmware should check if it is currently self or bus powered, and
//	respond accordingly.  If the hardware has been configured like demonstrated
//	on the PICDEM FS USB Demo Board, an I/O pin can be polled to determine the
//	currently selected power source.  On the PICDEM FS USB Demo Board, "RA2"
//	is used for	this purpose.  If using this feature, make sure "USE_SELF_POWER_SENSE_IO"
//	has been defined in HardwareProfile.h, and that an appropriate I/O pin has been mapped
//	to it in HardwareProfile.h.
    #if defined(USE_SELF_POWER_SENSE_IO)
    tris_self_power = INPUT_PIN;
    #endif

    //********* Initialize Peripheral Pin Select (PPS) *************************
    //  This section only pertains to devices that have the PPS capabilities.
    //    When migrating code into an application, please verify that the PPS
    //    setting is correct for the port pins that are used in the application.
    //Initialize the SPI
    RPINR20bits.SDI1R = 17;     //MSDI
    RPOR8bits.RP16R = 7;        //MSDO
    RPOR7bits.RP15R = 8;        //SCK

    //enable a pull-up for the card detect, just in case the SD-Card isn't attached
    //  then lets have a pull-up to make sure we don't think it is there.
    CNPU1bits.CN6PUE = 1;

    FILEIO_SD_IOInitialize(&sdCardMediaParameters);

    USBDeviceInit();	//usb_device.c.  Initializes USB module SFRs and firmware
    					//variables to known states.
}

void USER_SdSpiConfigurePins_1 (void)
{
    // Deassert the chip select pin
    LATAbits.LATA1 = 1;
    // Configure CS pin as an output
    TRISAbits.TRISA1 = 0;
    // Configure CD pin as an input
    TRISBbits.TRISB2 = 1;
    // Configure WP pin as an input
    TRISBbits.TRISB3 = 1;
}

inline void USER_SdSpiSetCs_1(uint8_t a)
{
    LATAbits.LATA1 = a;
}

inline bool USER_SdSpiGetCd_1(void)
{
    return (!PORTBbits.RB2) ? true : false;
}

inline bool USER_SdSpiGetWp_1(void)
{
    return (PORTBbits.RB3) ? true : false;
}

#if defined(USB_INTERRUPT)
void __attribute__((interrupt,auto_psv)) _USB1Interrupt()
{
    USBDeviceTasks();
}
#endif

