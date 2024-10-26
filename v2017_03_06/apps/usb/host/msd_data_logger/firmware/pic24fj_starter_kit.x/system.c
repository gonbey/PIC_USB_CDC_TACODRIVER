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

#include "buttons.h"
#include "leds.h"
#include "adc.h"
#include "timer_1ms.h"

/** CONFIGURATION Bits **********************************************/
#pragma config JTAGEN = OFF
#pragma config GCP = OFF
#pragma config GWRP = OFF
#pragma config FWDTEN = OFF
#pragma config ICS = PGx2

#pragma config PLL_96MHZ = ON
#pragma config IESO = OFF
#pragma config FCKSM = CSDCMD
#pragma config OSCIOFNC = ON
#pragma config POSCMOD = HS
#pragma config FNOSC = PRIPLL
#pragma config PLLDIV = DIV3
#pragma config IOL1WAY = ON
 
/*********************************************************************
* Function: void SYS_Initialize(void)
*
* Overview: Initializes the system.
*
* PreCondition: None
*
* Input:  None
*
* Output: None
*
********************************************************************/
void SYS_Initialize()
{	
    TIMER_SetConfiguration(TIMER_CONFIGURATION_1MS);
    ADC_SetConfiguration(ADC_CONFIGURATION_DEFAULT);
    LED_Enable(LED_USB_HOST_MSD_DATA_LOGGER);
    LED_Off(LED_USB_HOST_MSD_DATA_LOGGER);
    BUTTON_Enable(BUTTON_USB_HOST_MSD_DATA_LOGGER);
}

/* Interrupt handler for USB host. */
void __attribute__((interrupt,auto_psv)) _USB1Interrupt()
{
    USB_HostInterruptHandler();
}
