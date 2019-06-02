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
#include "system.h"
#include "usb.h"
#include "usb_host_msd.h"
#include "app_host_msd_simple.h"

void GetTimestamp (FILEIO_TIMESTAMP * timestamp);

/********************************************************************
 * Function:        void main(void)
 *
 * PreCondition:    None
 *
 * Input:           None
 *
 * Output:          None
 *
 * Side Effects:    None
 *
 * Overview:        Main program entry point.
 *
 * Note:            None
 *******************************************************************/
int main(void)
{
    SYS_Initialize();

    FILEIO_Initialize();

    FILEIO_RegisterTimestampGet(GetTimestamp);

    //Initialize the stack
    USBHostInit(0);

    APP_HostMSDSimpleInitialize();

    while(1)
    {
        USBHostTasks();
        USBHostMSDTasks();

        //Application specific tasks
        APP_HostMSDSimpleTasks();
    }//end while
}//end main



/****************************************************************************
  Function:
    bool USB_ApplicationEventHandler( uint8_t address, USB_EVENT event,
                void *data, uint32_t size )

  Summary:
    This is the application event handler.  It is called when the stack has
    an event that needs to be handled by the application layer rather than
    by the client driver.

  Description:
    This is the application event handler.  It is called when the stack has
    an event that needs to be handled by the application layer rather than
    by the client driver.  If the application is able to handle the event, it
    returns true.  Otherwise, it returns false.

  Precondition:
    None

  Parameters:
    uint8_t address    - Address of device where event occurred
    USB_EVENT event - Identifies the event that occurred
    void *data      - Pointer to event-specific data
    uint32_t size      - Size of the event-specific data

  Return Values:
    true    - The event was handled
    false   - The event was not handled

  Remarks:
    The application may also implement an event handling routine if it
    requires knowledge of events.  To do so, it must implement a routine that
    matches this function signature and define the USB_HOST_APP_EVENT_HANDLER
    macro as the name of that function.
  ***************************************************************************/

bool USB_ApplicationEventHandler( uint8_t address, USB_EVENT event, void *data, uint32_t size )
{
    switch( (int) event )
    {
        case EVENT_VBUS_REQUEST_POWER:
            // The data pointer points to a byte that represents the amount of power
            // requested in mA, divided by two.  If the device wants too much power,
            // we reject it.
            return true;

        case EVENT_VBUS_RELEASE_POWER:
            //This means that the device was removed
            return true;
            break;

        /* Here are various other events that a user might want to handle
         * or be aware of.  In this demo we are not handling them so we
         * will just return true to allow the stack to move on from the error.
         */
        case EVENT_HUB_ATTACH:
        case EVENT_UNSUPPORTED_DEVICE:
        case EVENT_CANNOT_ENUMERATE:
        case EVENT_CLIENT_INIT_ERROR:
        case EVENT_OUT_OF_MEMORY:
        case EVENT_UNSPECIFIED_ERROR:   // This should never be generated.
            return true;
            break;

        case EVENT_MSD_ATTACH:
            APP_MountDrive (address);
            break;

        default:
            break;
    }

    return false;
}

// Placeholder function to get the timestamp for FILEIO operations
void GetTimestamp (FILEIO_TIMESTAMP * timestamp)
{
    // Populate the timestamp field with some inaccurate timestamp information
    timestamp->date.bitfield.day = 5;
    timestamp->date.bitfield.month = 1;
    timestamp->date.bitfield.year = 33;
    timestamp->time.bitfield.hours = 8;
    timestamp->time.bitfield.secondsDiv2 = 20;
    timestamp->time.bitfield.minutes = 10;
    timestamp->timeMs = 0;
}

