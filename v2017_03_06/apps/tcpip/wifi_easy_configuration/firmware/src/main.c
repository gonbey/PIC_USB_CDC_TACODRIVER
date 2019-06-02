/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    Main Application Entry Point and TCP/IP Stack Demo

  Description:
    Module for Microchip TCP/IP Stack
    -Demonstrates how to call and use the Microchip TCP/IP stack
    -Reference: Microchip TCP/IP Stack Help (TCPIP Stack Help.chm)    

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

/*
 * This macro uniquely defines this file as the main entry point.
 * There should only be one such definition in the entire project,
 * and this file must define the AppConfig variable as described below.
 */
#define THIS_IS_STACK_APPLICATION

// Include all headers for any enabled TCPIP Stack functions
#include "tcpip/tcpip.h"

#if defined(STACK_USE_ZEROCONF_LINK_LOCAL)
#include "tcpip/zero_conf_link_local.h"
#endif
#if defined(STACK_USE_ZEROCONF_MDNS_SD)
#include "tcpip/zero_conf_link_multicast_dns.h"
#endif

// Include functions specific to this stack application
#include "main.h"

// Used for Wi-Fi assertions
#define WF_MODULE_NUMBER WF_MODULE_MAIN_DEMO

#if defined( WF_CONSOLE )
#include "driver/wifi/mrf24w/src/drv_wifi_console.h"
#include "iperf_app.h"
#endif

// Declare AppConfig structure and some other supporting stack variables
APP_CONFIG AppConfig;
static unsigned short wOriginalAppConfigChecksum; // Checksum of the ROM defaults for AppConfig
uint8_t AN0String[8];

#if (MY_DEFAULT_NETWORK_TYPE == WF_SOFT_AP)
uint8_t g_scan_done = 0; // WF_PRESCAN This will be set wheneven event scan results are ready.
uint8_t g_prescan_waiting = 1; // WF_PRESCAN This is used only to allow POR prescan once.
#endif

#if defined(WF_PRE_SCAN_IN_ADHOC)
extern void WFGetScanResults(void);
extern uint8_t g_prescan_adhoc_done;
#endif

// Private helper functions.
// These may or may not be present in all applications.

static void InitAppConfig(void);
static void InitializeBoard(void);

#if defined(WF_CS_TRIS)
void WF_Connect(void);
#endif

//
// PIC18 Interrupt Service Routines
//
#if defined(__XC8)

void interrupt low_priority LowISR(void)
{
    TickUpdate();
}

void interrupt HighISR(void)
{
#if defined(STACK_USE_UART2TCP_BRIDGE)
    UART2TCPBridgeISR();
#endif

#if defined(WF_CS_TRIS)
    WFEintISR();
#endif // WF_CS_TRIS
}

// PIC24 and PIC32 Exception Handlers
// If your code gets here, you either tried to read or write
// a NULL pointer, or your application overflowed the stack
// by having too many local variables or parameters declared.
#elif defined(__XC16)

#if defined(STACK_USE_UART)
#define UART2PrintString  putrsUART
#else
#define UART2PrintString(x)
#endif

void __attribute__((interrupt, auto_psv)) _DefaultInterrupt(void)
{
    UART2PrintString("!!! Default interrupt handler !!!\r\n");
    while (1) {
        Nop();
        Nop();
        Nop();
    }
}

void __attribute__((interrupt, auto_psv)) _OscillatorFail(void)
{
    UART2PrintString("!!! Oscillator Fail interrupt handler !!!\r\n");
    while (1) {
        Nop();
        Nop();
        Nop();
    }
}

void __attribute__((interrupt, auto_psv)) _AddressError(void)
{
    UART2PrintString("!!! Address Error interrupt handler !!!\r\n");
    while (1) {
        Nop();
        Nop();
        Nop();
    }
}

void __attribute__((interrupt, auto_psv)) _StackError(void)
{
    UART2PrintString("!!! Stack Error interrupt handler !!!\r\n");
    while (1) {
        Nop();
        Nop();
        Nop();
    }
}

void __attribute__((interrupt, auto_psv)) _MathError(void)
{
    UART2PrintString("!!! Math Error interrupt handler !!!\r\n");
    while (1) {
        Nop();
        Nop();
        Nop();
    }
}

#elif defined(__XC32)

void _general_exception_handler(unsigned cause, unsigned status)
{
    Nop();
    Nop();
}

#endif

#if defined(WF_CS_TRIS)
// Global variables
uint8_t ConnectionProfileID;
#endif

