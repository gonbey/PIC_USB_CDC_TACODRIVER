/*******************************************************************************
  System Specific Definitions

  Company:
    Microchip Technology Inc.

  File Name:
    system_config.h

  Summary:
    System level definitions for the specific Microchip Development Board used.

*******************************************************************************/

// DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) 2013 released Microchip Technology Inc.  All rights reserved.

Microchip licenses to you the right to use, modify, copy and distribute
Software only when embedded on a Microchip microcontroller or digital signal
controller that is integrated into your product or third party product
(pursuant to the sublicense terms in the accompanying license agreement).

You should refer to the license agreement accompanying this Software for
additional information regarding your rights and obligations.

SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND,
EITHER EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY, TITLE, NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE.
IN NO EVENT SHALL MICROCHIP OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER
CONTRACT, NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR
OTHER LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE OR
CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT OF
SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.
*******************************************************************************/
// DOM-IGNORE-END

#ifndef __SYSTEM_CONFIG_H
#define __SYSTEM_CONFIG_H

/*********************************************************************
* PIC Device Specific includes
*********************************************************************/
#include "gfx_config.h"
#include "memory_programmer/usb_config.h"

#define USE_8BIT_PMP
#define GFX_USE_DISPLAY_CONTROLLER_ILI9341

/*********************************************************************
 The following hardware is used in this system config:
 1. mikromedia for PIC24
    http://www.mikroe.com/mikromedia/pic24/
*********************************************************************/

