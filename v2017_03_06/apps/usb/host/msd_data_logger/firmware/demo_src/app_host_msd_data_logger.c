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

/** INCLUDES *******************************************************/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#include "system.h"
#include "usb.h"
#include "fileio.h"
#include "app_host_msd_data_logger.h"
#include "timer_1ms.h"
#include "adc.h"
#include "leds.h"


static FILEIO_OBJECT myFile;

typedef enum
{
    WAITING_FOR_ATTACH,
    MOUNTING_DRIVE,
    OPENING_FILE,
    WAITING_TO_WRITE,
    WRITING_TO_FILE,
    CLOSING_FILE,
    UNMOUNT_DRIVE,
    WAITING_FOR_DETACH
} USB_HOST_MSD_DATA_LOGGER_DEMO_STATE;

static USB_HOST_MSD_DATA_LOGGER_DEMO_STATE demoState;
static uint8_t deviceAddress = 0;
static volatile bool sampleRequested = false;
static char printBuffer[10];
static uint8_t blinkCount;

// Declare a FILEIO_DRIVE_CONFIG structure to describe which functions the File I/O library will use to communicate with the media
const FILEIO_DRIVE_CONFIG gUSBDrive =
{
    (FILEIO_DRIVER_IOInitialize)NULL,                     // Function to initialize the I/O pins used by the driver.
    (FILEIO_DRIVER_MediaDetect)USBHostMSDSCSIMediaDetect,                   // Function to detect that the media is inserted.
    (FILEIO_DRIVER_MediaInitialize)USBHostMSDSCSIMediaInitialize,           // Function to initialize the media.
    (FILEIO_DRIVER_MediaDeinitialize)USBHostMSDSCSIMediaDeinitialize,              // Function to de-initialize the media.
    (FILEIO_DRIVER_SectorRead)USBHostMSDSCSISectorRead,                     // Function to read a sector from the media.
    (FILEIO_DRIVER_SectorWrite)USBHostMSDSCSISectorWrite,                   // Function to write a sector to the media.
    (FILEIO_DRIVER_WriteProtectStateGet)USBHostMSDSCSIWriteProtectState,    // Function to determine if the media is write-protected.
};

/*********************************************************************
* Function: void APP_HostMSDDataLoggerInitialize(void);
*
* Overview: Initializes the demo code
*
* PreCondition: None
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_HostMSDDataLoggerInitialize()
{
    demoState = WAITING_FOR_ATTACH;
}

void APP_MountDrive(uint8_t address)
{
    if (demoState == WAITING_FOR_ATTACH)
    {
        deviceAddress = address;
        demoState = MOUNTING_DRIVE;
    }
}

void APP_HostMSDDataLoggerTickHandler(void)
{
    sampleRequested = true;

    blinkCount++;

    if(blinkCount > 5)
    {
        LED_Toggle(LED_USB_HOST_MSD_DATA_LOGGER);
        blinkCount = 0;
    }
}

/*********************************************************************
* Function: void APP_HostMSDDataLoggerTasks(void);
*
* Overview: Keeps the demo running.
*
* PreCondition: The demo should have been initialized via
*   the APP_HostMSDDataLoggerInitialize()  
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_HostMSDDataLoggerTasks()
{
    if(FILEIO_MediaDetect(&gUSBDrive, &deviceAddress) == false)
    {
        //The device has been removed.  Now we should wait for a new
        //  device to be attached.
        demoState = WAITING_FOR_ATTACH;
    }
    
    switch( demoState)
    {
        case WAITING_FOR_ATTACH:
            break;

        case MOUNTING_DRIVE:
        {           
            // Attempt to mount the drive described by gUSBDrive as drive 'A'
            // The deviceAddress parameter describes the USB address of the device; it is initialized by the application in the 
            // USB_ApplicationEventHandler function when a new device is detected.
            if( FILEIO_DriveMount ('A', &gUSBDrive, &deviceAddress) == FILEIO_ERROR_NONE)
            {
                demoState = OPENING_FILE;
            }
            break;
        }

        case OPENING_FILE:
            // Opening a file with the FILEIO_OPEN_WRITE option allows us to write to the file.
            // Opening a file with the FILEIO_OPEN_CREATE file will create the file if it does not already exist.
            // Opening a file with the FILEIO_OPEN_TRUNCATE file will truncate it to a 0-length file if it already exists.
            if(FILEIO_Open(&myFile, "LOG.CSV", FILEIO_OPEN_WRITE | FILEIO_OPEN_CREATE | FILEIO_OPEN_TRUNCATE) == FILEIO_RESULT_SUCCESS)
            {
                //Opening the file failed.  Since we can't write to the
                //  device, abort the write attempt and wait for the device
                //  to be removed.
                demoState = WRITING_TO_FILE;

                blinkCount = 0;
                sampleRequested = false;
                TIMER_RequestTick(&APP_HostMSDDataLoggerTickHandler, 50);
                LED_On(LED_USB_HOST_MSD_DATA_LOGGER);
                ADC_ChannelEnable(ADC_USB_HOST_MSD_DATA_SOURCE);
                break;
            }
            break;

        case WRITING_TO_FILE:
            if(sampleRequested == true)
            {
                uint16_t  adcResult;
                int charCount;

                sampleRequested = false;
                
                adcResult = ADC_Read10bit(ADC_USB_HOST_MSD_DATA_SOURCE);

                charCount = sprintf(printBuffer, "%d\r\n", adcResult);
                
                //Write some data to the new file.
                FILEIO_Write(printBuffer, 1, charCount, &myFile);
            }

            if(BUTTON_IsPressed(BUTTON_USB_HOST_MSD_DATA_LOGGER) == true)
            {
                demoState = CLOSING_FILE;
            }
            break;

        case CLOSING_FILE:
            //Always make sure to close the file so that the data gets
            //  written to the drive.
            FILEIO_Close(&myFile);
            TIMER_CancelTick(&APP_HostMSDDataLoggerTickHandler);

            demoState = UNMOUNT_DRIVE;
            break;

        case UNMOUNT_DRIVE:
            // Unmount the drive since it is no longer in use.
            FILEIO_DriveUnmount ('A');

            //Now that we are done writing, we can do nothing until the
            //  drive is removed.
            demoState = WAITING_FOR_DETACH;
            break;

        case WAITING_FOR_DETACH:
            LED_Off(LED_USB_HOST_MSD_DATA_LOGGER);
            break;

        default:
            break;
    }
}