//
// Main application entry point.
//
#if defined(__XC8)
void main(void)
#else
int main(void)
#endif
{
    static uint32_t t = 0;
    static uint32_t dwLastIP = 0;

#if defined (EZ_CONFIG_STORE)
    static uint32_t ButtonPushStart = 0;
#endif

#if (MY_DEFAULT_NETWORK_TYPE == WF_SOFT_AP)
    uint8_t channelList[] = MY_DEFAULT_CHANNEL_LIST_PRESCAN; // WF_PRESCAN
    tWFScanResult bssDesc;
#endif

    // Initialize application specific hardware
    InitializeBoard();

#if defined(USE_LCD)
    // Initialize and display the stack version on the LCD
    LCDInit();
    DelayMs(100);
    strcpypgm2ram((char *)LCDText, "TCPStack " TCPIP_STACK_VERSION "  "
            "                ");
    LCDUpdate();
#endif

    // Initialize stack-related hardware components that may be
    // required by the UART configuration routines

    // Initialize TCP/IP stack timer
    TickInit();

#if defined(STACK_USE_MPFS2)
    MPFSInit();
#endif

    // Initialize Stack and application related NV variables into AppConfig.
    InitAppConfig();

    // Initiates board setup process if button is depressed
    // on startup
    if (BUTTON0_IO == 0u) {
#if defined(EEPROM_CS_TRIS) || defined(SPIFLASH_CS_TRIS)
        // Invalidate the EEPROM contents if BUTTON0 is held down for more than 4 seconds
        uint32_t StartTime = TickGet();
        LED_PUT(0x00);

        while (BUTTON0_IO == 0u) {
            if (TickGet() - StartTime > 4 * TICK_SECOND) {
#if defined(EEPROM_CS_TRIS)
                XEEBeginWrite(0x0000);
                XEEWrite(0xFF);
                XEEWrite(0xFF);
                XEEEndWrite();
#elif defined(SPIFLASH_CS_TRIS)
                SPIFlashBeginWrite(0x0000);
                SPIFlashWrite(0xFF);
                SPIFlashWrite(0xFF);
#endif

#if defined(STACK_USE_UART)
                putrsUART("\r\n\r\nBUTTON0 held for more than 4 seconds.  Default settings restored.\r\n\r\n");
#endif

                LED_PUT(0x0F);
                while ((int32_t) (TickGet() - StartTime) <= (int32_t) (9 * TICK_SECOND / 2));
                LED_PUT(0x00);
                while (BUTTON0_IO == 0u);
                Reset();
                break;
            }
        }
#endif

#if defined(STACK_USE_UART)
        DoUARTConfig();
#endif
    }

    // Initialize core stack layers (MAC, ARP, TCP, UDP) and
    // application modules (HTTP, etc.)
    StackInit();

#if defined(EZ_CONFIG_SCAN)
    WFInitScan();
#endif

#if (MY_DEFAULT_NETWORK_TYPE == WF_SOFT_AP)
    // WF_PRESCAN: Pre-scan before starting up as SoftAP mode
    WF_CASetScanType(MY_DEFAULT_SCAN_TYPE);
    WF_CASetChannelList(channelList, sizeof(channelList));

    if (WFStartScan() == WF_SUCCESS) {
        SCAN_SET_DISPLAY(SCANCXT.scanState);
        SCANCXT.displayIdx = 0;
        //putsUART("main: Prescan WFStartScan() success ................. \r\n");
    }

    // Needed to trigger g_scan_done
    WFRetrieveScanResult(0, &bssDesc);
#else
#if defined(WF_CS_TRIS)
    WF_Connect();
#endif // defined(WF_CS_TRIS)
#endif // (MY_DEFAULT_NETWORK_TYPE == WF_SOFT_AP)

    // Initialize any application-specific modules or functions/
    // For this demo application, this only includes the
    // UART 2 TCP Bridge
#if defined(STACK_USE_UART2TCP_BRIDGE)
    UART2TCPBridgeInit();
#endif

#if defined(STACK_USE_ZEROCONF_LINK_LOCAL)
    ZeroconfLLInitialize();
#endif

#if defined(STACK_USE_ZEROCONF_MDNS_SD)
    mDNSInitialize(MY_DEFAULT_HOST_NAME);
    mDNSServiceRegister(
            (const char *)"DemoWebServer", // base name of the service
            "_http._tcp.local", // type of the service
            80, // TCP or UDP port, at which this service is available
            ((const uint8_t *)"path=/index.htm"), // TXT info
            1, // auto rename the service when if needed
            NULL, // no callback function
            NULL // no application context
            );
    mDNSMulticastFilterRegister();
#endif

#if defined(WF_CONSOLE)
    WFConsoleInit();
#endif

    // Now that all items are initialized, begin the co-operative
    // multitasking loop.  This infinite loop will continuously
    // execute all stack-related tasks, as well as your own
    // application's functions.  Custom functions should be added
    // at the end of this loop.
    // Note that this is a "co-operative mult-tasking" mechanism
    // where every task performs its tasks (whether all in one shot
    // or part of it) and returns so that other tasks can do their
    // job.
    // If a task needs very long time to do its job, it must be broken
    // down into smaller pieces so that other tasks can have CPU time.
    while (1) {
#if (MY_DEFAULT_NETWORK_TYPE == WF_SOFT_AP)
        if (g_scan_done) {
            if (g_prescan_waiting) {
#if defined(STACK_USE_UART)
                putrsUART((ROM char *)"\n SoftAP prescan results ........ \r\n\n");
#endif
                SCANCXT.displayIdx = 0;
                while (IS_SCAN_STATE_DISPLAY(SCANCXT.scanState)) {
                    WFDisplayScanMgr();
                }
#if defined(STACK_USE_UART)
                putrsUART((ROM char *)"\r\n ");
#endif

#if defined(WF_CS_TRIS)
                WF_Connect();
#endif
                g_scan_done = 0;
                g_prescan_waiting = 0;
            }
        }
#endif // (MY_DEFAULT_NETWORK_TYPE == WF_SOFT_AP)

#if defined(WF_PRE_SCAN_IN_ADHOC)
        if (g_prescan_adhoc_done) {
            WFGetScanResults();
            g_prescan_adhoc_done = 0;
        }
#endif

#if defined (EZ_CONFIG_STORE)
        // Hold button3 for 4 seconds to reset to defaults.
        if (BUTTON3_IO == 0u) { // Button is pressed
            if (ButtonPushStart == 0) // Just pressed
                ButtonPushStart = TickGet();
            else
                if (TickGet() - ButtonPushStart > 4 * TICK_SECOND)
                RestoreWifiConfig();
        } else {
            ButtonPushStart = 0; // Button release reset the clock
        }

        if (AppConfig.saveSecurityInfo) {
            // set true by WF_ProcessEvent after connecting to a new network
            // get the security info, and if required, push the PSK to EEPROM
            if (AppConfig.securityMode == WF_SECURITY_WPA_AUTO_WITH_PASS_PHRASE ||
                AppConfig.securityMode == WF_SECURITY_WPA_WITH_PASS_PHRASE ||
                AppConfig.securityMode == WF_SECURITY_WPA2_WITH_PASS_PHRASE) {
                // only need to save when doing passphrase
                tWFCPElements profile;
                uint8_t connState;
                uint8_t connID;
                WF_CMGetConnectionState(&connState, &connID);
                WF_CPGetElements(connID, &profile);

                memcpy((char *)AppConfig.securityKey, (char *)profile.securityKey, 32);
                AppConfig.securityMode--; // the calc psk is exactly one below for each passphrase option
                AppConfig.securityKeyLength = 32;

                SaveAppConfig(&AppConfig);
            }

            AppConfig.saveSecurityInfo = false;
        }
#endif // EZ_CONFIG_STORE

#if defined (STACK_USE_EZ_CONFIG)
        // Blink LED0 twice per sec when unconfigured, once per sec after config
        if ((TickGet() - t >= TICK_SECOND / (4ul - (CFGCXT.isWifiDoneConfigure * 2ul))))
#else
        // Blink LED0 (right most one) every second.
        if (TickGet() - t >= TICK_SECOND / 2ul)
#endif // STACK_USE_EZ_CONFIG
        {
            t = TickGet();
            LED0_IO ^= 1;
        }

        // This task performs normal stack task including checking
        // for incoming packet, type of packet and calling
        // appropriate stack entity to process it.
        StackTask();

        // This tasks invokes each of the core stack application tasks
        StackApplications();

#if defined(STACK_USE_ZEROCONF_LINK_LOCAL)
        ZeroconfLLProcess();
#endif

#if defined(STACK_USE_ZEROCONF_MDNS_SD)
        mDNSProcess();
        // Use this function to exercise service update function
        // HTTPUpdateRecord();
#endif

        // Process application specific tasks here.
        // For this demo app, this will include the Generic TCP
        // client and servers, and the Ping
        // demos.  Following that, we will process any IO from
        // the inputs on the board itself.
        // Any custom modules or processing you need to do should
        // go here.

#if defined(WF_CONSOLE)
        WFConsoleProcess();
        WFConsoleProcessEpilogue();
#endif

#if defined(STACK_USE_ICMP_CLIENT)
        PingDemo();
        PingConsole();
#endif

        // If the local IP address has changed (ex: due to DHCP lease change)
        // write the new IP address to the LCD display, UART, and Announce
        // service
        if (dwLastIP != AppConfig.MyIPAddr.Val) {
            dwLastIP = AppConfig.MyIPAddr.Val;

#if defined(STACK_USE_UART)
            putrsUART((ROM char *)"\r\nNew IP Address: ");
#endif

            DisplayIPValue(AppConfig.MyIPAddr);

#if defined(STACK_USE_UART)
            putrsUART((ROM char *)"\r\n");
#endif

#if defined(STACK_USE_ANNOUNCE)
            AnnounceIP();
#endif

#if defined(STACK_USE_ZEROCONF_MDNS_SD)
            mDNSFillHostRecord();
#endif
        }

        if (AppConfig.hibernateFlag == 1) AppConfig.hibernateFlag = 0;
    }
}