/*********************************************************************
* DISPLAY PARAMETERS
*********************************************************************/
/*
    The following are the parameters required for the
    different display drivers that is supported.
    When adding support to a new display controller or display panel,
    follow the parameterization of the driver for reusability of the
    driver.

    Display Parameter Macros Descriptions:

    1. DISP_ORIENTATION - defines the display rotation with
            respect to its native orientation. For example, if the display
            has a resolution specifications that says 240x320 (QVGA), the
            display is natively in portrait mode. If the application
            uses the display in landscape mode (320x240), then the
            orientation musts be defined as 90 or 180 degree rotation.
            The Graphics Library will calculate the actual pixel location
            to rotate the contents of the screen. So when users view the
            display, the image on the screen will come out in the correct
            orientation.

            Valid values: 0, 90, 180 and 270
            Example: #define DISP_ORIENTATION 90

    2. DISP_HOR_RESOLUTION - defines the horizontal dimension in pixels. This
            is the native horizontal dimension of the screen. In the example
            given in DISP_ORIENTATION, a 320x240 display will have DISP_HOR_RESOLUTION
            of 320.

            Valid Values: dependent on the display glass resolution used.
            Example: #define DISP_HOR_RESOLUTION 320

    3. DISP_VER_RESOLUTION - defines the vertical dimension in pixels. This
            is the native vertical dimension of the screen. In the example
            given in DISP_ORIENTATION, a 320x240 display will have DISP_VER_RESOLUTION
            of 240.

            Valid Values: dependent on the display glass resolution used.
            Example: #define DISP_VER_RESOLUTION 240

    4. COLOR_DEPTH - (NOTE: Define this macro in the gfx_config.h
             This defines the vertical dimension in pixels.

            Valid Values: 1, 4, 8, 16, 24 (note 24 bpp is only used if you are
                          using a Display Driver that supports 24 bpp color depth.
            Example: #define COLOR_DEPTH 16

    5. DISP_DATA_WIDTH - this defines the display controller physical interface
            to the display panel.

            Valid Values: 1, 4, 8, 16, 18, 24
                          1, 4, 8 are usually used in MSTN and CSTN displays
                          16, 18 and 24 are usually used in TFT displays.
            Example: #define DISP_DATA_WIDTH 18

    6. DISP_INV_LSHIFT - this indicates that the color data is sampled in the
            falling edge of the pixel clock.

            Example: #define DISP_INV_LSHIFT - define this to sample the
                          color data in the falling edge of the pixel clock.

    7. DISP_HOR_PULSE_WIDTH - this defines the horizontal sync signal pulse width.

            Valid Values: See display panel data sheet
            Example: #define DISP_HOR_PULSE_WIDTH 25

    8. DISP_VER_PULSE_WIDTH - this defines the vertical sync signal pulse width.

            Valid Values: See display panel data sheet
            Example: #define DISP_VER_PULSE_WIDTH 4

    9. DISP_HOR_BACK_PORCH - this defines the horizontal back porch.
            DISP_HOR_BACK_PORCH + DISP_HOR_FRONT_PORCH + DISP_HOR_PULSE_WIDTH
            makes up the horizontal blanking period.

            Valid Values: See display panel data sheet
            Example: #define DISP_HOR_BACK_PORCH 5

    10. DISP_HOR_FRONT_PORCH - this defines the horizontal front porch.
            DISP_HOR_BACK_PORCH + DISP_HOR_FRONT_PORCH + DISP_HOR_PULSE_WIDTH
            makes up the horizontal blanking period.

            Valid Values: See display panel data sheet
            Example: #define DISP_HOR_FRONT_PORCH 10

    11. DISP_VER_BACK_PORCH - this defines the vertical back porch.
            DISP_VER_BACK_PORCH + DISP_VER_FRONT_PORCH + DISP_VER_PULSE_WIDTH
            makes up the horizontal blanking period.

            Valid Values: See display panel data sheet
            Example: #define DISP_VER_BACK_PORCH 0

    12. DISP_VER_FRONT_PORCH - this defines the horizontal front porch.
            DISP_VER_BACK_PORCH + DISP_VER_FRONT_PORCH + DISP_VER_PULSE_WIDTH
            makes up the horizontal blanking period.

            Valid Values: See display panel data sheet
            Example: #define DISP_VER_FRONT_PORCH 2

    13. GFX_LCD_TYPE - this defines the type of display glass used.

            Valid Values: Dependent on the display controller supported LCD types.
                          GFX_LCD_TFT  // Type TFT Display
                          GFX_LCD_CSTN // Type Color STN Display
                          GFX_LCD_MSTN // Type Mono STN Display
                          GFX_LCD_OLED // Type OLED Display

            Example: #define GFX_LCD_TYPE GFX_LCD_TFT

    14. GFX_DISPLAYENABLE_ENABLE - this enables the use of the Display Enable Port
            (GEN) when using the Microchip Graphics Module. The signal required when
            using a display panel that supports DATA ENABLE Mode. DATA ENABLE Mode
            is used when the display panel do not have sync (HSYNC and VSYNC) signals.

            Example: #define GFX_DISPLAYENABLE_ENABLE

    15. GFX_DISPLAYENABLE_POLARITY - this sets the polarity of the Display Enable Port
            (GEN) when using the Microchip Graphics Module.

            Valid Values: GFX_ACTIVE_HIGH, GFX_ACTIVE_LOW
            Example: #define GFX_DISPLAYENABLE_POLARITY GFX_ACTIVE_HIGH

    16. GFX_HSYNC_ENABLE - this enables the use of the Display Horizontal Synchronization
            Port (HSYNC) when using the Microchip Graphics Module. The signal required when
            using a display panel that supports SYNC Mode. SYNC Mode
            is used when the display panel has sync (HSYNC and VSYNC) signals.

            Example: #define GFX_HSYNC_ENABLE

    17. GFX_HSYNC_POLARITY - this sets the polarity of the Display Horizontal Synchronization
            Port (HSYNC)when using the Microchip Graphics Module.

            Valid Values: GFX_ACTIVE_HIGH, GFX_ACTIVE_LOW
            Example: #define GFX_HSYNC_POLARITY GFX_ACTIVE_LOW

    18. GFX_VSYNC_ENABLE - this enables the use of the Display Vertical Synchronization
            Port (VSYNC) when using the Microchip Graphics Module. The signal required when
            using a display panel that supports SYNC Mode. SYNC Mode
            is used when the display panel has sync (HSYNC and VSYNC) signals.

            Example: #define GFX_VSYNC_ENABLE

    19. GFX_VSYNC_POLARITY - this sets the polarity of the Display Vertical Synchronization
            Port (VSYNC)when using the Microchip Graphics Module.

            Valid Values: GFX_ACTIVE_HIGH, GFX_ACTIVE_LOW
            Example: #define GFX_VSYNC_POLARITY GFX_ACTIVE_LOW

    20. GFX_DISPLAYPOWER_ENABLE - this enables the use of the Display Power Sequencer
            Control Port (GPWR) when using the Microchip Graphics Module. The signal
            can be used to control the display power circuitry. The Graphics Module
            can manage the powering up and powering down of the display since
            power up and power down sequence in display panels is essential to
            extend display life cycle.

            Example: #define GFX_DISPLAYPOWER_ENABLE

    21. GFX_DISPLAYPOWER_POLARITY - this sets the polarity of the Display Power Sequencer
            Control Port (GPWR) when using the Microchip Graphics Module.

            Valid Values: GFX_ACTIVE_HIGH, GFX_ACTIVE_LOW
            Example: #define GFX_DISPLAYPOWER_POLARITY GFX_ACTIVE_HIGH

    22. GFX_CLOCK_POLARITY - this sets the polarity of the Display Glass Clock Port (GCLK)
            when using the Microchip Graphics Module.

            Valid Values: GFX_ACTIVE_HIGH, GFX_ACTIVE_LOW
            Example: #define GFX_CLOCK_POLARITY GFX_ACTIVE_HIGH


    There are two major types of controllers enumerated here:
    1. Controllers in Smart Displays - these are display modules that have
       built in display controller. Interface to the PIC device(s)
       are usually through parallel interface or serial interface.

       Required parameters:
       #define DISP_ORIENTATION
       #define DISP_HOR_RESOLUTION
       #define DISP_VER_RESOLUTION

    2. Controller that drives the RGB Glass directly - these are display
       controllers that comes in a separate package or built into the
       microcontrollers.

       Required parameters:
       #define DISP_ORIENTATION
       #define DISP_HOR_RESOLUTION
       #define DISP_VER_RESOLUTION
       #define DISP_DATA_WIDTH
       #define DISP_HOR_PULSE_WIDTH
       #define DISP_HOR_BACK_PORCH
       #define DISP_HOR_FRONT_PORCH
       #define DISP_VER_PULSE_WIDTH
       #define DISP_VER_BACK_PORCH
       #define DISP_VER_FRONT_PORCH

       Optional parameters:
       #define DISP_INV_LSHIFT

       When using Microchip Graphics Module (mchpGfxDrv) required parameters:
       #define GFX_DISPLAYENABLE_ENABLE
       #define GFX_DISPLAYENABLE_POLARITY

       or

       #define GFX_HSYNC_ENABLE
       #define GFX_VSYNC_ENABLE
       #define GFX_HSYNC_POLARITY
       #define GFX_VSYNC_POLARITY

       #define GFX_CLOCK_POLARITY

       When using Microchip Graphics Module (mchpGfxDrv) Optional parameters:
       #define GFX_DISPLAYPOWER_ENABLE
       #define GFX_DISPLAYPOWER_POLARITY


*/
// -----------------------------------
// For Smart GLASS
// -----------------------------------
// Using Ilitek 9341 Smart Glass

