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

#include <xc.h>
#include "system.h"
#include "usb.h"

#include "leds.h"
#include "lcd.h"
#include "print_lcd.h"

/** CONFIGURATION Bits **********************************************/
// CONFIG4
#pragma config DSWDTPS = DSWDTPS3       // DSWDT Postscale Select (1:128 (132 ms))
#pragma config DSWDTOSC = LPRC          // Deep Sleep Watchdog Timer Oscillator Select (DSWDT uses Low Power RC Oscillator (LPRC))
#pragma config RTCOSC = SOSC            // RTCC Reference Oscillator Select (RTCC uses Secondary Oscillator (SOSC))
#pragma config DSBOREN = OFF            // Deep Sleep BOR Enable bit (BOR disabled in Deep Sleep)
#pragma config DSWDTEN = OFF            // Deep Sleep Watchdog Timer (DSWDT disabled)

// CONFIG3
#pragma config WPFP = WPFP0             // Write Protection Flash Page Segment Boundary (Page 0 (0x0))
#pragma config SOSCSEL = SOSC           // Secondary Oscillator Pin Mode Select (SOSC pins in Default (high drive-strength) Oscillator Mode)
#pragma config WUTSEL = LEG             // Voltage Regulator Wake-up Time Select (Default regulator start-up time used)
#pragma config WPDIS = WPDIS            // Segment Write Protection Disable (Segmented code protection disabled)
#pragma config WPCFG = WPCFGDIS         // Write Protect Configuration Page Select (Last page and Flash Configuration words are unprotected)
#pragma config WPEND = WPENDMEM         // Segment Write Protection End Page Select (Write Protect from WPFP to the last page of memory)

// CONFIG2
#pragma config POSCMOD = HS             // Primary Oscillator Select (HS Oscillator mode selected)
#pragma config I2C1SEL = PRI            // I2C1 Pin Select bit (Use default SCL1/SDA1 pins for I2C1 )
#pragma config IOL1WAY = OFF            // IOLOCK One-Way Set Enable (The IOLOCK bit can be set and cleared using the unlock sequence)
#pragma config OSCIOFNC = ON            // OSCO Pin Configuration (OSCO pin functions as port I/O (RA3))
#pragma config FCKSM = CSDCMD           // Clock Switching and Fail-Safe Clock Monitor (Sw Disabled, Mon Disabled)
#pragma config FNOSC = PRIPLL           // Initial Oscillator Select (Primary Oscillator with PLL module (XTPLL, HSPLL, ECPLL))
#pragma config PLL96MHZ = ON            // 96MHz PLL Startup Select (96 MHz PLL Startup is enabled automatically on start-up)
#pragma config PLLDIV = DIV2            // USB 96 MHz PLL Prescaler Select (Oscillator input divided by 2 (8 MHz input))
#pragma config IESO = OFF               // Internal External Switchover (IESO mode (Two-Speed Start-up) disabled)

// CONFIG1
#pragma config WDTPS = PS1              // Watchdog Timer Postscaler (1:1)
#pragma config FWPSA = PR32             // WDT Prescaler (Prescaler ratio of 1:32)
#pragma config WINDIS = OFF             // Windowed WDT (Standard Watchdog Timer enabled,(Windowed-mode is disabled))
#pragma config FWDTEN = OFF             // Watchdog Timer (Watchdog Timer is disabled)
#pragma config ICS = PGx1               // Emulator Pin Placement Select bits (Emulator functions are shared with PGEC1/PGED1)
#pragma config GWRP = OFF               // General Segment Write Protect (Writes to program memory are allowed)
#pragma config GCP = OFF                // General Segment Code Protect (Code protection is disabled)
#pragma config JTAGEN = OFF             // JTAG Port Enable (JTAG port is disabled)

 
/*********************************************************************
* Function: void SYSTEM_Initialize( SYSTEM_STATE state )
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
void SYSTEM_Initialize( SYSTEM_STATE state )
{
    //On the PIC24FJ64GB004 Family of USB microcontrollers, the PLL will not power up and be enabled
    //by default, even if a PLL enabled oscillator configuration is selected (such as HS+PLL).
    //This allows the device to power up at a lower initial operating frequency, which can be
    //advantageous when powered from a source which is not gauranteed to be adequate for 32MHz
    //operation.  On these devices, user firmware needs to manually set the CLKDIV<PLLEN> bit to
    //power up the PLL.
    {
        unsigned int pll_startup_counter = 600;
        CLKDIVbits.PLLEN = 1;
        while(pll_startup_counter--);
    }

    switch(state)
    {
        case SYSTEM_STATE_USB_HOST:
            PRINT_SetConfiguration(PRINT_CONFIGURATION_LCD);

            break;
            
        case SYSTEM_STATE_USB_HOST_HID_KEYBOARD:
            LED_Enable(LED_USB_HOST_HID_KEYBOARD_DEVICE_READY);

            PRINT_SetConfiguration(PRINT_CONFIGURATION_LCD);
            LCD_CursorEnable(true);

            TIMER_SetConfiguration(TIMER_CONFIGURATION_1MS);

            break;
    }
}

void __attribute__((interrupt,auto_psv)) _USB1Interrupt()
{
         USB_HostInterruptHandler();
}