#if defined(WF_CS_TRIS)

/*****************************************************************************
 * FUNCTION: WF_Connect
 *
 * RETURNS:  None
 *
 * PARAMS:   None
 *
 * NOTES:   Connects to an 802.11 network.  Customize this function as needed
 *           for your application.
 *****************************************************************************/
void WF_Connect(void)
{
    //uint8_t ConnectionProfileID;
    //uint8_t ConnectionState;
    uint8_t channelList[] = MY_DEFAULT_CHANNEL_LIST;
#if (MY_DEFAULT_NETWORK_TYPE == WF_SOFT_AP)
    uint8_t channelList_postscan[] = MY_DEFAULT_CHANNEL_LIST_POSTSCAN;
#endif

    /* create a Connection Profile */
    WF_CPCreate(&ConnectionProfileID);

    AppConfig.passphraseToBinaryKeyFlag = 0;

    WF_SetRegionalDomain(MY_DEFAULT_DOMAIN);

    WF_CPSetSsid(ConnectionProfileID,
            AppConfig.ssid,
            AppConfig.ssidLen);

    WF_CPSetNetworkType(ConnectionProfileID, AppConfig.networkType);
    if (AppConfig.networkType == WF_ADHOC) {
        WF_CPSetAdHocBehavior(ConnectionProfileID, WF_ADHOC_CONNECT_THEN_START);
    }

#if WF_HOST_DERIVE_KEY_FROM_PASSPHRASE == WF_ENABLED
    if (AppConfig.securityMode == WF_SECURITY_WPA_AUTO_WITH_PASS_PHRASE ||
        AppConfig.securityMode == WF_SECURITY_WPA_WITH_PASS_PHRASE ||
        AppConfig.securityMode == WF_SECURITY_WPA2_WITH_PASS_PHRASE) {
        WF_DeriveKey(AppConfig.securityKeyLength, AppConfig.securityKey, AppConfig.ssidLen, AppConfig.ssid);
        AppConfig.securityMode--;
        AppConfig.securityKeyLength = 32;
        AppConfig.passphraseToBinaryKeyFlag = 1;
    }
#endif /* #if WF_HOST_DERIVE_KEY_FROM_PASSPHRASE == WF_ENABLED */

#if !defined(MRF24WG)
    Delay10us(10); /* required for MRF24WB */
#endif

    switch (AppConfig.securityMode) {
    case WF_SECURITY_OPEN:
        WF_CPSetSecurity(ConnectionProfileID, WF_SECURITY_OPEN, 0, NULL, 0);
        break;
    case WF_SECURITY_WEP_40:
        // assume key 0
        WF_CPSetSecurity(ConnectionProfileID, AppConfig.securityMode, 0, AppConfig.securityKey, 5);
        break;
    case WF_SECURITY_WEP_104:
        // assume key 0
        WF_CPSetSecurity(ConnectionProfileID, AppConfig.securityMode, 0, AppConfig.securityKey, 13);
        break;
    case WF_SECURITY_WPA_AUTO_WITH_KEY:
        WF_CPSetSecurity(ConnectionProfileID, WF_SECURITY_WPA_AUTO_WITH_KEY,
                0, AppConfig.securityKey, 32);
        break;
    case WF_SECURITY_WPA_AUTO_WITH_PASS_PHRASE:
        WF_CPSetSecurity(ConnectionProfileID, WF_SECURITY_WPA_AUTO_WITH_PASS_PHRASE,
                0, AppConfig.securityKey, strlen((char *)AppConfig.securityKey));
        break;
    case WF_SECURITY_WPA_WITH_KEY:
        WF_CPSetSecurity(ConnectionProfileID, WF_SECURITY_WPA_WITH_KEY,
                0, AppConfig.securityKey, 32);
        break;
    case WF_SECURITY_WPA_WITH_PASS_PHRASE:
        WF_CPSetSecurity(ConnectionProfileID, WF_SECURITY_WPA_WITH_PASS_PHRASE,
                0, AppConfig.securityKey, strlen((char *)AppConfig.securityKey));
        break;
    case WF_SECURITY_WPA2_WITH_KEY:
        WF_CPSetSecurity(ConnectionProfileID, WF_SECURITY_WPA2_WITH_KEY,
                0, AppConfig.securityKey, 32);
        break;
    case WF_SECURITY_WPA2_WITH_PASS_PHRASE:
        WF_CPSetSecurity(ConnectionProfileID, WF_SECURITY_WPA2_WITH_PASS_PHRASE,
                0, AppConfig.securityKey, strlen((char *)AppConfig.securityKey));
        break;
    default:
        WF_ASSERT(false);
        break;
    }

    WF_CASetScanType(MY_DEFAULT_SCAN_TYPE);

#if (MY_DEFAULT_NETWORK_TYPE == WF_SOFT_AP)
    if (((CFGCXT.networkType != WF_SOFT_AP)&&(CFGCXT.prevNetworkType == WF_SOFT_AP)) || (AppConfig.networkType != WF_SOFT_AP)) {
        //putrsUART("\r\n\r\nWF_Connect: Channel list update when transitioning from SoftAP to non-SoftAP or NOT in SoftAP..\r\n\r\n");
        WF_CASetChannelList(channelList_postscan, sizeof(channelList_postscan));
    } else {
        WF_CASetChannelList(channelList, sizeof(channelList));
    }
#else
    WF_CASetChannelList(channelList, sizeof(channelList));
#endif // (MY_DEFAULT_NETWORK_TYPE == WF_SOFT_AP)

    // The Retry Count parameter tells the WiFi Connection manager how many attempts to make when trying
    // to connect to an existing network.  In the Infrastructure case, the default is to retry forever so that
    // if the AP is turned off or out of range, the radio will continue to attempt a connection until the
    // AP is eventually back on or in range.  In the Adhoc case, the default is to retry 3 times since the
    // purpose of attempting to establish a network in the Adhoc case is only to verify that one does not
    // initially exist.  If the retry count was set to WF_RETRY_FOREVER in the AdHoc mode, an AdHoc network
    // would never be established.  The constants MY_DEFAULT_LIST_RETRY_COUNT_ADHOC and
    // MY_DEFAULT_LIST_RETRY_COUNT_INFRASTRUCTURE have been created specifically for the June 2011 MAL release.
    if ((AppConfig.networkType == CFG_WF_ADHOC) || (AppConfig.networkType == CFG_WF_SOFT_AP)) {
        WF_CASetListRetryCount(ADHOC_RETRY_COUNT);
    } else /* AppConfig.networkType == CFG_WF_INFRASTRUCTURE */ {
        WF_CASetListRetryCount(MY_DEFAULT_LIST_RETRY_COUNT_INFRASTRUCTURE);
    }

    WF_CASetEventNotificationAction(MY_DEFAULT_EVENT_NOTIFICATION_LIST);

#if defined(WF_USE_POWER_SAVE_FUNCTIONS)
#if (MY_DEFAULT_PS_POLL == WF_ENABLED)
    WF_PsPollEnable(true);
#else
    WF_PsPollDisable();
#endif
#endif

    WF_CASetBeaconTimeout(MY_DEFAULT_BEACON_TIMEOUT);

#if defined(STACK_USE_UART)
    WF_OutputConnectionInfo(&AppConfig);
#endif

    if (AppConfig.networkType == CFG_WF_SOFT_AP) {
#if (WF_SOFTAP_SEND_KEEP_ALIVE == WF_ENABLED)
        WF_SetLinkDownThreshold(WF_SOFTAP_LINK_LOST_THRESHOLD);
#endif
    } else { // AppConfig.networkType != CFG_WF_SOFT_AP
#if (WF_CHECK_LINK_STATUS == WF_ENABLED)
        WF_SetLinkDownThreshold(WF_LINK_LOST_THRESHOLD);
#endif
    }

    WF_CMConnect(ConnectionProfileID);
}
#endif /* WF_CS_TRIS */