#define DISP_ORIENTATION    90
#define DISP_HOR_RESOLUTION 240
#define DISP_VER_RESOLUTION 320

// -----------------------------------
// Default font to be used in the demo
// -----------------------------------
#define APP_DEMO_FONT Gentium27

/*********************************************************************
* IOS FOR THE SWITCHES (SIDE BUTTONS)
*********************************************************************/
#define HardwareButtonInit()
#define GetHWButtonProgram()                0//(PORTEbits.RE9)
#define GetHWButtonUseInternalResource()    0//(PORTGbits.RG8)
#define GetHWButtonScanDown()               0//(HW_BUTTON_RELEASE)
#define GetHWButtonScanUp()                 0//(HW_BUTTON_RELEASE)
#define GetHWButtonCR()                     0//(PORTGbits.RG8)
#define GetHWButtonFocus()                  0//(PORTEbits.RE9)
#define GetHWButtonLeft()                   0//(PORTGbits.RG8)
#define GetHWButtonCenter()                 0//(PORTEbits.RE9)
#define GetHWButtonRight()                  0//(PORTBbits.RB5)

/*********************************************************************
* DISPLAY PARALLEL INTERFACE
*********************************************************************/
/*
   Select the correct Parallel Master Port Interface Driver.
   This selection is valid only for parallel interface. When using
   display drivers that uses serial interface, this portion should be
   commented out or removed.
*/
#define USE_GFX_PMP

