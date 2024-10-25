/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    main.c

  Summary:
    Main Application Entry Point

  Description:
    -Demonstrates how to call and use the Microchip WiFi Module and
     TCP/IP stack
    -Reference: Microchip TCP/IP Stack Help

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

#include <stdint.h>
#include "main.h"

//
// Differences to wifi comm demo board (MRF24WB0MA):
//        Wifi comm demo SSID : MCHP_xxxx
//        Wifi comm demo board is centered on variable CPElements.
//        SW0 functions : On powerup initiates self test.
//
//        Wifi G demo SSID : MCHP_G_xxxx
//        Wifi G demo board is centered on variable AppConfig, since this is the generic approach adopted by
//          TCPIP Demo App/Console Demo/EZConfig.
//        SW0 functions : On powerup initiates self test.
//          When running, initiates reboot to factory default conditions.
//

//
//  Wifi G Demo Web Pages
//  Generate a mpfs_img2.c file using the mpfs2 utility.
//  The mpfs_img2.c file gets compiled into source code and programmed into the flash.

APP_CONFIG AppConfig;

static unsigned short wOriginalAppConfigChecksum; // Checksum of the ROM defaults for AppConfig
extern unsigned char TelnetPut(unsigned char c);

uint8_t g_scan_done = 0; // WF_PRESCAN  This will be set wheneven event scan results are ready.
uint8_t g_prescan_waiting = 1; // WF_PRESCAN  This is used only to allow POR prescan once.

// Private helper functions.
static void InitAppConfig(void);
static void InitializeBoard(void);
static void SelfTest(void);
extern void WF_Connect(void);
void UARTTxBuffer(char *buffer, uint32_t size);

// Used for re-directing printf and UART statements to the Telnet daemon
void _mon_putc(char c)
{
#ifdef STACK_USE_TELNET_SERVER
    TelnetPut(c);
#endif
}

// Exception Handlers
// If your code gets here, you either tried to read or write
// a NULL pointer, or your application overflowed the stack
// by having too many local variables or parameters declared.
void _general_exception_handler(unsigned cause, unsigned status)
{
    Nop();
    Nop();
}