// Writes an IP address to the LCD display and the UART as available
void DisplayIPValue(IP_ADDR IPVal)
{
    //printf("%u.%u.%u.%u", IPVal.v[0], IPVal.v[1], IPVal.v[2], IPVal.v[3]);
    uint8_t IPDigit[4];
    uint8_t i;
#ifdef USE_LCD
    uint8_t j;
    uint8_t LCDPos = 16;
#endif

    for (i = 0; i < sizeof(IP_ADDR); i++) {
        tcpip_helper_uitoa((uint16_t) IPVal.v[i], IPDigit);

#if defined(STACK_USE_UART)
        putsUART((char *)IPDigit);
#endif

#ifdef USE_LCD
        for (j = 0; j < strlen((char *)IPDigit); j++) {
            LCDText[LCDPos++] = IPDigit[j];
        }
        if (i == sizeof(IP_ADDR) - 1)
            break;
        LCDText[LCDPos++] = '.';
#else
        if (i == sizeof(IP_ADDR) - 1)
            break;
#endif

#if defined(STACK_USE_UART)
        while (BusyUART());
        WriteUART('.');
#endif
    }

#ifdef USE_LCD
    if (LCDPos < 32u)
        LCDText[LCDPos] = 0;
    LCDUpdate();
#endif
}