/*********************************************************************
* PARALLEL MASTER PORT INTERFACE TIMING
*********************************************************************/
	// When using the Parallel Master Port (or Enhanced Parallel Master Port) the
	// PMP timing must be defined for the following:
	// PMP_DATA_SETUP_TIME or EPMPCSx_DATA_SETUP (for DWAITB)
	// PMP_DATA_WAIT_TIME or EPMPCSx_DATA_WAIT_TIME (for DWAITM)
	// and PMP_DATA_HOLD_TIME or EPMPCSx_DATA_HOLD_TIME (for DWAITE)
    // where x is 1 or 2 (EPMP has 2 chip selects)
	// All values are timing parameters in ns.
	// The timing is dependent on the display controller sampling period if
	// interfacing with the display controller or device AC timing requirements
	// if interfacing with a device (such as SRAM or Parallel Flash memory).
	// Refer to your specific display controller or device to determine the
	// optimum (fastest) timing requirement for your interface.

/*********************************************************************
* PARALLEL MASTER PORT INTERFACE TIMING AND POLARITIES
*********************************************************************/
/*
    This portion sets up the EPMP port timing when Graphics Module is
    used as a Alternate Master in EPMP (See Family Reference Manual (FRM)
    Section 42. Enhanced Parallel Master Port (EPMP)) for details.

    Macros to define the access timing of the parallel device in EPMP chip
	selects.  This value is used to calculate the AMWAIT cycles needed in EPMP
	Alternate Master I/Os direct access (EPMP bypass mode) (MSTSEL = 0x3).
	When in this mode the Alternate Master (in this case Graphics Module)
	will use the clock specified by the G1CLKSEL bits of CLKDIV register
	in the oscillator module. There are two possible values: 48 Mhz and
	96 Mhz. The equation used is:
	    temp = (1000000000ul) / ((CLKDIVbits.G1CLKSEL == 1)? 960000000ul : 480000000);

		if (EPMPCSx_ACCESS_TIME <= (temp*3))
		    PMCSxMDbits.AMWAIT = 0;
		else if (EPMPCSx_ACCESS_TIME > temp)
	    {
            PMCSxMDbits.AMWAIT = (EPMPCSx_ACCESS_TIME / temp);
            if ((EPMPCSx_ACCESS_TIME % temp) > 0)
                PMCSxMDbits.AMWAIT += 1;
        }

        where:
            x = 1 for CS1
            x = 2 for CS2
            the factor of 3 is used because of the fact that an AMWAIT of 0 means
            that 3 alternate master cycles are used per read/write.
            Actual cycles used is 3 + AMWAIT cycles per read/write access.

*/

// based on the ILI9341 data sheet
#define PMP_DATA_SETUP_TIME       (10)
#define PMP_DATA_WAIT_TIME        (45)
#define PMP_DATA_HOLD_TIME        (10)

