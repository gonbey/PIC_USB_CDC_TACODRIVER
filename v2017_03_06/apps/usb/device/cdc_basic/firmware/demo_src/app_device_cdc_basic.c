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

#include <stdint.h>
#include <string.h>
#include <stddef.h>

#include "usb.h"

#include "app_led_usb_status.h"
#include "app_device_cdc_basic.h"
#include "usb_config.h"

uint8_t storeBuffer[64];
int storeBufferPos = 0;
/*********************************************************************
* Function: void APP_DeviceCDCBasicDemoInitialize(void);
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
void APP_DeviceCDCBasicDemoInitialize()
{   
    line_coding.bCharFormat = 0;
    line_coding.bDataBits = 8;
    line_coding.bParityType = 0;
    line_coding.dwDTERate = 9600;
}

/*********************************************************************
* Function: void APP_DeviceCDCBasicDemoTasks(void);
*
* Overview: Keeps the demo running.
*
* PreCondition: The demo should have been initialized and started via
*   the APP_DeviceCDCBasicDemoInitialize() and APP_DeviceCDCBasicDemoStart() demos
*   respectively.
*
* Input: None
*
* Output: None
*
********************************************************************/
void APP_DeviceCDCBasicDemoTasks()
{
    /* If the USB device isn't configured yet, we can't really do anything
     * else since we don't have a host to talk to.  So jump back to the
     * top of the while loop. */
    if( USBGetDeviceState() < CONFIGURED_STATE )
    {
        return;
    }

    /* If we are currently suspended, then we need to see if we need to
     * issue a remote wakeup.  In either case, we shouldn't process any
     * keyboard commands since we aren't currently communicating to the host
     * thus just continue back to the start of the while loop. */
    if( USBIsDeviceSuspended()== true )
    {
        return;
    }
        
    /* Check to see if there is a transmission in progress, if there isn't, then
     * we can see about performing an echo response to data received.
     */
    if( USBUSARTIsTxTrfReady() == 1) {

        uint8_t readBuffer[CDC_DATA_OUT_EP_SIZE];
        uint8_t writeBuffer[CDC_DATA_IN_EP_SIZE];
        uint8_t numBytesRead;

        // 読み込み！
        numBytesRead = getsUSBUSART(readBuffer, sizeof(readBuffer));
        for (int i =0 ;i<numBytesRead; i++) {
            // 一文字読み込み
            uint8_t c = readBuffer[i];
            switch(c) {
                // 改行コードで書き込みバッファに出力
                case 0x0A:
                case 0x0D:
                    putUSBUSART(storeBuffer, storeBufferPos);
                    storeBufferPos = 0;
                    break;
                // それ以外の文字は内部にためる
                default:
                    // ためる
                    storeBuffer[i + storeBufferPos] = readBuffer[i];
                    storeBufferPos += i;
                    // ログ出力
//                    char log[64];
//                    char logheader = "im Bufferering : ";
//                    int catPos = 0;
//                    memcpy(log + catPos, logheader, strlen(logheader));
//                    catPos += strlen(logheader);
//                    memcpy(log + catPos, readBuffer[i], 1);
//                    
//                    catPos += 1;
                    
                    char t[32];
                    int pos; pos = 0;
                    char* buf; int append;
                    buf = "moge"; append = strlen(buf);
                    memcpy(t + pos, buf, append);
                    pos += append;
                    buf = "mige"; append = strlen(buf);
                    memcpy(t + pos, buf, append);
                    pos += append;
                    putUSBUSART(t, pos);
                    break;
            }
        }
    }
    CDCTxService();
}