/****************************************************************************
  Function:
    static void InitializeBoard(void)

  Description:
    This routine initializes the hardware.  It is a generic initialization
    routine for many of the Microchip development boards, using definitions
    in system_config.h to determine specific initialization.

  Precondition:
    None

  Parameters:
    None - None

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
static void InitializeBoard(void)
{
    // LEDs
    LED0_TRIS = 0;
    LED1_TRIS = 0;
    LED2_TRIS = 0;
    LED3_TRIS = 0;
    LED4_TRIS = 0;
    LED5_TRIS = 0;
    LED6_TRIS = 0;
    LED7_TRIS = 0;
    LED_PUT(0x00);

#if defined(__XC8)
    // Enable 4x/5x/96MHz PLL on PIC18F87J10, PIC18F97J60, PIC18F87J50, etc.
    OSCTUNE = 0x40;

    // Set up analog features of PORTA
    ADCON0 = 0x01; // ADON, Channel 0, Vdd/Vss is +/-REF
    WDTCONbits.ADSHR = 1;
    ANCON0 = 0xFC; // AN0 (POT) and AN1 (temp sensor) are anlog
    ANCON1 = 0xFF;
    WDTCONbits.ADSHR = 0;
    ADCON2 = 0xBE; // Right justify, 20TAD ACQ time, Fosc/64 (~21.0kHz)

    // Enable internal PORTB pull-ups
    INTCON2bits.RBPU = 0;

    // Configure USART
    TXSTA = 0x20;
    RCSTA = 0x90;

    // See if we can use the high baud rate setting
#if ((SYS_CLK_FrequencyPeripheralGet() + 2 * BAUD_RATE) / BAUD_RATE/4 - 1) <= 255
    SPBRG = (SYS_CLK_FrequencyPeripheralGet() + 2 * BAUD_RATE) / BAUD_RATE / 4 - 1;
    TXSTAbits.BRGH = 1;
#else // Use the low baud rate setting
    SPBRG = (SYS_CLK_FrequencyPeripheralGet() + 8 * BAUD_RATE) / BAUD_RATE / 16 - 1;
#endif

    // Enable Interrupts
    RCONbits.IPEN = 1; // Enable interrupt priorities
    INTCONbits.GIEH = 1;
    INTCONbits.GIEL = 1;

    // Do a calibration A/D conversion
    ADCON1bits.ADCAL = 1;
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO);
    ADCON1bits.ADCAL = 0;

#else // 16-bit XC16 and and 32-bit XC32

#if defined(__XC32)
    {
        // Enable multi-vectored interrupts
        INTEnableSystemMultiVectoredInt();

        // Enable optimal performance
        SYSTEMConfigPerformance(SYS_CLK_FrequencySystemGet());

        // Use 1:1 CPU Core:Peripheral Clocks
        mOSCSetPBDIV(OSC_PB_DIV_1);

        // Disable JTAG port so we get our I/O pins back, but first
        // wait 50ms so if you want to reprogram the part with
        // JTAG, you'll still have a tiny window before JTAG goes away.
        // The PIC32 Starter Kit debuggers use JTAG and therefore must not
        // disable JTAG.
        DelayMs(50);
#if !defined(__MPLAB_DEBUGGER_PIC32MXSK) && !defined(__MPLAB_DEBUGGER_FS2)
        DDPCONbits.JTAGEN = 0;
#endif

        // Turn the LEDs off
        LED_PUT(0x00);

        // Turn on weak pull ups on CN15, CN16, CN19 (RD5, RD7, RD13), which is connected to buttons on PIC32 Starter Kit boards
        CNPUESET = 0x00098000;
    }
#endif

#if defined(__PIC24F__)
    // Set 1:1 8MHz FRC Postscalar
    CLKDIVbits.RCDIV = 0;
#endif

    // ADC
    AD1CHS = 0; // Input to AN0 (potentiometer)
    AD1PCFGbits.PCFG4 = 0; // Disable digital input on AN4 (TC1047A temp sensor)
#if defined(__32MX460F512L__) || defined(__32MX795F512L__) // PIC32MX460F512L and PIC32MX795F512L PIMs has different pinout to accomodate USB module
    AD1PCFGbits.PCFG2 = 0; // Disable digital input on AN2 (potentiometer)
#else
    AD1PCFGbits.PCFG5 = 0; // Disable digital input on AN5 (potentiometer)
#endif
    AD1CON1 = 0x84E4; // Turn on, auto sample start, auto-convert, 12 bit mode (on parts with a 12bit A/D)
    AD1CON2 = 0x0404; // AVdd, AVss, int every 2 conversions, MUXA only, scan
    AD1CON3 = 0x1003; // 16 Tad auto-sample, Tad = 3*Tcy
#if defined(__32MX460F512L__) || defined(__32MX795F512L__) // PIC32MX460F512L and PIC32MX795F512L PIMs has different pinout to accomodate USB module
    AD1CSSL = 1 << 2; // Scan pot
#else
    AD1CSSL = 1 << 5; // Scan pot
#endif

    // UART
#if defined(STACK_USE_UART)
    UARTTX_TRIS = 0;
    UARTRX_TRIS = 1;
    UMODE = 0x8000; // Set UARTEN.  Note: this must be done before setting UTXEN

#if defined(__XC16)
    USTA = 0x0400; // UTXEN set
#define CLOSEST_UBRG_VALUE ((SYS_CLK_FrequencyPeripheralGet() + 8ul * BAUD_RATE) / 16 / BAUD_RATE - 1)
#define BAUD_ACTUAL (SYS_CLK_FrequencyPeripheralGet() / 16 / (CLOSEST_UBRG_VALUE + 1))
#else // defined(__XC32)
    USTA = 0x00001400; // RXEN set, TXEN set
#define CLOSEST_UBRG_VALUE ((SYS_CLK_FrequencyPeripheralGet() + 8ul * BAUD_RATE) / 16 / BAUD_RATE - 1)
#define BAUD_ACTUAL (SYS_CLK_FrequencyPeripheralGet() / 16 / (CLOSEST_UBRG_VALUE + 1))
#endif

#define BAUD_ERROR ((BAUD_ACTUAL > BAUD_RATE) ? BAUD_ACTUAL-BAUD_RATE : BAUD_RATE-BAUD_ACTUAL)
#define BAUD_ERROR_PRECENT    ((BAUD_ERROR * 100 + BAUD_RATE / 2) / BAUD_RATE)
#if (BAUD_ERROR_PRECENT > 3)
#warning UART frequency error is worse than 3%
#elif (BAUD_ERROR_PRECENT > 2)
#warning UART frequency error is worse than 2%
#endif

    UBRG = CLOSEST_UBRG_VALUE;
#endif

#endif

    // Deassert all chip select lines so there isn't any problem with
    // initialization order.  Ex: When ENC28J60 is on SPI2 with Explorer 16,
    // MAX3232 ROUT2 pin will drive RF12/U2CTS ENC28J60 CS line asserted,
    // preventing proper 25LC256 EEPROM operation.
#if defined(ENC_CS_TRIS)
    ENC_CS_IO = 1;
    ENC_CS_TRIS = 0;
#endif
#if defined(ENC100_CS_TRIS)
    ENC100_CS_IO = (ENC100_INTERFACE_MODE == 0);
    ENC100_CS_TRIS = 0;
#endif
#if defined(EEPROM_CS_TRIS)
    EEPROM_CS_IO = 1;
    EEPROM_CS_TRIS = 0;
#endif
#if defined(SPIRAM_CS_TRIS)
    SPIRAM_CS_IO = 1;
    SPIRAM_CS_TRIS = 0;
#endif
#if defined(SPIFLASH_CS_TRIS)
    SPIFLASH_CS_IO = 1;
    SPIFLASH_CS_TRIS = 0;
#endif
#if defined(WF_CS_TRIS)
    WF_CS_IO = 1;
    WF_CS_TRIS = 0;
#endif

#if defined(__PIC24FJ256GB110__)
    __builtin_write_OSCCONL(OSCCON & 0xBF); // Unlock PPS

    // Configure SPI1 PPS pins (ENC28J60/ENCX24J600/MRF24W or other PICtail Plus cards)
    RPOR0bits.RP0R = 8; // Assign RP0 to SCK1 (output)
    RPOR7bits.RP15R = 7; // Assign RP15 to SDO1 (output)
    RPINR20bits.SDI1R = 23; // Assign RP23 to SDI1 (input)

    // Configure SPI2 PPS pins (25LC256 EEPROM on Explorer 16)
    RPOR10bits.RP21R = 11; // Assign RG6/RP21 to SCK2 (output)
    RPOR9bits.RP19R = 10; // Assign RG8/RP19 to SDO2 (output)
    RPINR22bits.SDI2R = 26; // Assign RG7/RP26 to SDI2 (input)

    // Configure UART2 PPS pins (MAX3232 on Explorer 16)
#if !defined(ENC100_INTERFACE_MODE) || (ENC100_INTERFACE_MODE == 0) || defined(ENC100_PSP_USE_INDIRECT_RAM_ADDRESSING)
    RPINR19bits.U2RXR = 10; // Assign RF4/RP10 to U2RX (input)
    RPOR8bits.RP17R = 5; // Assign RF5/RP17 to U2TX (output)
#endif

    // Configure INT1 PPS pin (MRF24W Wi-Fi PICtail Plus interrupt signal when in SPI slot 1)
    RPINR0bits.INT1R = 33; // Assign RE8/RPI33 to INT1 (input)

    // Configure INT3 PPS pin (MRF24W Wi-Fi PICtail Plus interrupt signal when in SPI slot 2)
    RPINR1bits.INT3R = 40; // Assign RC3/RPI40 to INT3 (input)

    __builtin_write_OSCCONL(OSCCON | 0x40); // Lock PPS
#endif

#if defined(SPIRAM_CS_TRIS)
    SPIRAMInit();
#endif
#if defined(EEPROM_CS_TRIS)
    XEEInit();
#endif
#if defined(SPIFLASH_CS_TRIS)
    SPIFlashInit();
#endif
}

/*********************************************************************
 * Function:        void InitAppConfig(void)
 *
 * PreCondition:    MPFSInit() is already called.
 *
 * Input:           None
 *
 * Output:          Write/Read non-volatile config variables.
 *
 * Side Effects:    None
 *
 * Overview:        None
 *
 * Note:            None
 ********************************************************************/