/*********************************************************************
* HARDWARE PROFILE FOR DISPLAY CONTROLLER INTERFACE
*********************************************************************/
/*
    The following defines the different IO signals used to connect
    to the display controller.
    The hardware profile used is dependent on the PIC device
    and the development board used.

    The following are used to control the following signals to the
    Display Controller:
    1. Reset Signal - this signal controls the reset pin of the display
            controller if available.

            Example:
            #define DisplayResetConfig()        TRISAbits.TRISA4 = 0
            #define DisplayResetEnable()        LATAbits.LATA4 = 0
            #define DisplayResetDisable()       LATAbits.LATA4 = 1

    2. Command/Data Signal - this signal indicates to the controller
            if the command or data is being sent to the display controller.

            Example:
            #define DisplayCmdDataConfig()      TRISAbits.TRISA1 = 0
            #define DisplaySetCommand()         LATAbits.LATA1 = 0
            #define DisplaySetData()            LATAbits.LATA1 = 1

    3. Controller Chip Select Signal - this signal controls the chip select
            pin of the display controller is available.

            Example:
            #define DisplayConfig()             TRISBbits.TRISB15 = 0
            #define DisplayEnable()             LATBbits.LATB15 = 0
            #define DisplayDisable()            LATBbits.LATB15 = 1

    4. Display Module Power Control Signal - this signal turns on or off
            the display module if available. In some cases, this can be an
            IO port that will enable the power circuitry of your
            display module.

            Example:
            #define DisplayPowerConfig()        TRISAbits.TRISA5 = 0
            #define DisplayPowerOn()            LATAbits.LATA5 = 1
            #define DisplayPowerOff()           LATAbits.LATA5 = 0

    5. Display Backlight Brightness Control - this controls the backlight
            brightness or ON or OFF states if available. If brightness is
            controllable, this signal may be driven by some PWM signal.

            Example:
            #define DisplayBacklightConfig()    TRISDbits.TRISD0 = 0
            #define DisplayBacklightOn()        LATDbits.LATD0 = 0
            #define DisplayBacklightOff()       LATDbits.LATD0 = 1

*/

    // Depending on the display panel used, the backlight control may be
    // different. This portion sets up the correct backlight level to
    // turn on or off the backlight.
#define BACKLIGHT_ENABLE_LEVEL      1
#define BACKLIGHT_DISABLE_LEVEL     0

// Definitions for reset pin
#define DisplayResetConfig()        TRISCbits.TRISC1 = 0
#define DisplayResetEnable()        LATCbits.LATC1 = 0
#define DisplayResetDisable()       LATCbits.LATC1 = 1

// Definitions for RS pin
#define DisplayCmdDataConfig()      TRISBbits.TRISB15 = 0
#define DisplaySetCommand()         LATBbits.LATB15 = 0
#define DisplaySetData()            LATBbits.LATB15 = 1

// Definitions for CS pin
#define DisplayConfig()             TRISFbits.TRISF12 = 0
#define DisplayEnable()             LATFbits.LATF12 = 0
#define DisplayDisable()            LATFbits.LATF12 = 1


// Definitions for FLASH CS pin
#define DisplayFlashConfig()
#define DisplayFlashEnable()
#define DisplayFlashDisable()

// Definitions for POWER ON pin
#define DisplayPowerConfig()
#define DisplayPowerOn()
#define DisplayPowerOff()

// Definitions for backlight control pin
#define DisplayBacklightConfig()    (TRISDbits.TRISD2 = 0)
#define DisplayBacklightOn()        (LATDbits.LATD2 = BACKLIGHT_ENABLE_LEVEL)
#define DisplayBacklightOff()       (LATDbits.LATD2 = BACKLIGHT_DISABLE_LEVEL)


/*********************************************************************
* IOS FOR External Flash Memory
*********************************************************************/

// define the SPI channel used
#define DRV_SPI_CONFIG_CHANNEL_2_ENABLE

/*********************************************************************
* Macros for the SPI Flash Memory enable signal
*********************************************************************/
#define DRV_NVM_M25P80_ChipSelectEnable()   (LATCbits.LATC2 = 0)
#define DRV_NVM_M25P80_ChipSelectDisable()  (LATCbits.LATC2 = 1)

/*********************************************************************
* IOS FOR THE RESISTIVE TOUCHSCREEN
*********************************************************************/
// definitions for resistive touch screen
// ADC Input Channel Selector
#define RESISTIVETOUCH_ADC_INPUT_SEL        AD1CHS
// ADC Sample Start
#define RESISTIVETOUCH_ADC_START            AD1CON1bits.SAMP
// ADC Status
#define RESISTIVETOUCH_ADC_DONE             AD1CON1bits.DONE

