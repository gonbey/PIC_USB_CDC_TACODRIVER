/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    ping_demo.c

  Summary:
    ICMP Client Demo (Ping)

  Description:


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

#define __PINGDEMO_C_

#include <stdbool.h>
#include <stdint.h>
#include "system_config.h"

#if defined(STACK_USE_ICMP_CLIENT)

#include "tcpip/tcpip.h"
#include "main.h"

/* used for assertions */
#if defined(WF_DEBUG)
#define WF_MODULE_NUMBER WF_MODULE_PING_DEMO
#endif

#define HOST_TO_PING "ww1.microchip.com" // Address that ICMP client will ping.  If the DNS client module is not available in the stack, then this hostname is ignored and the local gateway IP address will be pinged instead.

/*****************************************************************************
  Function:
    void PingDemo(void)

  Summary:
    Demonstrates use of the ICMP (Ping) client.

  Description:
    This function implements a simple ICMP client.  The function is called
    periodically by the stack, and it checks if BUTTON0 has been pressed.
    If the button is pressed, the function sends an ICMP Echo Request (Ping)
    to a Microchip web server.  The round trip time is displayed on the UART
    when the response is received.

    This function can be used as a model for applications requiring Ping
    capabilities to check if a host is reachable.

  Precondition:
    TCP is initialized.

  Parameters:
    None

  Returns:
    None
 ***************************************************************************/
void PingDemo(void)
{
    static enum {
        SM_HOME = 0,
        SM_GET_ICMP_RESPONSE
    } PingState = SM_HOME;
    static uint32_t Timer;
    int32_t ret;

    switch (PingState) {
    case SM_HOME:
        // Send a ping request out if the user pushes BUTTON0 (right-most one)
        if (BUTTON0_IO == 0u) {
            // Don't ping flood: wait at least 1 second between ping requests
            if (TickGet() - Timer > 1ul * TICK_SECOND) {
                // Obtain ownership of the ICMP module
                if (!ICMPBeginUsage())
                    break;

                // Update anti-ping flood timer
                Timer = TickGet();

                // Send ICMP echo request
#if defined(STACK_USE_DNS_CLIENT)
                ICMPSendPingToHostROM((ROM uint8_t *) HOST_TO_PING);
#else
                ICMPSendPing(AppConfig.MyGateway.Val);
#endif
                PingState = SM_GET_ICMP_RESPONSE;
            }
        }
        break;

    case SM_GET_ICMP_RESPONSE:
        // Get the status of the ICMP module
        ret = ICMPGetReply();
        if (ret == -2) {
            // Do nothing: still waiting for echo
            break;
        } else if (ret == -1) {
            // Request timed out
#if defined(USE_LCD)
            memcpypgm2ram((void *) &LCDText[16], (ROM void *) "Ping timed out", 15);
            LCDUpdate();
#endif
            PingState = SM_HOME;
        } else if (ret == -3) {
            // DNS address not resolvable
#if defined(USE_LCD)
            memcpypgm2ram((void *) &LCDText[16], (ROM void *) "Can't resolve IP", 16);
            LCDUpdate();
#endif
            PingState = SM_HOME;
        } else {
            // Echo received.  Time elapsed is stored in ret (Tick units).
#if defined(USE_LCD)
            memcpypgm2ram((void *) &LCDText[16], (ROM void *) "Reply: ", 7);
            tcpip_helper_uitoa((uint16_t) TickConvertToMilliseconds((uint32_t) ret), &LCDText[16 + 7]);
            strcatpgm2ram((char *) &LCDText[16 + 7], "ms");
            LCDUpdate();
#endif
            PingState = SM_HOME;
        }

        // Finished with the ICMP module, release it so other apps can begin using it
        ICMPEndUsage();
        break;
    }
}

uint8_t PING_Console_Host[32] = "192.168.1.1";
int32_t Count_PingConsole = 0;
bool b_PingFroever = false;