// MAC Address Serialization using a MPLAB PM3 Programmer and
// Serialized Quick Turn Programming (SQTP).
// The advantage of using SQTP for programming the MAC Address is it
// allows you to auto-increment the MAC address without recompiling
// the code for each unit.  To use SQTP, the MAC address must be fixed
// at a specific location in program memory.  Uncomment these two pragmas
// that locate the MAC address at 0x1FFF0.  Syntax below is for MPLAB C
// Compiler for PIC18 MCUs. Syntax will vary for other compilers.
//#pragma romdata MACROM = 0x1FFF0
static ROM uint8_t SerializedMACAddress[6] = {MY_DEFAULT_MAC_BYTE1, MY_DEFAULT_MAC_BYTE2, MY_DEFAULT_MAC_BYTE3, MY_DEFAULT_MAC_BYTE4, MY_DEFAULT_MAC_BYTE5, MY_DEFAULT_MAC_BYTE6};
//#pragma romdata

static void InitAppConfig(void)
{
#if defined(EEPROM_CS_TRIS) || defined(SPIFLASH_CS_TRIS)
    unsigned char vNeedToSaveDefaults = 0;
#endif

    while (1) {
        // Start out zeroing all AppConfig bytes to ensure all fields are
        // deterministic for checksum generation
        memset((void *)&AppConfig, 0x00, sizeof(AppConfig));

        AppConfig.Flags.bIsDHCPEnabled = true;
        AppConfig.Flags.bInConfigMode = true;
        memcpypgm2ram((void *)&AppConfig.MyMACAddr, (ROM void *)SerializedMACAddress, sizeof(AppConfig.MyMACAddr));
        //        {
        //            _prog_addressT MACAddressAddress;
        //            MACAddressAddress.next = 0x157F8;
        //            _memcpy_p2d24((char *)&AppConfig.MyMACAddr, MACAddressAddress, sizeof(AppConfig.MyMACAddr));
        //        }

#if MY_DEFAULT_NETWORK_TYPE == WF_SOFT_AP
        // SoftAP on certain setups with IP 192.168.1.1 has problem with DHCP client assigning new IP address on redirection.
        // 192.168.1.1 is a common IP address with most APs. For now, assign this as 192.168.1.3
        // Conflict arises when there are 2 active DHCP servers in the same network (i.e. AP DHCP server and MRF24W EasyConfig DHCP Server).
        // When network redirection is executed, the TCPIP SW may still have the device DHCP server still active.
        // This may require change in TCPIP SW to be able to disable the local DHCP server after network redirection.
        AppConfig.MyIPAddr.Val = MY_DEFAULT_IP_ADDR_SOFTAP_BYTE1 | MY_DEFAULT_IP_ADDR_SOFTAP_BYTE2 << 8ul | MY_DEFAULT_IP_ADDR_SOFTAP_BYTE3 << 16ul | MY_DEFAULT_IP_ADDR_SOFTAP_BYTE4 << 24ul;
        AppConfig.DefaultIPAddr.Val = AppConfig.MyIPAddr.Val;
        AppConfig.MyMask.Val = MY_DEFAULT_MASK_BYTE1 | MY_DEFAULT_MASK_BYTE2 << 8ul | MY_DEFAULT_MASK_BYTE3 << 16ul | MY_DEFAULT_MASK_BYTE4 << 24ul;
        AppConfig.DefaultMask.Val = AppConfig.MyMask.Val;
        AppConfig.MyGateway.Val = AppConfig.MyIPAddr.Val;
        AppConfig.PrimaryDNSServer.Val = AppConfig.MyIPAddr.Val;
        AppConfig.SecondaryDNSServer.Val = AppConfig.MyIPAddr.Val;
#else
        AppConfig.MyIPAddr.Val = MY_DEFAULT_IP_ADDR_BYTE1 | MY_DEFAULT_IP_ADDR_BYTE2 << 8ul | MY_DEFAULT_IP_ADDR_BYTE3 << 16ul | MY_DEFAULT_IP_ADDR_BYTE4 << 24ul;
        AppConfig.DefaultIPAddr.Val = AppConfig.MyIPAddr.Val;
        AppConfig.MyMask.Val = MY_DEFAULT_MASK_BYTE1 | MY_DEFAULT_MASK_BYTE2 << 8ul | MY_DEFAULT_MASK_BYTE3 << 16ul | MY_DEFAULT_MASK_BYTE4 << 24ul;
        AppConfig.DefaultMask.Val = AppConfig.MyMask.Val;
        AppConfig.MyGateway.Val = MY_DEFAULT_GATE_BYTE1 | MY_DEFAULT_GATE_BYTE2 << 8ul | MY_DEFAULT_GATE_BYTE3 << 16ul | MY_DEFAULT_GATE_BYTE4 << 24ul;
        AppConfig.PrimaryDNSServer.Val = MY_DEFAULT_PRIMARY_DNS_BYTE1 | MY_DEFAULT_PRIMARY_DNS_BYTE2 << 8ul | MY_DEFAULT_PRIMARY_DNS_BYTE3 << 16ul | MY_DEFAULT_PRIMARY_DNS_BYTE4 << 24ul;
        AppConfig.SecondaryDNSServer.Val = MY_DEFAULT_SECONDARY_DNS_BYTE1 | MY_DEFAULT_SECONDARY_DNS_BYTE2 << 8ul | MY_DEFAULT_SECONDARY_DNS_BYTE3 << 16ul | MY_DEFAULT_SECONDARY_DNS_BYTE4 << 24ul;
#endif

        // Load the default NetBIOS Host Name
        memcpypgm2ram(AppConfig.NetBIOSName, (ROM void *)MY_DEFAULT_HOST_NAME, 16);
        FormatNetBIOSName(AppConfig.NetBIOSName);

#if defined(WF_CS_TRIS)
        WF_ASSERT(sizeof(MY_DEFAULT_SSID_NAME) - 1 <= sizeof(AppConfig.ssid));
        memcpypgm2ram(AppConfig.ssid, (ROM void *)MY_DEFAULT_SSID_NAME, sizeof(MY_DEFAULT_SSID_NAME));
        AppConfig.ssidLen = sizeof(MY_DEFAULT_SSID_NAME) - 1;
        AppConfig.networkType = MY_DEFAULT_NETWORK_TYPE;
        AppConfig.securityMode = MY_DEFAULT_WIFI_SECURITY_MODE;
        if (AppConfig.securityMode == WF_SECURITY_WEP_40) {
            AppConfig.wepIndex = 0; // This is the only WEP key index we currently support
            memcpypgm2ram(AppConfig.securityKey, (ROM void *)MY_DEFAULT_WEP_KEYS_40, sizeof(MY_DEFAULT_WEP_KEYS_40) - 1);
            AppConfig.securityKeyLength = sizeof(MY_DEFAULT_WEP_KEYS_40) - 1;
        } else if (AppConfig.securityMode == WF_SECURITY_WEP_104) {
            AppConfig.wepIndex = 0; // This is the only WEP key index we currently support
            memcpypgm2ram(AppConfig.securityKey, (ROM void *)MY_DEFAULT_WEP_KEYS_104, sizeof(MY_DEFAULT_WEP_KEYS_104) - 1);
            AppConfig.securityKeyLength = sizeof(MY_DEFAULT_WEP_KEYS_104) - 1;
        }
        AppConfig.dataValid = 0;
#endif

        // Compute the checksum of the AppConfig defaults as loaded from ROM
        wOriginalAppConfigChecksum = CalcIPChecksum((uint8_t *)&AppConfig, sizeof(AppConfig));

#if defined(EEPROM_CS_TRIS) || defined(SPIFLASH_CS_TRIS)
        {
            NVM_VALIDATION_STRUCT NVMValidationStruct;

            // Check to see if we have a flag set indicating that we need to
            // save the ROM default AppConfig values.
            if (vNeedToSaveDefaults)
                SaveAppConfig(&AppConfig);

            // Read the NVMValidation record and AppConfig struct out of EEPROM/Flash
#if defined(EEPROM_CS_TRIS)
            {
                XEEReadArray(0x0000, (uint8_t *)&NVMValidationStruct, sizeof(NVMValidationStruct));
                XEEReadArray(sizeof(NVMValidationStruct), (uint8_t *)&AppConfig, sizeof(AppConfig));
            }
#elif defined(SPIFLASH_CS_TRIS)
            {
                SPIFlashReadArray(0x0000, (uint8_t *)&NVMValidationStruct, sizeof(NVMValidationStruct));
                SPIFlashReadArray(sizeof(NVMValidationStruct), (uint8_t *)&AppConfig, sizeof(AppConfig));
            }
#endif

            // Check EEPROM/Flash validity.  If it isn't valid, set a flag so
            // that we will save the ROM default values on the next loop
            // iteration.
            if ((NVMValidationStruct.wConfigurationLength != sizeof(AppConfig)) ||
                (NVMValidationStruct.wOriginalChecksum != wOriginalAppConfigChecksum) ||
                (NVMValidationStruct.wCurrentChecksum != CalcIPChecksum((uint8_t *)&AppConfig, sizeof(AppConfig)))) {
                // Check to ensure that the vNeedToSaveDefaults flag is zero,
                // indicating that this is the first iteration through the do
                // loop.  If we have already saved the defaults once and the
                // EEPROM/Flash still doesn't pass the validity check, then it
                // means we aren't successfully reading or writing to the
                // EEPROM/Flash.  This means you have a hardware error and/or
                // SPI configuration error.
                if (vNeedToSaveDefaults) {
                    WF_ASSERT(false);
                }

                // Set flag and restart loop to load ROM defaults and save them
                vNeedToSaveDefaults = 1;
                continue;
            }

            // If we get down here, it means the EEPROM/Flash has valid contents
            // and either matches the ROM defaults or previously matched and
            // was run-time reconfigured by the user.  In this case, we shall
            // use the contents loaded from EEPROM/Flash.
            break;
        }
#endif
        break;
    }

#if defined (EZ_CONFIG_STORE)
    // Set configuration for ZG from NVM
    /* Set security type and key if necessary, convert from app storage to ZG driver */
    if (AppConfig.dataValid)
        CFGCXT.isWifiDoneConfigure = 1;

    AppConfig.saveSecurityInfo = false;
#endif // EZ_CONFIG_STORE
}

