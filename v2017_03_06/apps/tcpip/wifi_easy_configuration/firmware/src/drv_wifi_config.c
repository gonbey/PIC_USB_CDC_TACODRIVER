/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    drv_wifi_config.c

  Summary:
    Module for Microchip TCP/IP Stack
    -Provides access to MRF24W WiFi controller
    -Reference: MRF24W Data sheet, IEEE 802.11 Standard

  Description:
    MRF24W Driver Customization

 *******************************************************************************/

//DOM-IGNORE-BEGIN
/*******************************************************************************
Copyright (c) <2014> released Microchip Technology Inc.  All rights reserved.

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
//DOM-IGNORE-END

#include "system_config.h"

#if defined(WF_CS_TRIS)

/*==========================================================================*/
/*                                  INCLUDES                                */
/*==========================================================================*/
#include "tcpip/tcpip.h"

#if defined ( EZ_CONFIG_SCAN )
#include "driver/wifi/mrf24w/src/drv_wifi_easy_config.h"
#endif /* EZ_CONFIG_SCAN */

/*==========================================================================*/
/*                                  DEFINES                                 */
/*==========================================================================*/

/* used for assertions */
#if defined(WF_DEBUG)
#define WF_MODULE_NUMBER WF_MODULE_WF_CONFIG
#endif

#if MY_DEFAULT_NETWORK_TYPE == WF_SOFT_AP
extern uint8_t g_scan_done;
#endif

#if defined(WF_PRE_SCAN_IN_ADHOC)
uint8_t g_prescan_adhoc_done = 0;
#endif

/*****************************************************************************
 * FUNCTION: WF_ProcessEvent
 *
 * RETURNS:  None
 *
 * PARAMS:   event      -- event that occurred
 *           eventInfo  -- additional information about the event.  Not all events
 *                         have associated info, in which case this value will be
 *                         set to WF_NO_ADDITIONAL_INFO (0xff)
 *           extraInfo - more additional information about the event
 *
 *  NOTES:   The Host application must NOT directly call this function.  This
 *           function is called by the WiFi Driver code when an event occurs
 *           that may need processing by the Host CPU.
 *
 *           No other WiFi Driver function should be called from this function, with the
 *           exception of WF_ASSERT.  It is recommended that if the application wishes to be
 *           notified of an event that it simply set a flag and let application code in the
 *           main loop handle the event.
 *
 *           WFSetFuncState must be called when entering and leaving this function.
 *           When WF_DEBUG is enabled this allows a runtime check if any illegal WF functions
 *           are called from within this function.
 *
 *           For events that the application is not interested in simply leave the
 *           case statement empty.
 *
 *           Customize this function as needed for your application.
 *****************************************************************************/
void WF_ProcessEvent(uint8_t event, uint16_t eventInfo, uint8_t *extraInfo)
{
#if defined(STACK_USE_UART)
    char buf[8];
#endif
    tMgmtIndicateSoftAPEvent *softAPEvent;

    /* this function tells the WF driver that we are in this function */
    WFSetFuncState(WF_PROCESS_EVENT_FUNC, WF_ENTERING_FUNCTION);

    switch (event) {
    /*--------------------------------------*/
    case WF_EVENT_CONNECTION_SUCCESSFUL:
    /*--------------------------------------*/
#if defined(STACK_USE_UART)
        putrsUART("Event: Connection Successful\r\n");

#if defined(EZ_CONFIG_STORE)
        AppConfig.saveSecurityInfo = true;
#endif

#endif
        break;

    /*--------------------------------------*/
    case WF_EVENT_CONNECTION_FAILED:
    case WF_EVENT_CONNECTION_TEMPORARILY_LOST:
    case WF_EVENT_CONNECTION_PERMANENTLY_LOST:
    /*--------------------------------------*/
#if defined(STACK_USE_UART)
        WF_OutputConnectionDebugMsg(event, eventInfo);
#endif
        break;

    /*--------------------------------------*/
    case WF_EVENT_CONNECTION_REESTABLISHED:
    /*--------------------------------------*/
#if defined(STACK_USE_UART)
        putrsUART("Event: Connection Reestablished\r\n");
#endif

        break;

    /*--------------------------------------*/
    case WF_EVENT_SCAN_RESULTS_READY:
    /*--------------------------------------*/
#if defined(STACK_USE_UART)
        putrsUART("Event: Scan Results Ready,");
        sprintf(buf, " %d", eventInfo);
        putsUART(buf);
        putrsUART("results\r\n");
#endif /* STACK_USE_UART */

#if defined ( EZ_CONFIG_SCAN ) && !defined(__XC8)
        WFScanEventHandler(eventInfo);
#endif /* EZ_CONFIG_SCAN */

#if MY_DEFAULT_NETWORK_TYPE == WF_SOFT_AP
        g_scan_done = 1; // WF_PRESCAN
#endif

#if defined(WF_PRE_SCAN_IN_ADHOC)
        g_prescan_adhoc_done = 1;
#endif
        break;

    case WF_EVENT_SOFT_AP_EVENT:
        softAPEvent = (tMgmtIndicateSoftAPEvent *) extraInfo;
#if defined(STACK_USE_UART)
    {
        char str[96];
        char *result = "None";
        char *reason = "None";
        uint8_t *addr = softAPEvent->address;
        putrsUART("Event: SoftAP, ");
        if (softAPEvent->event == SOFTAP_EVENT_CONNECTED) {
            result = "Connected";
        } else if (softAPEvent->event == SOFTAP_EVENT_DISCONNECTED) {
            result = "Disconnected";
            if (softAPEvent->reason == SOFTAP_EVENT_LINK_LOST)
                reason = "LinkLost";
            else if (softAPEvent->reason == SOFTAP_EVENT_RECEIVED_DEAUTH)
            {
                reason = "ReceivedDeauth";
                DHCPServer_ClientRemove(addr);
            }
        }
        sprintf(str, "%s, %s, %x:%x:%x:%x:%x:%x", result, reason, addr[0], addr[1], addr[2],
                addr[3], addr[4], addr[5]);
        putsUART(str);
        putrsUART("\r\n");
    }
#endif /* STACK_USE_UART */
        break;

    default:
        WF_ASSERT(false); /* unknown event */
        break;
    }

    /* Informs the WF driver that we are leaving this function */
    WFSetFuncState(WF_PROCESS_EVENT_FUNC, WF_LEAVING_FUNCTION);
}

#endif /* WF_CS_TRIS */