// ************************************************************
// Main application entry point.
// ************************************************************
int main(void)
{
    static uint32_t t = 0;
    static uint32_t dwLastIP = 0;

#if defined (EZ_CONFIG_STORE)
    static uint32_t ButtonPushStart = 0;
#endif

    uint8_t channelList[] = MY_DEFAULT_CHANNEL_LIST_PRESCAN; // WF_PRESCAN
    tWFScanResult bssDesc;

#if 0
    int8_t TxPower; // Needed to change MRF24WG transmit power.
#endif

    // Initialize application specific hardware
    InitializeBoard();

    // Initialize TCP/IP stack timer
    TickInit();

#if defined(STACK_USE_MPFS2)
    // Initialize the MPFS File System
    // Generate a WifiGDemoMPFSImg.c file using the MPFS utility (refer to Convert WebPages to MPFS.bat)
    // that gets compiled into source code and programmed into the flash of the uP.
    MPFSInit();
#endif

    // Initialize Stack and application related NV variables into AppConfig.
    InitAppConfig();

    // Initialize core stack layers (MAC, ARP, TCP, UDP) and
    // application modules (HTTP, etc.)
    StackInit();

#if 0
    // Below is used to change MRF24WG transmit power.
    // This has been verified to be functional (Jan 2013)
    if (AppConfig.networkType == WF_SOFT_AP) {
        WF_TxPowerGetMax(&TxPower);
        WF_TxPowerSetMax(TxPower);
    }
#endif

    // Run Self Test if SW0 pressed on startup
    if (SW0_IO == 0)
        SelfTest();

#ifdef STACK_USE_TELNET_SERVER
    // Initialize Telnet and
    // Put Remote client in Remote Character Echo Mode
    TelnetInit();
    putc(0xff, stdout); // IAC = Interpret as Command
    putc(0xfe, stdout); // Type of Operation = DONT
    putc(0x22, stdout); // Option = linemode
    putc(0xff, stdout); // IAC = Interpret as Command
    putc(0xfb, stdout); // Type of Operation = DO
    putc(0x01, stdout); // Option = echo
#endif

#if defined ( EZ_CONFIG_SCAN )
    // Initialize Wi-Fi Scan State Machine NV variables
    WFInitScan();
#endif

    // WF_PRESCAN: Pre-scan before starting up as SoftAP mode
    WF_CASetScanType(MY_DEFAULT_SCAN_TYPE);
    WF_CASetChannelList(channelList, sizeof(channelList));

    if (WFStartScan() == WF_SUCCESS) {
        SCAN_SET_DISPLAY(SCANCXT.scanState);
        SCANCXT.displayIdx = 0;
    }

    // Needed to trigger g_scan_done
    WFRetrieveScanResult(0, &bssDesc);

#if defined(STACK_USE_ZEROCONF_LINK_LOCAL)
    // Initialize Zeroconf Link-Local state-machine, regardless of network type.
    ZeroconfLLInitialize();
#endif

#if defined(STACK_USE_ZEROCONF_MDNS_SD)
    // Initialize DNS Host-Name from tcpip_config.h, regardless of network type.
    mDNSInitialize(MY_DEFAULT_HOST_NAME);
    mDNSServiceRegister(
            //(const char *)AppConfig.NetBIOSName, // base name of the service. Ensure uniformity with CheckHibernate().
            (const char *)"DemoWebServer", // base name of the service. Ensure uniformity with CheckHibernate().
            "_http._tcp.local", // type of the service
            80, // TCP or UDP port, at which this service is available
            ((const uint8_t *)"path=/index.htm"), // TXT info
            1, // auto rename the service when if needed
            NULL, // no callback function
            NULL // no application context
            );
    mDNSMulticastFilterRegister();
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
        if (AppConfig.networkType == WF_SOFT_AP) {
            if (g_scan_done) {
                if (g_prescan_waiting) {
                    SCANCXT.displayIdx = 0;
                    while (IS_SCAN_STATE_DISPLAY(SCANCXT.scanState)) {
                        WFDisplayScanMgr();
                    }

#if defined(WF_CS_TRIS)
                    WF_Connect();
#endif
                    g_scan_done = 0;
                    g_prescan_waiting = 0;
                }
            }
        }

#if defined (EZ_CONFIG_STORE)
        // Hold SW0 for 4 seconds to reset to defaults.
        if (SW0_IO == 0u) { // Button is pressed
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

        // Blink LED0 twice per sec when unconfigured, once per sec after config
        if ((TickGet() - t >= TICK_SECOND / (4ul - (CFGCXT.isWifiDoneConfigure * 2ul)))) {
            t = TickGet();
#ifdef LED_STOP_BLINKING_IF_CONNECTION_FAILED
            // If connection is failed, keep LED0 on
            if (AppConfig.connectionFailedFlag == 1) {
                LED0_ON();
            } else {
                LED0_INV();
            }
#else
            LED0_INV();
#endif
        }

        // This task performs normal stack task including checking
        // for incoming packet, type of packet and calling
        // appropriate stack entity to process it.
        StackTask();

        // This task invokes each of the core stack application tasks
        StackApplications();

        // Enable WF_USE_POWER_SAVE_FUNCTIONS
        WiFiTask();

#if defined(STACK_USE_ZEROCONF_LINK_LOCAL)
        ZeroconfLLProcess();
#endif

#if defined(STACK_USE_ZEROCONF_MDNS_SD)
        mDNSProcess();
#endif

        // Process application specific tasks here.
        // Any custom modules or processing you need to do should
        // go here.

        // If the local IP address has changed (ex: due to DHCP lease change)
        // write the new IP address to the LCD display, UART, and Announce
        // service
        if (dwLastIP != AppConfig.MyIPAddr.Val) {
            dwLastIP = AppConfig.MyIPAddr.Val;
            DisplayIPValue(AppConfig.MyIPAddr);

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
    // Note: WiFi Module hardware Initialization handled by StackInit() Library Routine

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
    DDPCONbits.JTAGEN = 0;

    // LEDs
    LEDS_OFF();
    LED0_TRIS = 0;
    LED1_TRIS = 0;
    LED2_TRIS = 0;

    // Push Button
    SW0_TRIS = 1;
}

static ROM uint8_t SerializedMACAddress[6] = {MY_DEFAULT_MAC_BYTE1, MY_DEFAULT_MAC_BYTE2, MY_DEFAULT_MAC_BYTE3, MY_DEFAULT_MAC_BYTE4, MY_DEFAULT_MAC_BYTE5, MY_DEFAULT_MAC_BYTE6};

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

        // SoftAP on certain setups with IP 192.168.1.1 has problem with DHCP client assigning new IP address on redirection.
        // 192.168.1.1 is a common IP address with most APs. This is still under investigation.
        // For now, assign this as 192.168.1.3 (you can configure this in tcpip_config.h)

        AppConfig.MyIPAddr.Val = MY_DEFAULT_IP_ADDR_BYTE1 | MY_DEFAULT_IP_ADDR_BYTE2 << 8ul | MY_DEFAULT_IP_ADDR_BYTE3 << 16ul | MY_DEFAULT_IP_ADDR_BYTE4 << 24ul;
        AppConfig.DefaultIPAddr.Val = AppConfig.MyIPAddr.Val;
        AppConfig.MyMask.Val = MY_DEFAULT_MASK_BYTE1 | MY_DEFAULT_MASK_BYTE2 << 8ul | MY_DEFAULT_MASK_BYTE3 << 16ul | MY_DEFAULT_MASK_BYTE4 << 24ul;
        AppConfig.DefaultMask.Val = AppConfig.MyMask.Val;
        AppConfig.MyGateway.Val = AppConfig.MyIPAddr.Val;
        AppConfig.PrimaryDNSServer.Val = AppConfig.MyIPAddr.Val;
        AppConfig.SecondaryDNSServer.Val = AppConfig.MyIPAddr.Val;

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

#if defined(EEPROM_CS_TRIS)
        NVM_VALIDATION_STRUCT NVMValidationStruct;

        // Check to see if we have a flag set indicating that we need to
        // save the ROM default AppConfig values.
        if (vNeedToSaveDefaults)
            SaveAppConfig(&AppConfig);

        // Read the NVMValidation record and AppConfig struct out of EEPROM/Flash
        XEEReadArray(0x0000, (uint8_t *)&NVMValidationStruct, sizeof(NVMValidationStruct));
        XEEReadArray(sizeof(NVMValidationStruct), (uint8_t *)&AppConfig, sizeof(AppConfig));

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

/****************************************************************************
  Function:
    void SelfTest()

  Description:
    This routine performs a self test of the hardware.

  Precondition:
    None

  Parameters:
    None - None

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
static void SelfTest(void)
{
    char value = 0;
    char * buf[32];

    // Configure Sensor Serial Port
    UARTConfigure(SENSOR_UART, UART_ENABLE_PINS_TX_RX_ONLY);
    UARTSetFifoMode(SENSOR_UART, UART_INTERRUPT_ON_TX_NOT_FULL | UART_INTERRUPT_ON_RX_NOT_EMPTY);
    UARTSetLineControl(SENSOR_UART, UART_DATA_SIZE_8_BITS | UART_PARITY_NONE | UART_STOP_BITS_1);
    UARTSetDataRate(SENSOR_UART, SYS_CLK_FrequencyPeripheralGet(), 9600);
    UARTEnable(SENSOR_UART, UART_ENABLE_FLAGS(UART_PERIPHERAL | UART_RX | UART_TX));

    // Verify MRF24WB/G0MA MAC Address
    if (AppConfig.MyMACAddr.v[0] == 0x00 && AppConfig.MyMACAddr.v[1] == 0x1E) {
        //********************************************************************
        // Prints a label using ESC/P commands to a Brother PT-9800PCN printer
        //********************************************************************
        // Send ESC/P Commands to setup printer
        UARTTxBuffer("\033ia\000\033@\033X\002", 9); // ESC i a 0 = Put Printer in ESC/P Mode
        // ESC @ = Reset Printer to Default settings
        // ESC X 2 = Specify Character Size
        // Send the Info to Print for the MAC Address label
        // UARTTxBuffer("MRF24WB0MA\r",11); // Remove "MRF24WBOMA from label.
        sprintf((char *)buf, "MAC: %02X%02X%02X%02X%02X%02X", AppConfig.MyMACAddr.v[0], AppConfig.MyMACAddr.v[1], AppConfig.MyMACAddr.v[2], AppConfig.MyMACAddr.v[3], AppConfig.MyMACAddr.v[4], AppConfig.MyMACAddr.v[5]);
        UARTTxBuffer((char *)buf, strlen((const char *)buf));

        // Print the label
        UARTTxBuffer("\f", 1);

        // Toggle LED's
        while (1) {
            LED0_IO = value;
            LED1_IO = value >> 1;
            LED2_IO = value >> 2;

            DelayMs(400);

            if (value == 8)
                value = 0;
            else
                value++;
        }
    } else // MRF24WG0MA Failure
    {
        while (1) {
            LEDS_ON();
            DelayMs(700);
            LEDS_OFF();
            DelayMs(700);
        }
    }
}

/****************************************************************************
  Function:
    void UARTTxBuffer(char *buffer, uint32_t size)

  Description:
    This routine sends data out the Sensor UART port.

  Precondition:
    None

  Parameters:
    None - None

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
void UARTTxBuffer(char *buffer, uint32_t size)
{
    while (size) {
        while (!UARTTransmitterIsReady(SENSOR_UART))
            ;

        UARTSendDataByte(SENSOR_UART, *buffer);

        buffer++;
        size--;
    }

    while (!UARTTransmissionHasCompleted(SENSOR_UART));
}

/****************************************************************************
  Function:
    void DisplayIPValue(IP_ADDR IPVal)

  Description:
    This routine formats and prints the current IP Address.

  Precondition:
    None

  Parameters:
    None - None

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
void DisplayIPValue(IP_ADDR IPVal)
{
    printf("%u.%u.%u.%u", IPVal.v[0], IPVal.v[1], IPVal.v[2], IPVal.v[3]);
}

/****************************************************************************
  Function:
    void SaveAppConfig(const APP_CONFIG *ptrAppConfig)

  Description:
    This routine saves the AppConfig into EEPROM.

  Precondition:
    None

  Parameters:
    None - None

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
void SaveAppConfig(const APP_CONFIG *ptrAppConfig)
{
#if 0 // Wi-Fi G demo does not have NVM
    NVM_VALIDATION_STRUCT NVMValidationStruct;

    // Get proper values for the validation structure indicating that we can use
    // these EEPROM/Flash contents on future boot ups
    NVMValidationStruct.wOriginalChecksum = wOriginalAppConfigChecksum;
    NVMValidationStruct.wCurrentChecksum = CalcIPChecksum((uint8_t *)ptrAppConfig, sizeof(APP_CONFIG));
    NVMValidationStruct.wConfigurationLength = sizeof(APP_CONFIG);

    // Write the validation struct and current AppConfig contents to EEPROM/Flash
    XEEBeginWrite(0x0000);
    XEEWriteArray((uint8_t *)&NVMValidationStruct, sizeof(NVMValidationStruct));
    XEEWriteArray((uint8_t *)ptrAppConfig, sizeof(APP_CONFIG));
#endif
}

#if defined (EZ_CONFIG_STORE)

/****************************************************************************
  Function:
    void RestoreWifiConfig(void)

  Description:
    This routine performs reboot when SW0 is pressed.

  Precondition:
    None

  Parameters:
    None - None

  Returns:
    None

  Remarks:
    None
 ***************************************************************************/
void RestoreWifiConfig(void)
{
#if 0
    XEEBeginWrite(0x0000);
    XEEWrite(0xFF);
    XEEWrite(0xFF);
    XEEEndWrite();
#endif
    // reboot here...
    //LED_PUT(0x00);
    while (SW0_IO == 0u);
    Reset();
}

#endif // EZ_CONFIG_STORE