#if defined(EEPROM_CS_TRIS) || defined(SPIFLASH_CS_TRIS)

void SaveAppConfig(const APP_CONFIG *ptrAppConfig)
{
    NVM_VALIDATION_STRUCT NVMValidationStruct;

    // Ensure adequate space has been reserved in non-volatile storage to
    // store the entire AppConfig structure.  If you get stuck in this while (1)
    // trap, it means you have a design time misconfiguration in tcpip_config.h.
    // You must increase MPFS_RESERVE_BLOCK to allocate more space.
#if defined(STACK_USE_MPFS2)
    if (sizeof(NVMValidationStruct) + sizeof(AppConfig) > MPFS_RESERVE_BLOCK)
        WF_ASSERT(false);
#endif

    // Get proper values for the validation structure indicating that we can use
    // these EEPROM/Flash contents on future boot ups
    NVMValidationStruct.wOriginalChecksum = wOriginalAppConfigChecksum;
    NVMValidationStruct.wCurrentChecksum = CalcIPChecksum((uint8_t *)ptrAppConfig, sizeof(APP_CONFIG));
    NVMValidationStruct.wConfigurationLength = sizeof(APP_CONFIG);

    // Write the validation struct and current AppConfig contents to EEPROM/Flash
#if defined(EEPROM_CS_TRIS)
    XEEBeginWrite(0x0000);
    XEEWriteArray((uint8_t *)&NVMValidationStruct, sizeof(NVMValidationStruct));
    XEEWriteArray((uint8_t *)ptrAppConfig, sizeof(APP_CONFIG));
#else
    SPIFlashBeginWrite(0x0000);
    SPIFlashWriteArray((uint8_t *)&NVMValidationStruct, sizeof(NVMValidationStruct));
    SPIFlashWriteArray((uint8_t *)ptrAppConfig, sizeof(APP_CONFIG));
#endif
}
#endif

#if defined (EZ_CONFIG_STORE)
void RestoreWifiConfig(void)
{
#if defined(STACK_USE_UART)
    putrsUART((ROM char *)"\r\nButton push, restore wifi configuration!!!\r\n");
#endif

#if defined(EEPROM_CS_TRIS)
    XEEBeginWrite(0x0000);
    XEEWrite(0xFF);
    XEEWrite(0xFF);
    XEEEndWrite();
#elif defined(SPIFLASH_CS_TRIS)
    SPIFlashBeginWrite(0x0000);
    SPIFlashWrite(0xFF);
    SPIFlashWrite(0xFF);
#endif

    // reboot here...
    LED_PUT(0x00);
    while (BUTTON3_IO == 0u);
    Reset();
}
#endif // EZ_CONFIG_STORE