void PingConsole(void)
{
    static enum {
        SM_HOME = 0,
        SM_GET_ICMP_RESPONSE
    } PingState = SM_HOME;
    static uint32_t Timer;
    int32_t ret;
    static int32_t statistics_send = 0, statistics_Recv = 0, statistics_lost = 0;
    static int32_t statistics_TimeMax = 0, statistics_TimeMin = 0, statistics_TimeTotal = 0;

    if (b_PingFroever == true)
        Count_PingConsole = 4;

    switch (PingState) {
    case SM_HOME:
        if (Count_PingConsole > 0) {
            if (TickGet() - Timer > 1ul * TICK_SECOND) {
                if (!ICMPBeginUsage())
                    break;

                Timer = TickGet();
                // Send ICMP echo request
#if defined(STACK_USE_DNS_CLIENT)
                ICMPSendPingToHostROM((ROM uint8_t *) PING_Console_Host);
                statistics_send++;
#else
                putsUART("DNS assert ...");
                WF_ASSERT(false);
#endif
                Count_PingConsole--;
                PingState = SM_GET_ICMP_RESPONSE;
            }
        }

        break;

    case SM_GET_ICMP_RESPONSE:
        // Get the status of the ICMP module
        ret = ICMPGetReply();
        if (ret == -2) {
            // Do nothing: still waiting for echo
            break;
        } else if (ret == -1) {
            // Request timed out
            statistics_lost++;
            putsUART("Ping timed out ");

            {
                char buf_t[20];
                sprintf(buf_t, ":Lost %d times\r\n", (int) statistics_lost);
                putsUART(buf_t);
            }

            PingState = SM_HOME;
            if (Count_PingConsole == 0)
                goto _DonePingConsole;
        } else if (ret == -3) {
            // DNS address not resolvable
            putsUART("Can't resolve IP\r\n");
            PingState = SM_HOME;
            Count_PingConsole = 0;
        } else {
            // Echo received.  Time elapsed is stored in ret (Tick units).
            statistics_Recv++;
            uint32_t delay = TickConvertToMilliseconds((uint32_t) ret);
            if (delay > statistics_TimeMax)
                statistics_TimeMax = delay;

            if (delay < statistics_TimeMin)
                statistics_TimeMin = delay;

            if (statistics_TimeMin == 0)
                statistics_TimeMin = delay;

            statistics_TimeTotal += delay;
            putsUART("Reply From ");
            char buf_t[32] = {0};
            sprintf(buf_t, "%s: time=%dms\r\n", PING_Console_Host, (int) delay);
            putsUART(buf_t);
            PingState = SM_HOME;
            if (Count_PingConsole == 0)
                goto _DonePingConsole;
        }

        // Finished with the ICMP module, release it so other apps can begin using it
        ICMPEndUsage();
        break;

    default:
        break;
    }
    return;

_DonePingConsole:
    ICMPEndUsage();
    putsUART("Ping statistics for ");
    putsUART((char *) PING_Console_Host);
    putsUART(":\r\n");

    putsUART("  Packets: ");
    char buf_t[20];
    sprintf(buf_t, "Sent = %d, ", (int) statistics_send);
    putsUART(buf_t);
    sprintf(buf_t, "Received = %d, ", (int) statistics_Recv);
    putsUART(buf_t);
    sprintf(buf_t, "Lost = %d ", (int) statistics_lost);
    putsUART(buf_t);
    sprintf(buf_t, "(%d%c loss)", (int) ((100 * statistics_lost) / statistics_send), '%');
    putsUART(buf_t);

    putsUART("\r\nApproximate round trip times in milli-seconds:\r\n");
    sprintf(buf_t, "  Minimum = %dms, ", (int) statistics_TimeMin);
    putsUART(buf_t);
    sprintf(buf_t, "Maximum = %dms, ", (int) statistics_TimeMax);
    putsUART(buf_t);
    if (statistics_Recv != 0)
        sprintf(buf_t, "Average = %dms\r\n>", (int) (statistics_TimeTotal / statistics_Recv));
    putsUART(buf_t);

    putsUART("\r\n>");
    statistics_send = 0;
    statistics_Recv = 0;
    statistics_lost = 0;
    statistics_TimeMax = 0;
    statistics_TimeMin = 0;
    statistics_TimeTotal = 0;
    return;
}

#endif /* defined(STACK_USE_ICMP_CLIENT) */