// resistive touch screen analog pins
#define RESISTIVETOUCH_ADC_XPOS             13
#define RESISTIVETOUCH_ADC_YPOS             12

#define RESISTIVETOUCH_ADPCFG_XPOS          AD1PCFGbits.PCFG13
#define RESISTIVETOUCH_ADPCFG_YPOS          AD1PCFGbits.PCFG12

#define RESISTIVETOUCH_ANALOG               0
#define RESISTIVETOUCH_DIGITAL              1

// X port definitions
#define RESISTIVETOUCH_XP_Signal            LATBbits.LATB13
#define RESISTIVETOUCH_XM_Signal            LATBbits.LATB11
#define RESISTIVETOUCH_XP_Direction         TRISBbits.TRISB13
#define RESISTIVETOUCH_XM_Direction         TRISBbits.TRISB11

// Y port definitions
#define RESISTIVETOUCH_YP_Signal            LATBbits.LATB12
#define RESISTIVETOUCH_YM_Signal            LATBbits.LATB10
#define RESISTIVETOUCH_YP_Direction         TRISBbits.TRISB12
#define RESISTIVETOUCH_YM_Direction         TRISBbits.TRISB10

// Touch Screen Non-Volatile Memory Storage Macros
#define ADDRESS_RESISTIVE_TOUCH_VERSION	(unsigned long)0xFFFFFFFE
#define ADDRESS_RESISTIVE_TOUCH_ULX     (unsigned long)0xFFFFFFFC
#define ADDRESS_RESISTIVE_TOUCH_ULY     (unsigned long)0xFFFFFFFA
#define ADDRESS_RESISTIVE_TOUCH_URX     (unsigned long)0xFFFFFFF8
#define ADDRESS_RESISTIVE_TOUCH_URY     (unsigned long)0xFFFFFFF6

#define ADDRESS_RESISTIVE_TOUCH_LLX     (unsigned long)0xFFFFFFF4
#define ADDRESS_RESISTIVE_TOUCH_LLY     (unsigned long)0xFFFFFFF2
#define ADDRESS_RESISTIVE_TOUCH_LRX     (unsigned long)0xFFFFFFF0
#define ADDRESS_RESISTIVE_TOUCH_LRY     (unsigned long)0xFFFFFFEE

// define the functions to call for the non-volatile memory
// check out touch screen module for definitions of the following function pointers
// used: NVM_READ_FUNC, NVM_WRITE_FUNC & NVM_SECTORERASE_FUNC
#define NVMSectorErase                  ((NVM_SECTORERASE_FUNC)&DRV_NVM_M25P80_SectorErase)
#define NVMWrite                        ((NVM_WRITE_FUNC      )&DRV_NVM_M25P80_Write)
#define NVMRead                         ((NVM_READ_FUNC       )&DRV_NVM_M25P80_Read)

// touch screen font definition
#define DRV_TOUCHSCREEN_FONT Gentium27

/*********************************************************************
* HARDWARE PROFILE for USB to program external memory
*********************************************************************/

#define DataChipErase                  ((FLASH_CHIPERASE_FUNC)&DRV_NVM_M25P80_ChipErase)
#define DataWrite                      ((FLASH_WRITE_FUNC    )&DRV_NVM_M25P80_Write)
#define DataRead                       ((FLASH_READ_FUNC     )&DRV_NVM_M25P80_Read)

#define USE_COMM_PKT_MEDIA_USB
#define COMM_PKT_RX_MAX_SIZE    (1024)

//#define USE_SELF_POWER_SENSE_IO
#define tris_self_power     TRISAbits.TRISA2    // Input
#define self_power          1

//#define USE_USB_BUS_SENSE_IO
#define tris_usb_bus_sense  TRISBbits.TRISB5    // Input
#define USB_BUS_SENSE       U1OTGSTATbits.SESVD // Special considerations required if using SESVD for this purpose.  See documentation.


/*********************************************************************
* Macro for the background image stored in the internal flash
*********************************************************************/
#define APP_INTERNAL_FLASH_BACKGROUND_IMAGE (&background_abstract_light_weave_4bpp_480x272)


#endif




