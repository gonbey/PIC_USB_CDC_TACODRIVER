/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    custom_http_app.c

  Summary:
    Support for HTTP2 module in Microchip TCP/IP Stack
    -Implements the application
    -Reference: RFC 1002

  Description:
    Application to Demo HTTP2 Server
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

#define __CUSTOMHTTPAPP_C

#include "system_config.h"

#if defined(STACK_USE_HTTP2_SERVER)

#include "tcpip/tcpip.h"
#include "driver/wifi/mrf24w/src/drv_wifi_console_msg_handler.h"
#include "main.h" // Needed for SaveAppConfig() prototype

/****************************************************************************
  Section:
    Function Prototypes and Memory Globalizers
 ***************************************************************************/
#if defined(HTTP_USE_POST)
#if defined(USE_LCD)
static HTTP_IO_RESULT HTTPPostLCD(void);
#endif
#if defined(STACK_USE_HTTP_MD5_DEMO)
#if !defined(STACK_USE_MD5)
#error The HTTP_MD5_DEMO requires STACK_USE_MD5
#endif
static HTTP_IO_RESULT HTTPPostMD5(void);
#endif
#if defined(STACK_USE_EZ_CONFIG)
static HTTP_IO_RESULT HTTPPostWifiConfig(void);
#endif
#if defined(STACK_USE_HTTP_APP_RECONFIG)
extern APP_CONFIG AppConfig;
static HTTP_IO_RESULT HTTPPostConfig(void);
#endif
#if defined(STACK_USE_HTTP_EMAIL_DEMO) || defined(STACK_USE_SMTP_CLIENT)
#if !defined(STACK_USE_SMTP_CLIENT)
#error The HTTP_EMAIL_DEMO requires STACK_USE_SMTP_CLIENT
#endif
static HTTP_IO_RESULT HTTPPostEmail(void);
#endif
#if defined(STACK_USE_DYNAMICDNS_CLIENT)
static HTTP_IO_RESULT HTTPPostDDNSConfig(void);
#endif
#endif

// bss descriptor fetched from on-chip
static tWFScanResult bssDesc;
static bool bssDescIsValid = false;

// RAM allocated for DDNS parameters
#if defined(STACK_USE_DYNAMICDNS_CLIENT)
static uint8_t DDNSData[100];
#endif

// Sticky status message variable.
// This is used to indicated whether or not the previous POST operation was
// successful.  The application uses these to store status messages when a
// POST operation redirects.  This lets the application provide status messages
// after a redirect, when connection instance data has already been lost.
static bool lastSuccess = false;

// Stick status message variable.  See lastSuccess for details.
static bool lastFailure = false;

#if (MY_DEFAULT_NETWORK_TYPE == WF_SOFT_AP) || defined(WF_PRE_SCAN_IN_ADHOC)
extern tWFScanResult preScanResult[]; // WF_PRESCAN
#endif

/****************************************************************************
  Section:
    Authorization Handlers
 ***************************************************************************/

/*****************************************************************************
  Function:
    uint8_t HTTPNeedsAuth(uint8_t * cFile)

  Internal:
    See documentation in the TCP/IP Stack API or http2.h for details.
 ***************************************************************************/
#if defined(HTTP_USE_AUTHENTICATION)
uint8_t HTTPNeedsAuth(uint8_t * cFile)
{
    // If the filename begins with the folder "protect", then require auth
    if (memcmppgm2ram(cFile, (ROM void *) "protect", 7) == 0)
        return 0x00; // Authentication will be needed later

#if defined(HTTP_MPFS_UPLOAD_REQUIRES_AUTH)
    if (memcmppgm2ram(cFile, (ROM void *) "mpfsupload", 10) == 0)
        return 0x00;
#endif

    // You can match additional strings here to password protect other files.
    // You could switch this and exclude files from authentication.
    // You could also always return 0x00 to require auth for all files.
    // You can return different values (0x00 to 0x79) to track "realms" for below.

    return 0x80; // No authentication required
}
#endif

/*****************************************************************************
  Function:
    uint8_t HTTPCheckAuth(uint8_t * cUser, uint8_t * cPass)

  Internal:
    See documentation in the TCP/IP Stack API or http2.h for details.
 ***************************************************************************/
#if defined(HTTP_USE_AUTHENTICATION)
uint8_t HTTPCheckAuth(uint8_t * cUser, uint8_t * cPass)
{
    if (strcmppgm2ram((char *) cUser, (ROM char *) "admin") == 0
            && strcmppgm2ram((char *) cPass, (ROM char *) "microchip") == 0)
        return 0x80; // We accept this combination

    // You can add additional user/pass combos here.
    // If you return specific "realm" values above, you can base this
    //   decision on what specific file or folder is being accessed.
    // You could return different values (0x80 to 0xff) to indicate
    //   various users or groups, and base future processing decisions
    //   in HTTPExecuteGet/Post or HTTPPrint callbacks on this value.

    return 0x00; // Provided user/pass is invalid
}
#endif

/****************************************************************************
  Section:
    GET Form Handlers
 ***************************************************************************/

/*****************************************************************************
  Function:
    HTTP_IO_RESULT HTTPExecuteGet(void)

  Internal:
      See documentation in the TCP/IP Stack API or http2.h for details.
 ***************************************************************************/
HTTP_IO_RESULT HTTPExecuteGet(void)
{
    uint8_t *ptr, *ptr1;
    uint8_t filename[20];

    uint8_t bssIdx;
    TCPIP_UINT16_VAL bssIdxStr;

    // Load the file name
    // Make sure uint8_t filename[] above is large enough for your longest name
    MPFSGetFilename(curHTTP.file, filename, 20);

    // If its the forms.htm page
    if (!memcmppgm2ram(filename, "forms.htm", 9)) {
        // Seek out each of the four LED strings, and if it exists set the LED states
        ptr = HTTPGetROMArg(curHTTP.data, (ROM uint8_t *) "led4");
        if (ptr)
            LED4_IO = (*ptr == '1');

        ptr = HTTPGetROMArg(curHTTP.data, (ROM uint8_t *) "led3");
        if (ptr)
            LED3_IO = (*ptr == '1');

        ptr = HTTPGetROMArg(curHTTP.data, (ROM uint8_t *) "led2");
        if (ptr)
            LED2_IO = (*ptr == '1');

        ptr = HTTPGetROMArg(curHTTP.data, (ROM uint8_t *) "led1");
        if (ptr)
            LED1_IO = (*ptr == '1');
    } else if (!memcmppgm2ram(filename, "cookies.htm", 11)) { // If it's the LED updater file
        // This is very simple.  The names and values we want are already in
        // the data array.  We just set the hasArgs value to indicate how many
        // name/value pairs we want stored as cookies.
        // To add the second cookie, just increment this value.
        // remember to also add a dynamic variable callback to control the printout.
        curHTTP.hasArgs = 0x01;
    } else if (!memcmppgm2ram(filename, "leds.cgi", 8)) { // If it's the LED updater file
        // Determine which LED to toggle
        ptr = HTTPGetROMArg(curHTTP.data, (ROM uint8_t *) "led");

        // Toggle the specified LED
        switch (*ptr) {
        case '1':
            LED1_IO ^= 1;
            break;
        case '2':
            LED2_IO ^= 1;
            break;
        case '3':
            LED3_IO ^= 1;
            break;
        case '4':
            LED4_IO ^= 1;
            break;
        case '5':
            LED5_IO ^= 1;
            break;
        case '6':
            LED6_IO ^= 1;
            break;
        case '7':
            LED7_IO ^= 1;
            break;
        }
    } else if (!memcmppgm2ram(filename, "scan.cgi", 8)) {
        ptr = HTTPGetROMArg(curHTTP.data, (ROM uint8_t *) "scan");
        ptr1 = HTTPGetROMArg(curHTTP.data, (ROM uint8_t *) "getBss");

        if ((ptr != NULL) && (ptr1 == NULL)) {
            bssDescIsValid = false;
            // scan request
#if (MY_DEFAULT_NETWORK_TYPE == WF_SOFT_AP)
            if (AppConfig.networkType == CFG_WF_SOFT_AP) { // SoftAP: display pre-scan results before starting as SoftAP. SoftAP does not scan.
                // putsUART("HTTPExecuteGet: SoftAP ... bypass scan  ................. \r\n");
            } else {
                if (WFStartScan() == WF_SUCCESS) {
                    // putsUART("HTTPExecuteGet: WFStartScan success ................. \r\n");
                    SCAN_SET_DISPLAY(SCANCXT.scanState);
                    SCANCXT.displayIdx = 0;
                }
                //else
                //   putsUART("HTTPExecuteGet: WFStartScan fails ................. \r\n");
            }
#else
            if (WFStartScan() == WF_SUCCESS) {
                SCAN_SET_DISPLAY(SCANCXT.scanState);
                SCANCXT.displayIdx = 0;
            }
#endif
        } else if ((ptr == NULL) && (ptr1 != NULL)) {
            // getBss request
            // use the value to get the nth bss stored on chip
            bssDescIsValid = false;
            bssIdxStr.v[1] = *ptr1;
            bssIdxStr.v[0] = *(ptr1 + 1);
            bssIdx = hexatob(bssIdxStr);

#if defined(WF_PRE_SCAN_IN_ADHOC)
            bssDesc = preScanResult[bssIdx];
#else /* !defined(WF_PRE_SCAN_IN_ADHOC) */
#if (MY_DEFAULT_NETWORK_TYPE == WF_SOFT_AP)
            if (AppConfig.networkType == CFG_WF_SOFT_AP) { // SoftAP: display pre-scan results before starting as SoftAP. SoftAP does not scan.
                bssDesc = preScanResult[bssIdx];
                //putsUART("HTTPExecuteGet: SoftAP ... display pre-scan  ................. \r\n");
            } else {
                WFRetrieveScanResult(bssIdx, &bssDesc);
            }
#else
            WFRetrieveScanResult(bssIdx, &bssDesc);
#endif
#endif /* defined(WF_PRE_SCAN_IN_ADHOC) */

            bssDescIsValid = true;
        } else {
            // impossible to get here
        }
    } else {
    }

    return HTTP_IO_DONE;
}

/****************************************************************************
  Section:
    POST Form Handlers
 ***************************************************************************/
#if defined(HTTP_USE_POST)
/*****************************************************************************
  Function:
    HTTP_IO_RESULT HTTPExecutePost(void)

  Internal:
    See documentation in the TCP/IP Stack API or http2.h for details.
 ***************************************************************************/
HTTP_IO_RESULT HTTPExecutePost(void)
{
    // Resolve which function to use and pass along
    uint8_t filename[20];

    // Load the file name
    // Make sure uint8_t filename[] above is large enough for your longest name
    MPFSGetFilename(curHTTP.file, filename, sizeof (filename));

#if defined(USE_LCD)
    if (!memcmppgm2ram(filename, "forms.htm", 9))
        return HTTPPostLCD();
#endif

#if defined(STACK_USE_EZ_CONFIG)
    if (!memcmppgm2ram(filename, "configure.htm", 13))
        return HTTPPostWifiConfig();
#endif

#if defined(STACK_USE_HTTP_MD5_DEMO)
    if (!memcmppgm2ram(filename, "upload.htm", 10))
        return HTTPPostMD5();
#endif

#if defined(STACK_USE_HTTP_APP_RECONFIG)
    if (!memcmppgm2ram(filename, "protect/config.htm", 18))
        return HTTPPostConfig();
#endif

#if defined(STACK_USE_SMTP_CLIENT)
    if (!strcmppgm2ram((char *) filename, "email/index.htm"))
        return HTTPPostEmail();
#endif

#if defined(STACK_USE_DYNAMICDNS_CLIENT)
    if (!strcmppgm2ram((char *) filename, "dyndns/index.htm"))
        return HTTPPostDDNSConfig();
#endif

    return HTTP_IO_DONE;
}

/*****************************************************************************
  Function:
    static HTTP_IO_RESULT HTTPPostWifiConfig(void)

  Summary:
    Processes the wifi config data

  Description:
    Accepts wireless configuration data from the www site and saves them to a
    structure to be applied by the ZG configuration manager.

    The following configurations are possible:
         i) Mode: adhoc or infrastructure
        ii) Security:
               - None
               - WPA Auto passphrase
               - WPA Auto pre-calculated key
               - WEP 64-bit
               - WEP 128-bit
       iii) Key material

    If an error occurs, such as data is invalid they will be redirected to a page
    informing the user of such results.

    NOTE: This code for modified originally from HTTPPostWifiConfig as distributed
          by Microchip.

  Precondition:
    None

  Parameters:
    None

  Return Values:
    HTTP_IO_DONE - all parameters have been processed
    HTTP_IO_NEED_DATA - data needed by this function has not yet arrived
 ***************************************************************************/
#if defined(STACK_USE_EZ_CONFIG)
static HTTP_IO_RESULT HTTPPostWifiConfig(void)
{
    // Check to see if the browser is attempting to submit more data than we
    // can parse at once.  This function needs to receive all updated
    // parameters and validate them all before committing them to memory so that
    // orphaned configuration parameters do not get written (for example, if a
    // static IP address is given, but the subnet mask fails parsing, we
    // should not use the static IP address).  Everything needs to be processed
    // in a single transaction.  If this is impossible, fail and notify the user.
    // As a web developer, if you add parameters to AppConfig and run into this
    // problem, you could fix this by to splitting your update web page into two
    // separate web pages (causing two transactional writes).  Alternatively,
    // you could fix it by storing a static shadow copy of AppConfig someplace
    // in memory and using it instead of newAppConfig.  Lastly, you could
    // increase the TCP RX FIFO size for the HTTP server.  This will allow more
    // data to be POSTed by the web browser before hitting this limit.

    uint8_t ssidLen;

    if (curHTTP.byteCount > TCPIsGetReady(sktHTTP) + TCPGetRxFIFOFree(sktHTTP))
        goto ConfigFailure;

    // Ensure that all data is waiting to be parsed.  If not, keep waiting for
    // all of it to arrive.
    if (TCPIsGetReady(sktHTTP) < curHTTP.byteCount)
        return HTTP_IO_NEED_DATA;

    // Read all browser POST data
    while (curHTTP.byteCount) {
        // Read a form field name
        if (HTTPReadPostName(curHTTP.data, 6) != HTTP_READ_OK)
            goto ConfigFailure;

        // Read a form field value
        if (HTTPReadPostValue(curHTTP.data + 6, sizeof (curHTTP.data) - 6 - 2) != HTTP_READ_OK)
            goto ConfigFailure;

        // Parse the value that was read
        if (!strcmppgm2ram((char *) curHTTP.data, "sec")) { // Read security type
            char security_type[7];

            if (strlen((char *) (curHTTP.data + 6)) > 6) /* Sanity check */
                goto ConfigFailure;

            memcpy(security_type, (void *) (curHTTP.data + 6), strlen((char *) (curHTTP.data + 6)));
            security_type[strlen((char *) (curHTTP.data + 6))] = 0; /* Terminate string */

            if (!strcmppgm2ram((char *) security_type, "no")) {
                CFGCXT.securityMode = WF_SECURITY_OPEN;
                //putrsUART((ROM char *)"\r\nSelect open on www! ");
            } else if (!strcmppgm2ram((char *) security_type, "wpa")) {
                CFGCXT.securityMode = WF_SECURITY_WPA_AUTO_WITH_PASS_PHRASE;
                //putrsUART((ROM char *)"\r\nWPA passphrase! ");
            } else if (!strcmppgm2ram((char *) security_type, "calc")) { /* Pre-calculated key */
                CFGCXT.securityMode = WF_SECURITY_WPA_AUTO_WITH_KEY;
            } else if (!strcmppgm2ram((char *) security_type, "wep40")) {
                CFGCXT.securityMode = WF_SECURITY_WEP_40;
                //putrsUART((ROM char *)"\r\nSelect wep64 on www! ");
            } else if (!strcmppgm2ram((char *) security_type, "wep104")) {
                CFGCXT.securityMode = WF_SECURITY_WEP_104;
                //putrsUART((ROM char *)"\r\nSelect wep128 on www! ");
            } else { //Security type no good  :-(
                //memset(LCDText, ' ', 32);
                //strcpy((char *)LCDText, (char *)security_type);
                //LCDUpdate();
                //putrsUART((ROM char *)"\r\nUnknown key type on www! ");
                goto ConfigFailure;
            }
        } else if (!strcmppgm2ram((char *) curHTTP.data, "key")) { // Read new key material
            uint8_t key_size = 0, ascii_key = 0;

            switch ((uint8_t) CFGCXT.securityMode) {
            case WF_SECURITY_OPEN: // keep compiler happy, nothing to do here!
                break;
            case WF_SECURITY_WPA_AUTO_WITH_PASS_PHRASE: // wpa passphrase
                //putrsUART((ROM char *)"\r\nPassphrase type of key! ");
                ascii_key = 1;
                key_size = strlen((char *) (curHTTP.data + 6));
                // between 8-63 characters, passphrase
                if ((key_size < 8) || (key_size > 63))
                    goto ConfigFailure;
                break;
            case WF_SECURITY_WPA_AUTO_WITH_KEY: // wpa pre-calculated key!!!
                key_size = 64;
                //memset(LCDText, ' ', 32);
                //strcpy((char *)LCDText, (char *)(curHTTP.data + 6));
                //LCDUpdate();
                break;
            case WF_SECURITY_WEP_40:
                key_size = 10; /* Assume hex size */
                if (strlen((char *) (curHTTP.data + 6)) == 5) {
                    key_size = 5; /* ASCII key support */
                    ascii_key = 1;
                }
                CFGCXT.wepIndex = 0; /* Example uses only key idx 0 (sometimes called 1) */
                break;
            case WF_SECURITY_WEP_104:
                key_size = 26; /* Assume hex size */
                if (strlen((char *) (curHTTP.data + 6)) == 13) {
                    key_size = 13; /* ASCII key support */
                    ascii_key = 1;
                }
                CFGCXT.wepIndex = 0; /* Example uses only key idx 0 (sometimes called 1) */
                break;
            }

            if (strlen((char *) (curHTTP.data + 6)) != key_size) {
                //putrsUART((ROM char *)"\r\nIncomplete key received! ");
                goto ConfigFailure;
            }

            memcpy(CFGCXT.securityKey, (void *) (curHTTP.data + 6), key_size);
            CFGCXT.securityKey[key_size] = 0; /* terminate string */
            if (!ascii_key) {
                //if ((cfg.security == sec_wep64) || (cfg.security == sec_wep128))
                key_size /= 2;
                if (!convertAsciiToHexInPlace((int8_t *) & CFGCXT.securityKey[0], key_size)) {
                    //putrsUART((ROM char *)"\r\nFailed to convert ASCII to hex! ");
                    goto ConfigFailure;
                }
            }
        } else if (!strcmppgm2ram((char *) curHTTP.data, "ssid")) { // Get new ssid and make sure it is valid
            if (strlen((char *) (curHTTP.data + 6)) < 33u) {
                memcpy(CFGCXT.ssid, (void *) (curHTTP.data + 6), strlen((char *) (curHTTP.data + 6)));
                CFGCXT.ssid[strlen((char *) (curHTTP.data + 6))] = 0; /* Terminate string */

                /* save current profile SSID for displaying later */
                WF_CPGetSsid(1, (uint8_t *) & CFGCXT.prevSSID, &ssidLen);
                CFGCXT.prevSSID[ssidLen] = 0;
            } else { // Invalid SSID... fail :-(
                goto ConfigFailure;
            }
        } else if (!strcmppgm2ram((char *) curHTTP.data, (ROM char *) "wlan")) { // Get the wlan mode: adhoc or infrastructure
            char mode[6];

            if (strlen((char *) (curHTTP.data + 6)) > 5) /* Sanity check */
                goto ConfigFailure;

            memcpy(mode, (void *) (curHTTP.data + 6), strlen((char *) (curHTTP.data + 6)));
            mode[strlen((char *) (curHTTP.data + 6))] = 0; /* Terminate string */
            if (!strcmppgm2ram((char *) mode, (ROM char *) "infra")) {
                CFGCXT.networkType = WF_INFRASTRUCTURE;
            } else if (!strcmppgm2ram((char *) mode, "adhoc")) {
                //putrsUART((ROM char *)"\r\nSetting mode to adhoc! ");
                CFGCXT.networkType = WF_ADHOC;

                // always setup adhoc to attempt to connect first, then start
                WF_CPSetAdHocBehavior(1, WF_ADHOC_CONNECT_THEN_START);
            } else { // Mode type no good  :-(
                //memset(LCDText, ' ', 32);
                //putrsUART((ROM char *)"\r\nUnknown mode type on www! ");
                //strcpy((char *)LCDText, (char *)mode);
                //LCDUpdate();
                goto ConfigFailure;
            }

            // save old WLAN mode
            WF_CPGetNetworkType(1, &CFGCXT.prevNetworkType);
        }
    }

    /* Check if WPA hasn't been selected with adhoc, if it has we choke! */
    if ((CFGCXT.networkType == WF_ADHOC) &&
            ((CFGCXT.securityMode == WF_SECURITY_WPA_AUTO_WITH_PASS_PHRASE) || (CFGCXT.securityMode == WF_SECURITY_WPA_AUTO_WITH_KEY)))
        goto ConfigFailure;

    /*
     * All parsing complete!  If we have got to here all data has been validated and
     * We can handle what is necessary to start the reconfigure process of the WiFi device
     */
#if defined ( EZ_CONFIG_STORE )
    /* Copy wifi cfg data to be committed to NVM */
    strcpy((char *) AppConfig.ssid, (char *) CFGCXT.ssid);
    AppConfig.ssidLen = strlen((char *) (CFGCXT.ssid));
    /* Going to set security type */
    AppConfig.securityMode = CFGCXT.securityMode;
    /* Going to save the key, if required */
    if (CFGCXT.securityMode != WF_SECURITY_OPEN) {
        uint8_t key_size = 0;

        switch ((uint8_t) CFGCXT.securityMode) {
        case WF_SECURITY_WPA_AUTO_WITH_PASS_PHRASE: // wpa passphrase
            key_size = strlen((char *) (CFGCXT.securityKey)); // ascii so use strlen
            break;
        case WF_SECURITY_WPA_AUTO_WITH_KEY: // wpa pre-calculated key!!!
            key_size = 32;
            break;
        case WF_SECURITY_WEP_40:
            key_size = 5;
            break;
        case WF_SECURITY_WEP_104:
            key_size = 13;
            break;
        }

        memcpy(AppConfig.securityKey, CFGCXT.securityKey, key_size);
        AppConfig.securityKeyLength = key_size;
        AppConfig.securityKey[strlen((char *) (CFGCXT.securityKey))] = 0;
    }

    /* Going to save the network type */
    AppConfig.networkType = CFGCXT.networkType;

    AppConfig.dataValid = 1; /* Validate wifi configuration */

#endif // EZ_CONFIG_STORE

    // Set the board to reboot and display reconnecting information
    strcpypgm2ram((char *) curHTTP.data, "/reconnect.htm");
    curHTTP.httpStatus = HTTP_REDIRECT;

    /*
     * Set state here to inform that the Wi-Fi device has config data and it is ready
     * to be acted upon.
     */
    //putrsUART((ROM char *)"\r\nFLaggin to start config change! ");
    WF_START_EASY_CONFIG();

    return HTTP_IO_DONE;

ConfigFailure:
    lastFailure = true;
    strcpypgm2ram((char *) curHTTP.data, "/error.htm");
    curHTTP.httpStatus = HTTP_REDIRECT;

    return HTTP_IO_DONE;
}
#endif

/*****************************************************************************
  Function:
    static HTTP_IO_RESULT HTTPPostLCD(void)

  Summary:
    Processes the LCD form on forms.htm

  Description:
    Locates the 'lcd' parameter and uses it to update the text displayed
    on the board's LCD display.

    This function has four states.  The first reads a name from the data
    string returned as part of the POST request.  If a name cannot
    be found, it returns, asking for more data.  Otherwise, if the name
    is expected, it reads the associated value and writes it to the LCD.
    If the name is not expected, the value is discarded and the next name
    parameter is read.

    In the case where the expected string is never found, this function
    will eventually return HTTP_IO_NEED_DATA when no data is left.  In that
    case, the HTTP2 server will automatically trap the error and issue an
    Internal Server Error to the browser.

  Precondition:
    None

  Parameters:
    None

  Return Values:
    HTTP_IO_DONE - the parameter has been found and saved
    HTTP_IO_WAITING - the function is pausing to continue later
    HTTP_IO_NEED_DATA - data needed by this function has not yet arrived
 ***************************************************************************/
#if defined(USE_LCD)
static HTTP_IO_RESULT HTTPPostLCD(void)
{
    uint8_t * cDest;

#define SM_POST_LCD_READ_NAME       (0u)
#define SM_POST_LCD_READ_VALUE      (1u)

    switch (curHTTP.smPost) {
    // Find the name
    case SM_POST_LCD_READ_NAME:

        // Read a name
        if (HTTPReadPostName(curHTTP.data, HTTP_MAX_DATA_LEN) == HTTP_READ_INCOMPLETE)
            return HTTP_IO_NEED_DATA;

        curHTTP.smPost = SM_POST_LCD_READ_VALUE;
        // No break...continue reading value

        // Found the value, so store the LCD and return
    case SM_POST_LCD_READ_VALUE:

        // If value is expected, read it to data buffer,
        // otherwise ignore it (by reading to NULL)
        if (!strcmppgm2ram((char *) curHTTP.data, (ROM char *) "lcd"))
            cDest = curHTTP.data;
        else
            cDest = NULL;

        // Read a value string
        if (HTTPReadPostValue(cDest, HTTP_MAX_DATA_LEN) == HTTP_READ_INCOMPLETE)
            return HTTP_IO_NEED_DATA;

        // If this was an unexpected value, look for a new name
        if (!cDest) {
            curHTTP.smPost = SM_POST_LCD_READ_NAME;
            break;
        }

        // Copy up to 32 characters to the LCD
        if (strlen((char *) cDest) < 32u) {
            memset(LCDText, ' ', 32);
            strcpy((char *) LCDText, (char *) cDest);
        } else {
            memcpy(LCDText, (void *) cDest, 32);
        }
        LCDUpdate();

        // This is the only expected value, so callback is done
        strcpypgm2ram((char *) curHTTP.data, "/forms.htm");
        curHTTP.httpStatus = HTTP_REDIRECT;
        return HTTP_IO_DONE;
    }

    // Default assumes that we're returning for state machine convenience.
    // Function will be called again later.
    return HTTP_IO_WAITING;
}
#endif

/*****************************************************************************
  Function:
    static HTTP_IO_RESULT HTTPPostConfig(void)

  Summary:
    Processes the configuration form on config/index.htm

  Description:
    Accepts configuration parameters from the form, saves them to a
    temporary location in RAM, then eventually saves the data to EEPROM or
    external Flash.

    When complete, this function redirects to config/reboot.htm, which will
    display information on reconnecting to the board.

    This function creates a shadow copy of the AppConfig structure in
    RAM and then overwrites incoming data there as it arrives.  For each
    name/value pair, the name is first read to curHTTP.data[0:5].  Next, the
    value is read to newAppConfig.  Once all data has been read, the new
    AppConfig is saved back to EEPROM and the browser is redirected to
    reboot.htm.  That file includes an AJAX call to reboot.cgi, which
    performs the actual reboot of the machine.

    If an IP address cannot be parsed, too much data is POSTed, or any other
    parsing error occurs, the browser reloads config.htm and displays an error
    message at the top.

  Precondition:
    None

  Parameters:
    None

  Return Values:
    HTTP_IO_DONE - all parameters have been processed
    HTTP_IO_NEED_DATA - data needed by this function has not yet arrived
 ***************************************************************************/
#if defined(STACK_USE_HTTP_APP_RECONFIG)
static HTTP_IO_RESULT HTTPPostConfig(void)
{
    APP_CONFIG newAppConfig;
    uint8_t *ptr;
    uint8_t i;

    // Check to see if the browser is attempting to submit more data than we
    // can parse at once.  This function needs to receive all updated
    // parameters and validate them all before committing them to memory so that
    // orphaned configuration parameters do not get written (for example, if a
    // static IP address is given, but the subnet mask fails parsing, we
    // should not use the static IP address).  Everything needs to be processed
    // in a single transaction.  If this is impossible, fail and notify the user.
    // As a web developer, if you add parameters to AppConfig and run into this
    // problem, you could fix this by to splitting your update web page into two
    // separate web pages (causing two transactional writes).  Alternatively,
    // you could fix it by storing a static shadow copy of AppConfig someplace
    // in memory and using it instead of newAppConfig.  Lastly, you could
    // increase the TCP RX FIFO size for the HTTP server.  This will allow more
    // data to be POSTed by the web browser before hitting this limit.
    if (curHTTP.byteCount > TCPIsGetReady(sktHTTP) + TCPGetRxFIFOFree(sktHTTP))
        goto ConfigFailure;

    // Ensure that all data is waiting to be parsed.  If not, keep waiting for
    // all of it to arrive.
    if (TCPIsGetReady(sktHTTP) < curHTTP.byteCount)
        return HTTP_IO_NEED_DATA;

    // Use current config in non-volatile memory as defaults
#if defined(EEPROM_CS_TRIS)
    XEEReadArray(sizeof (NVM_VALIDATION_STRUCT), (uint8_t *) & newAppConfig, sizeof (newAppConfig));
#elif defined(SPIFLASH_CS_TRIS)
    SPIFlashReadArray(sizeof (NVM_VALIDATION_STRUCT), (uint8_t *) & newAppConfig, sizeof (newAppConfig));
#endif

    // Start out assuming that DHCP is disabled.  This is necessary since the
    // browser doesn't submit this field if it is unchecked (meaning zero).
    // However, if it is checked, this will be overridden since it will be
    // submitted.
    newAppConfig.Flags.bIsDHCPEnabled = 0;

    // Read all browser POST data
    while (curHTTP.byteCount) {
        // Read a form field name
        if (HTTPReadPostName(curHTTP.data, 6) != HTTP_READ_OK)
            goto ConfigFailure;

        // Read a form field value
        if (HTTPReadPostValue(curHTTP.data + 6, sizeof (curHTTP.data) - 6 - 2) != HTTP_READ_OK)
            goto ConfigFailure;

        // Parse the value that was read
        if (!strcmppgm2ram((char *) curHTTP.data, (ROM char *) "ip")) {// Read new static IP Address
            if (!StringToIPAddress(curHTTP.data + 6, &newAppConfig.MyIPAddr))
                goto ConfigFailure;

            newAppConfig.DefaultIPAddr.Val = newAppConfig.MyIPAddr.Val;
        } else if (!strcmppgm2ram((char *) curHTTP.data, (ROM char *) "gw")) {// Read new gateway address
            if (!StringToIPAddress(curHTTP.data + 6, &newAppConfig.MyGateway))
                goto ConfigFailure;
        } else if (!strcmppgm2ram((char *) curHTTP.data, (ROM char *) "sub")) {// Read new static subnet
            if (!StringToIPAddress(curHTTP.data + 6, &newAppConfig.MyMask))
                goto ConfigFailure;

            newAppConfig.DefaultMask.Val = newAppConfig.MyMask.Val;
        } else if (!strcmppgm2ram((char *) curHTTP.data, (ROM char *) "dns1")) {// Read new primary DNS server
            if (!StringToIPAddress(curHTTP.data + 6, &newAppConfig.PrimaryDNSServer))
                goto ConfigFailure;
        } else if (!strcmppgm2ram((char *) curHTTP.data, (ROM char *) "dns2")) {// Read new secondary DNS server
            if (!StringToIPAddress(curHTTP.data + 6, &newAppConfig.SecondaryDNSServer))
                goto ConfigFailure;
        } else if (!strcmppgm2ram((char *) curHTTP.data, (ROM char *) "mac")) {
            // Read new MAC address
            uint16_t w;
            uint8_t i;

            ptr = curHTTP.data + 6;

            for (i = 0; i < 12u; i++) {// Read the MAC address

                // Skip non-hex bytes
                while (*ptr != 0x00u && !(*ptr >= '0' && *ptr <= '9') && !(*ptr >= 'A' && *ptr <= 'F') && !(*ptr >= 'a' && *ptr <= 'f'))
                    ptr++;

                // MAC string is over, so zeroize the rest
                if (*ptr == 0x00u) {
                    for (; i < 12u; i++)
                        curHTTP.data[i] = '0';
                    break;
                }

                // Save the MAC byte
                curHTTP.data[i] = *ptr++;
            }

            // Read MAC Address, one byte at a time
            for (i = 0; i < 6u; i++) {
                ((uint8_t *) & w)[1] = curHTTP.data[i * 2];
                ((uint8_t *) & w)[0] = curHTTP.data[i * 2 + 1];
                newAppConfig.MyMACAddr.v[i] = hexatob(*((TCPIP_UINT16_VAL*) & w));
            }
        } else if (!strcmppgm2ram((char *) curHTTP.data, (ROM char *) "host")) {// Read new hostname
            FormatNetBIOSName(&curHTTP.data[6]);
            memcpy((void *) newAppConfig.NetBIOSName, (void *) curHTTP.data + 6, 16);
        } else if (!strcmppgm2ram((char *) curHTTP.data, (ROM char *) "dhcp")) {// Read new DHCP Enabled flag
            if (curHTTP.data[6] == '1')
                newAppConfig.Flags.bIsDHCPEnabled = 1;
        }
    }

    // All parsing complete!  Save new settings and force a reboot
    SaveAppConfig(&newAppConfig);

    // Set the board to reboot and display reconnecting information
    strcpypgm2ram((char *) curHTTP.data, "/protect/reboot.htm?");
    memcpy((void *) (curHTTP.data + 20), (void *) newAppConfig.NetBIOSName, 16);
    curHTTP.data[20 + 16] = 0x00; // Force null termination
    for (i = 20; i < 20u + 16u; i++) {
        if (curHTTP.data[i] == ' ')
            curHTTP.data[i] = 0x00;
    }
    curHTTP.httpStatus = HTTP_REDIRECT;

    return HTTP_IO_DONE;

ConfigFailure:
    lastFailure = true;
    strcpypgm2ram((char *) curHTTP.data, "/protect/config.htm");
    curHTTP.httpStatus = HTTP_REDIRECT;

    return HTTP_IO_DONE;
}

#endif /* defined(STACK_USE_HTTP_APP_RECONFIG) */

/*****************************************************************************
  Function:
    static HTTP_IO_RESULT HTTPPostMD5(void)

  Summary:
    Processes the file upload form on upload.htm

  Description:
    This function demonstrates the processing of file uploads.  First, the
    function locates the file data, skipping over any headers that arrive.
    Second, it reads the file 64 bytes at a time and hashes that data.  Once
    all data has been received, the function calculates the MD5 sum and
    stores it in curHTTP.data.

    After the headers, the first line from the form will be the MIME
    separator.  Following that is more headers about the file, which we
    discard.  After another CRLFCRLF, the file data begins, and we read
    it 16 bytes at a time and add that to the MD5 calculation.  The reading
    terminates when the separator string is encountered again on its own
    line.  Notice that the actual file data is trashed in this process,
    allowing us to accept files of arbitrary size, not limited by RAM.
    Also notice that the data buffer is used as an arbitrary storage array
    for the result.  The ~uploadedmd5~ callback reads this data later to
    send back to the client.

  Precondition:
    None

  Parameters:
    None

  Return Values:
    HTTP_IO_DONE - all parameters have been processed
    HTTP_IO_WAITING - the function is pausing to continue later
    HTTP_IO_NEED_DATA - data needed by this function has not yet arrived
 ***************************************************************************/
#if defined(STACK_USE_HTTP_MD5_DEMO)
static HTTP_IO_RESULT HTTPPostMD5(void)
{
    uint16_t lenA, lenB;
    static HASH_SUM md5; // Assume only one simultaneous MD5

#define SM_MD5_READ_SEPARATOR    (0u)
#define SM_MD5_SKIP_TO_DATA        (1u)
#define SM_MD5_READ_DATA        (2u)
#define SM_MD5_POST_COMPLETE    (3u)

    // Don't care about curHTTP.data at this point, so use that for buffer
    switch (curHTTP.smPost) {
    // Just started, so try to find the separator string
    case SM_MD5_READ_SEPARATOR:
        // Reset the MD5 calculation
        MD5Initialize(&md5);

        // See if a CRLF is in the buffer
        lenA = TCPFindROMArray(sktHTTP, (ROM uint8_t *) "\r\n", 2, 0, false);
        if (lenA == 0xffff) {//if not, ask for more data
            return HTTP_IO_NEED_DATA;
        }

        // If so, figure out where the last byte of data is
        // Data ends at CRLFseparator--CRLF, so 6+len bytes
        curHTTP.byteCount -= lenA + 6;

        // Read past the CRLF
        curHTTP.byteCount -= TCPGetArray(sktHTTP, NULL, lenA + 2);

        // Save the next state (skip to CRLFCRLF)
        curHTTP.smPost = SM_MD5_SKIP_TO_DATA;

        // No break...continue reading the headers if possible

    // Skip the headers
    case SM_MD5_SKIP_TO_DATA:
        // Look for the CRLFCRLF
        lenA = TCPFindROMArray(sktHTTP, (ROM uint8_t *) "\r\n\r\n", 4, 0, false);

        if (lenA != 0xffff) {// Found it, so remove all data up to and including
            lenA = TCPGetArray(sktHTTP, NULL, lenA + 4);
            curHTTP.byteCount -= lenA;
            curHTTP.smPost = SM_MD5_READ_DATA;
        } else {// Otherwise, remove as much as possible
            lenA = TCPGetArray(sktHTTP, NULL, TCPIsGetReady(sktHTTP) - 4);
            curHTTP.byteCount -= lenA;

            // Return the need more data flag
            return HTTP_IO_NEED_DATA;
        }

        // No break if we found the header terminator

    // Read and hash file data
    case SM_MD5_READ_DATA:
        // Find out how many bytes are available to be read
        lenA = TCPIsGetReady(sktHTTP);
        if (lenA > curHTTP.byteCount)
            lenA = curHTTP.byteCount;

        while (lenA > 0u) {// Add up to 64 bytes at a time to the sum
            lenB = TCPGetArray(sktHTTP, curHTTP.data, (lenA < 64u) ? lenA : 64);
            curHTTP.byteCount -= lenB;
            lenA -= lenB;
            MD5AddData(&md5, curHTTP.data, lenB);
        }

        // If we've read all the data
        if (curHTTP.byteCount == 0u) {// Calculate and copy result to curHTTP.data for printout
            curHTTP.smPost = SM_MD5_POST_COMPLETE;
            MD5Calculate(&md5, curHTTP.data);
            return HTTP_IO_DONE;
        }

        // Ask for more data
        return HTTP_IO_NEED_DATA;
    }

    return HTTP_IO_DONE;
}
#endif /* defined(STACK_USE_HTTP_MD5_DEMO) */

/*****************************************************************************
  Function:
    static HTTP_IO_RESULT HTTPPostEmail(void)

  Summary:
    Processes the e-mail form on email/index.htm

  Description:
    This function sends an e-mail message using the SMTP client and
    optionally encrypts the connection to the SMTP server using SSL.  It
    demonstrates the use of the SMTP client, waiting for asynchronous
    processes in an HTTP callback, and how to send e-mail attachments using
    the stack.

    Messages with attachments are sent using multipart/mixed MIME encoding,
    which has three sections.  The first has no headers, and is only to be
    displayed by old clients that cannot interpret the MIME format.  (The
    overwhelming majority of these clients have been obsoleted, but the
    so-called "ignored" section is still used.)  The second has a few
    headers to indicate that it is the main body of the message in plain-
    text encoding.  The third section has headers indicating an attached
    file, along with its name and type.  All sections are separated by a
    boundary string, which cannot appear anywhere else in the message.

  Precondition:
    None

  Parameters:
    None

  Return Values:
    HTTP_IO_DONE - the message has been sent
    HTTP_IO_WAITING - the function is waiting for the SMTP process to complete
    HTTP_IO_NEED_DATA - data needed by this function has not yet arrived
 ***************************************************************************/
#if defined(STACK_USE_SMTP_CLIENT)
static HTTP_IO_RESULT HTTPPostEmail(void)
{
    static uint8_t *ptrData;
    static uint8_t *szPort;
#if defined(STACK_USE_SSL_CLIENT)
    static uint8_t *szUseSSL;
#endif
    uint16_t len, rem;
    uint8_t cName[8];

#define SM_EMAIL_CLAIM_MODULE               (0u)
#define SM_EMAIL_READ_PARAM_NAME            (1u)
#define SM_EMAIL_READ_PARAM_VALUE           (2u)
#define SM_EMAIL_PUT_IGNORED                (3u)
#define SM_EMAIL_PUT_BODY                   (4u)
#define SM_EMAIL_PUT_ATTACHMENT_HEADER      (5u)
#define SM_EMAIL_PUT_ATTACHMENT_DATA_BTNS   (6u)
#define SM_EMAIL_PUT_ATTACHMENT_DATA_LEDS   (7u)
#define SM_EMAIL_PUT_ATTACHMENT_DATA_POT    (8u)
#define SM_EMAIL_PUT_TERMINATOR             (9u)
#define SM_EMAIL_FINISHING                  (10u)

#define EMAIL_SPACE_REMAINING               (HTTP_MAX_DATA_LEN - (ptrData - curHTTP.data))

    switch (curHTTP.smPost) {
    case SM_EMAIL_CLAIM_MODULE:
        // Try to claim module
        if (SMTPBeginUsage()) {// Module was claimed, so set up static parameters
            SMTPClient.Subject.szROM = (ROM uint8_t *) "Microchip TCP/IP Stack Status Update";
            SMTPClient.ROMPointers.Subject = 1;
            SMTPClient.From.szROM = (ROM uint8_t *) "\"SMTP Service\" <mchpboard@picsaregood.com>";
            SMTPClient.ROMPointers.From = 1;

            // The following two lines indicate to the receiving client that
            // this message has an attachment.  The boundary field *must not*
            // be included anywhere in the content of the message.  In real
            // applications it is typically a long random string.
            SMTPClient.OtherHeaders.szROM = (ROM uint8_t *) "MIME-version: 1.0\r\nContent-type: multipart/mixed; boundary=\"frontier\"\r\n";
            SMTPClient.ROMPointers.OtherHeaders = 1;

            // Move our state machine forward
            ptrData = curHTTP.data;
            szPort = NULL;
            curHTTP.smPost = SM_EMAIL_READ_PARAM_NAME;
        }
        return HTTP_IO_WAITING;

    case SM_EMAIL_READ_PARAM_NAME:
        // Search for a parameter name in POST data
        if (HTTPReadPostName(cName, sizeof (cName)) == HTTP_READ_INCOMPLETE)
            return HTTP_IO_NEED_DATA;

        // Try to match the name value
        if (!strcmppgm2ram((char *) cName, (ROM char *) "server")) {// Read the server name
            SMTPClient.Server.szRAM = ptrData;
            curHTTP.smPost = SM_EMAIL_READ_PARAM_VALUE;
        } else if (!strcmppgm2ram((char *) cName, (ROM char *) "port")) {// Read the server port
            szPort = ptrData;
            curHTTP.smPost = SM_EMAIL_READ_PARAM_VALUE;
        }
#if defined(STACK_USE_SSL_CLIENT)
        else if (!strcmppgm2ram((char *) cName, (ROM char *) "ssl")) {// Read the server port
            szUseSSL = ptrData;
            curHTTP.smPost = SM_EMAIL_READ_PARAM_VALUE;
        }
#endif
        else if (!strcmppgm2ram((char *) cName, (ROM char *) "user")) {// Read the user name
            SMTPClient.Username.szRAM = ptrData;
            curHTTP.smPost = SM_EMAIL_READ_PARAM_VALUE;
        } else if (!strcmppgm2ram((char *) cName, (ROM char *) "pass")) {// Read the password
            SMTPClient.Password.szRAM = ptrData;
            curHTTP.smPost = SM_EMAIL_READ_PARAM_VALUE;
        } else if (!strcmppgm2ram((char *) cName, (ROM char *) "to")) {// Read the To string
            SMTPClient.To.szRAM = ptrData;
            curHTTP.smPost = SM_EMAIL_READ_PARAM_VALUE;
        } else if (!strcmppgm2ram((char *) cName, (ROM char *) "msg")) {// Done with headers, move on to the message
            // Delete paramters that are just null strings (no data from user) or illegal (ex: password without username)
            if (SMTPClient.Server.szRAM)
                if (*SMTPClient.Server.szRAM == 0x00u)
                    SMTPClient.Server.szRAM = NULL;
            if (SMTPClient.Username.szRAM)
                if (*SMTPClient.Username.szRAM == 0x00u)
                    SMTPClient.Username.szRAM = NULL;
            if (SMTPClient.Password.szRAM)
                if ((*SMTPClient.Password.szRAM == 0x00u) || (SMTPClient.Username.szRAM == NULL))
                    SMTPClient.Password.szRAM = NULL;

            // Decode server port string if it exists
            if (szPort)
                if (*szPort)
                    SMTPClient.ServerPort = (uint16_t) atol((char *) szPort);

            // Determine if SSL should be used
#if defined(STACK_USE_SSL_CLIENT)
            if (szUseSSL)
                if (*szUseSSL == '1')
                    SMTPClient.UseSSL = true;
#endif

            // Start sending the message
            SMTPSendMail();
            curHTTP.smPost = SM_EMAIL_PUT_IGNORED;
            return HTTP_IO_WAITING;
        } else {// Don't know what we're receiving
            curHTTP.smPost = SM_EMAIL_READ_PARAM_VALUE;
        }

        // No break...continue to try reading the value

    case SM_EMAIL_READ_PARAM_VALUE:
        // Search for a parameter value in POST data
        if (HTTPReadPostValue(ptrData, EMAIL_SPACE_REMAINING) == HTTP_READ_INCOMPLETE)
            return HTTP_IO_NEED_DATA;

        // Move past the data that was just read
        ptrData += strlen((char *) ptrData);
        if (ptrData < curHTTP.data + HTTP_MAX_DATA_LEN - 1)
            ptrData += 1;

        // Try reading the next parameter
        curHTTP.smPost = SM_EMAIL_READ_PARAM_NAME;
        return HTTP_IO_WAITING;

    case SM_EMAIL_PUT_IGNORED:
        // This section puts a message that is ignored by compatible clients.
        // This text will not display unless the receiving client is obsolete
        // and does not understand the MIME structure.
        // The "--frontier" indicates the start of a section, then any
        // needed MIME headers follow, then two CRLF pairs, and then
        // the actual content (which will be the body text in the next state).

        // Check to see if a failure occurred
        if (!SMTPIsBusy()) {
            curHTTP.smPost = SM_EMAIL_FINISHING;
            return HTTP_IO_WAITING;
        }

        // See if we're ready to write data
        if (SMTPIsPutReady() < 90u)
            return HTTP_IO_WAITING;

        // Write the ignored text
        SMTPPutROMString((ROM uint8_t *) "This is a multi-part message in MIME format.\r\n");
        SMTPPutROMString((ROM uint8_t *) "--frontier\r\nContent-type: text/plain\r\n\r\n");
        SMTPFlush();

        // Move to the next state
        curHTTP.smPost = SM_EMAIL_PUT_BODY;

    case SM_EMAIL_PUT_BODY:
        // Write as much body text as is available from the TCP buffer
        // return HTTP_IO_NEED_DATA or HTTP_IO_WAITING
        // On completion, => PUT_ATTACHMENT_HEADER and continue

        // Check to see if a failure occurred
        if (!SMTPIsBusy()) {
            curHTTP.smPost = SM_EMAIL_FINISHING;
            return HTTP_IO_WAITING;
        }

        // Loop as long as data remains to be read
        while (curHTTP.byteCount) {
            // See if space is available to write
            len = SMTPIsPutReady();
            if (len == 0u)
                return HTTP_IO_WAITING;

            // See if data is ready to be read
            rem = TCPIsGetReady(sktHTTP);
            if (rem == 0u)
                return HTTP_IO_NEED_DATA;

            // Only write as much as we can handle
            if (len > rem)
                len = rem;
            if (len > HTTP_MAX_DATA_LEN - 2)
                len = HTTP_MAX_DATA_LEN - 2;

            // Read the data from HTTP POST buffer and send it to SMTP
            curHTTP.byteCount -= TCPGetArray(sktHTTP, curHTTP.data, len);
            curHTTP.data[len] = '\0';
            HTTPURLDecode(curHTTP.data);
            SMTPPutString(curHTTP.data);
            SMTPFlush();
        }

        // We're done with the POST data, so continue
        curHTTP.smPost = SM_EMAIL_PUT_ATTACHMENT_HEADER;

    case SM_EMAIL_PUT_ATTACHMENT_HEADER:
        // This section writes the attachment to the message.
        // This portion generally will not display in the reader, but
        // will be downloadable to the local machine.  Use caution
        // when selecting the content-type and file name, as certain
        // types and extensions are blocked by virus filters.

        // The same structure as the message body is used.
        // Any attachment must not include high-bit ASCII characters or
        // binary data.  If binary data is to be sent, the data should
        // be encoded using Base64 and a MIME header should be added:
        // Content-transfer-encoding: base64

        // Check to see if a failure occurred
        if (!SMTPIsBusy()) {
            curHTTP.smPost = SM_EMAIL_FINISHING;
            return HTTP_IO_WAITING;
        }

        // See if we're ready to write data
        if (SMTPIsPutReady() < 100u)
            return HTTP_IO_WAITING;

        // Write the attachment header
        SMTPPutROMString((ROM uint8_t *) "\r\n--frontier\r\nContent-type: text/csv\r\nContent-Disposition: attachment; filename=\"status.csv\"\r\n\r\n");
        SMTPFlush();

        // Move to the next state
        curHTTP.smPost = SM_EMAIL_PUT_ATTACHMENT_DATA_BTNS;

    case SM_EMAIL_PUT_ATTACHMENT_DATA_BTNS:
        // The following states output the system status as a CSV file.

        // Check to see if a failure occurred
        if (!SMTPIsBusy()) {
            curHTTP.smPost = SM_EMAIL_FINISHING;
            return HTTP_IO_WAITING;
        }

        // See if we're ready to write data
        if (SMTPIsPutReady() < 36u)
            return HTTP_IO_WAITING;

        // Write the header and button strings
        SMTPPutROMString((ROM uint8_t *) "SYSTEM STATUS\r\n");
        SMTPPutROMString((ROM uint8_t *) "Buttons:,");
        SMTPPut(BUTTON0_IO + '0');
        SMTPPut(',');
        SMTPPut(BUTTON1_IO + '0');
        SMTPPut(',');
        SMTPPut(BUTTON2_IO + '0');
        SMTPPut(',');
        SMTPPut(BUTTON3_IO + '0');
        SMTPPut('\r');
        SMTPPut('\n');
        SMTPFlush();

        // Move to the next state
        curHTTP.smPost = SM_EMAIL_PUT_ATTACHMENT_DATA_LEDS;

    case SM_EMAIL_PUT_ATTACHMENT_DATA_LEDS:
        // Check to see if a failure occurred
        if (!SMTPIsBusy()) {
            curHTTP.smPost = SM_EMAIL_FINISHING;
            return HTTP_IO_WAITING;
        }

        // See if we're ready to write data
        if (SMTPIsPutReady() < 30u)
            return HTTP_IO_WAITING;

        // Write the header and button strings
        SMTPPutROMString((ROM uint8_t *) "LEDs:,");
        SMTPPut(LED0_IO + '0');
        SMTPPut(',');
        SMTPPut(LED1_IO + '0');
        SMTPPut(',');
        SMTPPut(LED2_IO + '0');
        SMTPPut(',');
        SMTPPut(LED3_IO + '0');
        SMTPPut(',');
        SMTPPut(LED4_IO + '0');
        SMTPPut(',');
        SMTPPut(LED5_IO + '0');
        SMTPPut(',');
        SMTPPut(LED6_IO + '0');
        SMTPPut(',');
        SMTPPut(LED7_IO + '0');
        SMTPPut('\r');
        SMTPPut('\n');
        SMTPFlush();

        // Move to the next state
        curHTTP.smPost = SM_EMAIL_PUT_ATTACHMENT_DATA_POT;

    case SM_EMAIL_PUT_ATTACHMENT_DATA_POT:
        // Check to see if a failure occurred
        if (!SMTPIsBusy()) {
            curHTTP.smPost = SM_EMAIL_FINISHING;
            return HTTP_IO_WAITING;
        }

        // See if we're ready to write data
        if (SMTPIsPutReady() < 16u)
            return HTTP_IO_WAITING;

        // Do the A/D conversion
#if defined(__XC8)
        // Wait until A/D conversion is done
        ADCON0bits.GO = 1;
        while (ADCON0bits.GO);
        // Convert 10-bit value into ASCII string
        len = (uint16_t) ADRES;
        uitoa(len, (uint8_t *) & curHTTP.data[1]);
#else
        len = (uint16_t) ADC1BUF0;
        uitoa(len, (uint8_t *) & curHTTP.data[1]);
#endif

        // Write the header and button strings
        SMTPPutROMString((ROM uint8_t *) "Pot:,");
        SMTPPutString(&curHTTP.data[1]);
        SMTPPut('\r');
        SMTPPut('\n');
        SMTPFlush();

        // Move to the next state
        curHTTP.smPost = SM_EMAIL_PUT_TERMINATOR;

    case SM_EMAIL_PUT_TERMINATOR:
        // This section finishes the message
        // This consists of two dashes, the boundary, and two more dashes
        // on a single line, followed by a CRLF pair to terminate the message.

        // Check to see if a failure occured
        if (!SMTPIsBusy()) {
            curHTTP.smPost = SM_EMAIL_FINISHING;
            return HTTP_IO_WAITING;
        }

        // See if we're ready to write data
        if (SMTPIsPutReady() < 16u)
            return HTTP_IO_WAITING;

        // Write the ignored text
        SMTPPutROMString((ROM uint8_t *) "--frontier--\r\n");
        SMTPPutDone();
        SMTPFlush();

        // Move to the next state
        curHTTP.smPost = SM_EMAIL_FINISHING;

    case SM_EMAIL_FINISHING:
        // Wait for status
        if (!SMTPIsBusy()) {
            // Release the module and check success
            // Redirect the user based on the result
            if (SMTPEndUsage() == SMTP_SUCCESS)
                lastSuccess = true;
            else
                lastFailure = true;

            // Redirect to the page
            strcpypgm2ram((char *) curHTTP.data, "/email/index.htm");
            curHTTP.httpStatus = HTTP_REDIRECT;
            return HTTP_IO_DONE;
        }

        return HTTP_IO_WAITING;
    }

    return HTTP_IO_DONE;
}
#endif /* defined(STACK_USE_SMTP_CLIENT) */

/****************************************************************************
  Function:
    HTTP_IO_RESULT HTTPPostDDNSConfig(void)

  Summary:
    Parsing and collecting http data received from http form.

  Description:
    This routine will be executed every time the Dynamic DNS Client
    configuration form is submitted.  The http data is received
    as a string of the variables separated by '&' characters in the TCP RX
    buffer.  This data is parsed to read the required configuration values,
    and those values are populated to the global array (DDNSData) reserved
    for this purpose.  As the data is read, DDNSPointers is also populated
    so that the dynamic DNS client can execute with the new parameters.

  Precondition:
     curHTTP is loaded.

  Parameters:
    None.

  Return Values:
    HTTP_IO_DONE         -  Finished with procedure
    HTTP_IO_NEED_DATA    -  More data needed to continue, call again later
    HTTP_IO_WAITING     -  Waiting for asynchronous process to complete,
                            call again later
 ***************************************************************************/
#if defined(STACK_USE_DYNAMICDNS_CLIENT)
static HTTP_IO_RESULT HTTPPostDDNSConfig(void)
{
    static uint8_t *ptrDDNS;

#define SM_DDNS_START           (0u)
#define SM_DDNS_READ_NAME       (1u)
#define SM_DDNS_READ_VALUE      (2u)
#define SM_DDNS_READ_SERVICE    (3u)
#define SM_DDNS_DONE            (4u)

#define DDNS_SPACE_REMAINING    (sizeof(DDNSData) - (ptrDDNS - DDNSData))

    switch (curHTTP.smPost) {
    // Sets defaults for the system
    case SM_DDNS_START:
        ptrDDNS = DDNSData;
        DDNSSetService(0);
        DDNSClient.Host.szROM = NULL;
        DDNSClient.Username.szROM = NULL;
        DDNSClient.Password.szROM = NULL;
        DDNSClient.ROMPointers.Host = 0;
        DDNSClient.ROMPointers.Username = 0;
        DDNSClient.ROMPointers.Password = 0;
        curHTTP.smPost++;

    // Searches out names and handles them as they arrive
    case SM_DDNS_READ_NAME:
        // If all parameters have been read, end
        if (curHTTP.byteCount == 0u) {
            curHTTP.smPost = SM_DDNS_DONE;
            break;
        }

        // Read a name
        if (HTTPReadPostName(curHTTP.data, HTTP_MAX_DATA_LEN) == HTTP_READ_INCOMPLETE)
            return HTTP_IO_NEED_DATA;

        if (!strcmppgm2ram((char *) curHTTP.data, (ROM char *) "service")) {
            // Reading the service (numeric)
            curHTTP.smPost = SM_DDNS_READ_SERVICE;
            break;
        } else if (!strcmppgm2ram((char *) curHTTP.data, (ROM char *) "user"))
            DDNSClient.Username.szRAM = ptrDDNS;
        else if (!strcmppgm2ram((char *) curHTTP.data, (ROM char *) "pass"))
            DDNSClient.Password.szRAM = ptrDDNS;
        else if (!strcmppgm2ram((char *) curHTTP.data, (ROM char *) "host"))
            DDNSClient.Host.szRAM = ptrDDNS;

        // Move to reading the value for user/pass/host
        curHTTP.smPost++;

    // Reads in values and assigns them to the DDNS RAM
    case SM_DDNS_READ_VALUE:
        // Read a name
        if (HTTPReadPostValue(ptrDDNS, DDNS_SPACE_REMAINING) == HTTP_READ_INCOMPLETE)
            return HTTP_IO_NEED_DATA;

        // Move past the data that was just read
        ptrDDNS += strlen((char *) ptrDDNS);
        if (ptrDDNS < DDNSData + sizeof (DDNSData) - 1)
            ptrDDNS += 1;

        // Return to reading names
        curHTTP.smPost = SM_DDNS_READ_NAME;
        break;

    // Reads in a service ID
    case SM_DDNS_READ_SERVICE:
        // Read the integer id
        if (HTTPReadPostValue(curHTTP.data, HTTP_MAX_DATA_LEN) == HTTP_READ_INCOMPLETE)
            return HTTP_IO_NEED_DATA;

        // Convert to a service ID
        DDNSSetService((uint8_t) atol((char *) curHTTP.data));

        // Return to reading names
        curHTTP.smPost = SM_DDNS_READ_NAME;
        break;

    // Sets up the DDNS client for an update
    case SM_DDNS_DONE:
        // Since user name and password changed, force an update immediately
        DDNSForceUpdate();

        // Redirect to prevent POST errors
        lastSuccess = true;
        strcpypgm2ram((char *) curHTTP.data, "/dyndns/index.htm");
        curHTTP.httpStatus = HTTP_REDIRECT;
        return HTTP_IO_DONE;
    }

    return HTTP_IO_WAITING; // Assume we're waiting to process more data
}
#endif /* defined(STACK_USE_DYNAMICDNS_CLIENT) */

#endif /* defined(HTTP_USE_POST) */

/****************************************************************************
  Section:
    Dynamic Variable Callback Functions
 ***************************************************************************/

/*****************************************************************************
  Function:
    void HTTPPrint_varname(void)

  Internal:
      See documentation in the TCP/IP Stack API or http2.h for details.
 ***************************************************************************/
void HTTPPrint_builddate(void)
{
    curHTTP.callbackPos = 0x01;
    if (TCPIsPutReady(sktHTTP) < strlenpgm((ROM char *) __DATE__" ""00:08:00"))
        return;

    curHTTP.callbackPos = 0x00;
    TCPPutROMString(sktHTTP, (ROM void *) __DATE__" ""00:08:00");
}

void HTTPPrint_version(void)
{
    TCPPutROMString(sktHTTP, (ROM void *) TCPIP_STACK_VERSION);
}

ROM uint8_t HTML_UP_ARROW[] = "up";
ROM uint8_t HTML_DOWN_ARROW[] = "dn";

void HTTPPrint_btn(uint16_t num)
{
    // Determine which button
    switch (num) {
    case 0:
        num = BUTTON0_IO;
        break;
    case 1:
        num = BUTTON1_IO;
        break;
    case 2:
        num = BUTTON2_IO;
        break;
    case 3:
        num = BUTTON3_IO;
        break;
    default:
        num = 0;
    }

    // Print the output
    TCPPutROMString(sktHTTP, (num ? HTML_UP_ARROW : HTML_DOWN_ARROW));
    return;
}

void HTTPPrint_led(uint16_t num)
{
    // Determine which LED
    switch (num) {
    case 0:
        num = LED0_IO;
        break;
    case 1:
        num = LED1_IO;
        break;
    case 2:
        num = LED2_IO;
        break;
    case 3:
        num = LED3_IO;
        break;
    case 4:
        num = LED4_IO;
        break;
    case 5:
        num = LED5_IO;
        break;
    case 6:
        num = LED6_IO;
        break;
    case 7:
        num = LED7_IO;
        break;

    default:
        num = 0;
    }

    // Print the output
    TCPPut(sktHTTP, (num ? '1' : '0'));
    return;
}

void HTTPPrint_ledSelected(uint16_t num, uint16_t state)
{
    // Determine which LED to check
    switch (num) {
    case 0:
        num = LED0_IO;
        break;
    case 1:
        num = LED1_IO;
        break;
    case 2:
        num = LED2_IO;
        break;
    case 3:
        num = LED3_IO;
        break;
    case 4:
        num = LED4_IO;
        break;
    case 5:
        num = LED5_IO;
        break;
    case 6:
        num = LED6_IO;
        break;
    case 7:
        num = LED7_IO;
        break;

    default:
        num = 0;
    }

    // Print output if true and ON or if false and OFF
    if ((state && num) || (!state && !num))
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "SELECTED");
    return;
}

void HTTPPrint_pot(void)
{
    uint8_t AN0String[8];
    uint16_t ADval;

#if defined(__XC8)
    // Wait until A/D conversion is done
    ADCON0bits.GO = 1;
    while (ADCON0bits.GO);

    // Convert 10-bit value into ASCII string
    ADval = (uint16_t) ADRES;
    //ADval *= (uint16_t)10;
    //ADval /= (uint16_t)102;
    uitoa(ADval, AN0String);
#else
    ADval = (uint16_t) ADC1BUF0;
    //ADval *= (uint16_t)10;
    //ADval /= (uint16_t)102;
    tcpip_helper_uitoa(ADval, (uint8_t *) AN0String);
#endif

    TCPPutString(sktHTTP, AN0String);
}

void HTTPPrint_lcdtext(void)
{
    uint16_t len;

    // Determine how many bytes we can write
    len = TCPIsPutReady(sktHTTP);

#if defined(USE_LCD)
    // If just starting, set callbackPos
    if (curHTTP.callbackPos == 0u)
        curHTTP.callbackPos = 32;

    // Write a byte at a time while we still can
    // It may take up to 12 bytes to write a character
    // (spaces and newlines are longer)
    while (len > 12u && curHTTP.callbackPos) {
        // After 16 bytes write a newline
        if (curHTTP.callbackPos == 16u)
            len -= TCPPutROMArray(sktHTTP, (ROM uint8_t *) "<br />", 6);

        if (LCDText[32 - curHTTP.callbackPos] == ' ' || LCDText[32 - curHTTP.callbackPos] == '\0')
            len -= TCPPutROMArray(sktHTTP, (ROM uint8_t *) "&nbsp;", 6);
        else
            len -= TCPPut(sktHTTP, LCDText[32 - curHTTP.callbackPos]);

        curHTTP.callbackPos--;
    }
#else
    TCPPutROMString(sktHTTP, (ROM uint8_t *) "No LCD Present");
#endif

    return;
}

void HTTPPrint_hellomsg(void)
{
    uint8_t *ptr;

    ptr = HTTPGetROMArg(curHTTP.data, (ROM uint8_t *) "name");

    // We omit checking for space because this is the only data being written
    if (ptr != NULL) {
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "Hello, ");
        TCPPutString(sktHTTP, ptr);
    }

    return;
}

void HTTPPrint_cookiename(void)
{
    uint8_t *ptr;

    ptr = HTTPGetROMArg(curHTTP.data, (ROM uint8_t *) "name");

    if (ptr)
        TCPPutString(sktHTTP, ptr);
    else
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "not set");

    return;
}

void HTTPPrint_uploadedmd5(void)
{
    uint8_t i;

    // Set a flag to indicate not finished
    curHTTP.callbackPos = 1;

    // Make sure there's enough output space
    if (TCPIsPutReady(sktHTTP) < 32u + 37u + 5u)
        return;

    // Check for flag set in HTTPPostMD5
#if defined(STACK_USE_HTTP_MD5_DEMO)
    if (curHTTP.smPost != SM_MD5_POST_COMPLETE)
#endif
    {// No file uploaded, so just return
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "<b>Upload a File</b>");
        curHTTP.callbackPos = 0;
        return;
    }

    TCPPutROMString(sktHTTP, (ROM uint8_t *) "<b>Uploaded File's MD5 was:</b><br />");

    // Write a byte of the md5 sum at a time
    for (i = 0; i < 16u; i++) {
        TCPPut(sktHTTP, btohexa_high(curHTTP.data[i]));
        TCPPut(sktHTTP, btohexa_low(curHTTP.data[i]));
        if ((i & 0x03) == 3u)
            TCPPut(sktHTTP, ' ');
    }

    curHTTP.callbackPos = 0x00;
    return;
}

extern APP_CONFIG AppConfig;

void HTTPPrintIP(IP_ADDR ip)
{
    uint8_t digits[4];
    uint8_t i;

    for (i = 0; i < 4u; i++) {
        if (i)
            TCPPut(sktHTTP, '.');
        tcpip_helper_uitoa(ip.v[i], digits);
        TCPPutString(sktHTTP, digits);
    }
}

void HTTPPrint_config_hostname(void)
{
    TCPPutString(sktHTTP, AppConfig.NetBIOSName);
    return;
}

void HTTPPrint_config_dhcpchecked(void)
{
    if (AppConfig.Flags.bIsDHCPEnabled)
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "checked");
    return;
}

void HTTPPrint_config_ip(void)
{
    HTTPPrintIP(AppConfig.MyIPAddr);
    return;
}

void HTTPPrint_config_gw(void)
{
    HTTPPrintIP(AppConfig.MyGateway);
    return;
}

void HTTPPrint_config_subnet(void)
{
    HTTPPrintIP(AppConfig.MyMask);
    return;
}

void HTTPPrint_config_dns1(void)
{
    HTTPPrintIP(AppConfig.PrimaryDNSServer);
    return;
}

void HTTPPrint_config_dns2(void)
{
    HTTPPrintIP(AppConfig.SecondaryDNSServer);
    return;
}

void HTTPPrint_config_mac(void)
{
    uint8_t i;

    if (TCPIsPutReady(sktHTTP) < 18u) { // need 17 bytes to write a MAC
        curHTTP.callbackPos = 0x01;
        return;
    }

    // Write each byte
    for (i = 0; i < 6u; i++) {
        if (i)
            TCPPut(sktHTTP, ':');
        TCPPut(sktHTTP, btohexa_high(AppConfig.MyMACAddr.v[i]));
        TCPPut(sktHTTP, btohexa_low(AppConfig.MyMACAddr.v[i]));
    }

    // Indicate that we're done
    curHTTP.callbackPos = 0x00;
    return;
}

void HTTPPrint_reboot(void)
{
    // This is not so much a print function, but causes the board to reboot
    // when the configuration is changed.  If called via an AJAX call, this
    // will gracefully reset the board and bring it back online immediately
    Reset();
}

void HTTPPrint_rebootaddr(void) // This is the expected address of the board upon rebooting
{
    TCPPutString(sktHTTP, curHTTP.data);
}

void HTTPPrint_ddns_user(void)
{
#if defined(STACK_USE_DYNAMICDNS_CLIENT)
    if (DDNSClient.ROMPointers.Username || !DDNSClient.Username.szRAM)
        return;
    if (curHTTP.callbackPos == 0x00u)
        curHTTP.callbackPos = (PTR_BASE) DDNSClient.Username.szRAM;
    curHTTP.callbackPos = (PTR_BASE) TCPPutString(sktHTTP, (uint8_t *) (PTR_BASE) curHTTP.callbackPos);
    if (*(uint8_t *) (PTR_BASE) curHTTP.callbackPos == '\0')
        curHTTP.callbackPos = 0x00;
#endif
}

void HTTPPrint_ddns_pass(void)
{
#if defined(STACK_USE_DYNAMICDNS_CLIENT)
    if (DDNSClient.ROMPointers.Password || !DDNSClient.Password.szRAM)
        return;
    if (curHTTP.callbackPos == 0x00u)
        curHTTP.callbackPos = (PTR_BASE) DDNSClient.Password.szRAM;
    curHTTP.callbackPos = (PTR_BASE) TCPPutString(sktHTTP, (uint8_t *) (PTR_BASE) curHTTP.callbackPos);
    if (*(uint8_t *) (PTR_BASE) curHTTP.callbackPos == '\0')
        curHTTP.callbackPos = 0x00;
#endif
}

void HTTPPrint_ddns_host(void)
{
#if defined(STACK_USE_DYNAMICDNS_CLIENT)
    if (DDNSClient.ROMPointers.Host || !DDNSClient.Host.szRAM)
        return;
    if (curHTTP.callbackPos == 0x00u)
        curHTTP.callbackPos = (PTR_BASE) DDNSClient.Host.szRAM;
    curHTTP.callbackPos = (PTR_BASE) TCPPutString(sktHTTP, (uint8_t *) (PTR_BASE) curHTTP.callbackPos);
    if (*(uint8_t *) (PTR_BASE) curHTTP.callbackPos == '\0')
        curHTTP.callbackPos = 0x00;
#endif
}

extern ROM char * ROM ddnsServiceHosts[];

void HTTPPrint_ddns_service(uint16_t i)
{
#if defined(STACK_USE_DYNAMICDNS_CLIENT)
    if (!DDNSClient.ROMPointers.UpdateServer || !DDNSClient.UpdateServer.szROM)
        return;
    if ((ROM char *) DDNSClient.UpdateServer.szROM == ddnsServiceHosts[i])
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "selected");
#endif
}

void HTTPPrint_ddns_status(void)
{
#if defined(STACK_USE_DYNAMICDNS_CLIENT)
    DDNS_STATUS s;
    s = DDNSGetLastStatus();
    if (s == DDNS_STATUS_GOOD || s == DDNS_STATUS_UNCHANGED || s == DDNS_STATUS_NOCHG)
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "ok");
    else if (s == DDNS_STATUS_UNKNOWN)
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "unk");
    else
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "fail");
#else
    TCPPutROMString(sktHTTP, (ROM uint8_t *) "fail");
#endif
}

void HTTPPrint_ddns_status_msg(void)
{
    if (TCPIsPutReady(sktHTTP) < 75u) {
        curHTTP.callbackPos = 0x01;
        return;
    }

#if defined(STACK_USE_DYNAMICDNS_CLIENT)
    switch (DDNSGetLastStatus()) {
    case DDNS_STATUS_GOOD:
    case DDNS_STATUS_NOCHG:
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "The last update was successful.");
        break;
    case DDNS_STATUS_UNCHANGED:
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "The IP has not changed since the last update.");
        break;
    case DDNS_STATUS_UPDATE_ERROR:
    case DDNS_STATUS_CHECKIP_ERROR:
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "Could not communicate with DDNS server.");
        break;
    case DDNS_STATUS_INVALID:
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "The current configuration is not valid.");
        break;
    case DDNS_STATUS_UNKNOWN:
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "The Dynamic DNS client is pending an update.");
        break;
    default:
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "An error occurred during the update.<br />The DDNS Client is suspended.");
        break;
    }
#else
    TCPPutROMString(sktHTTP, (ROM uint8_t *) "The Dynamic DNS Client is not enabled.");
#endif

    curHTTP.callbackPos = 0x00;
}

void HTTPPrint_smtps_en(void)
{
#if defined(STACK_USE_SSL_CLIENT)
    TCPPutROMString(sktHTTP, (ROM uint8_t *) "inline");
#else
    TCPPutROMString(sktHTTP, (ROM uint8_t *) "none");
#endif
}

void HTTPPrint_status_ok(void)
{
    if (lastSuccess)
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "block");
    else
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "none");
    lastSuccess = false;
}

void HTTPPrint_status_fail(void)
{
    if (lastFailure)
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "block");
    else
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "none");
    lastFailure = false;
}

void HTTPPrint_scan(void)
{
    uint8_t scanInProgressString[4];

    tcpip_helper_uitoa(IS_SCAN_IN_PROGRESS(SCANCXT.scanState), scanInProgressString);
    TCPPutString(sktHTTP, scanInProgressString);
}

void HTTPPrint_fwver(void)
{
    static bool firstTime = true;
    static tWFDeviceInfo deviceInfo;
    uint8_t fwVerString[8];

    if (firstTime) {
        firstTime = false;
        WF_GetDeviceInfo(&deviceInfo); // only call this once, not continually
    }

    tcpip_helper_uitoa((deviceInfo.romVersion << 8) | deviceInfo.patchVersion, fwVerString);
    TCPPutString(sktHTTP, fwVerString);
}

void HTTPPrint_ssid(void)
{
    static bool firstTime = true;
    static uint8_t ssidString[33];
    static uint8_t ssidLength;

    // we don't need to check the connection state as the only way this function
    // is called is from the webserver.  if the webserver is requesting this,
    // then you can infer that we should be connected to the network
    if (firstTime) {
        firstTime = false;
        WF_CPGetSsid(1, ssidString, &ssidLength);
    }

    TCPPutArray(sktHTTP, ssidString, ssidLength);
}

void HTTPPrint_bssCount(void)
{
    uint8_t bssCountString[4];

    tcpip_helper_uitoa(SCANCXT.numScanResults, bssCountString);
    TCPPutString(sktHTTP, bssCountString);
}

void HTTPPrint_valid(void)
{
    uint8_t bssDescIsValidString[4];

    tcpip_helper_uitoa((uint8_t) bssDescIsValid, bssDescIsValidString);
    TCPPutString(sktHTTP, bssDescIsValidString);
}

void HTTPPrint_name(void)
{
    if (bssDescIsValid) {
        //TCPPutString(sktHTTP, bssDesc.ssid);
        if (strlen((const char *) bssDesc.ssid) < 32)
            TCPPutString(sktHTTP, bssDesc.ssid);
        else {
            unsigned char buf_tmp[33];
            int i;
            for (i = 0; i < 32; i++) buf_tmp[i] = bssDesc.ssid[i];
            buf_tmp[32] = 0;
            TCPPutString(sktHTTP, buf_tmp);
        }
    } else {
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "0");
    }
}

void HTTPPrint_privacy(void)
{
    uint8_t security = (bssDesc.apConfig & 0xd0) >> 4;
    uint8_t secString[4];

    if (bssDescIsValid) {
        tcpip_helper_uitoa(security, secString);
        TCPPutString(sktHTTP, secString);
    } else {
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "0");
    }
}

void HTTPPrint_wlan(void)
{
    uint8_t bssTypeString[4];
    if (bssDescIsValid) {
        tcpip_helper_uitoa(bssDesc.bssType, bssTypeString);
        TCPPutString(sktHTTP, bssTypeString);
    } else {
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "0");
    }
}

void HTTPPrint_strength(void)
{
    uint8_t strVal;
    uint8_t strString[4];

    if (bssDescIsValid) {
        // MRF24WB : RSSI_MAX (200) , RSSI_MIN (106).
        // MRF24WG : RSSI_MAX (128) , RSSI_MIN (43).
#ifdef MRF24WG
        if (bssDesc.rssi < 61) {
            strVal = 1;
        } else if (bssDesc.rssi < 81) {
            strVal = 2;
        } else if (bssDesc.rssi < 101) {
            strVal = 3;
        } else {
            strVal = 4;
        }
#else
        if (bssDesc.rssi < 121) {
            strVal = 1;
        } else if (bssDesc.rssi < 141) {
            strVal = 2;
        } else if (bssDesc.rssi < 161) {
            strVal = 3;
        } else {
            strVal = 4;
        }
#endif /* MRF24WG */

        tcpip_helper_uitoa(strVal, strString);
        TCPPutString(sktHTTP, strString);
    } else {
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "0");
    }
}

void HTTPPrint_nextSSID(void)
{
    TCPPutString(sktHTTP, (uint8_t *) CFGCXT.ssid);
}

void HTTPPrint_prevSSID(void)
{
    TCPPutString(sktHTTP, (uint8_t *) CFGCXT.prevSSID);
}

void HTTPPrint_prevWLAN(void)
{
    if (CFGCXT.prevNetworkType == WF_INFRASTRUCTURE) {
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "infrastructure (BSS)");
    } else if (CFGCXT.prevNetworkType == WF_SOFT_AP) {
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "softAP (BSS)");
    } else {
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "adhoc (IBSS)");
    }
}

void HTTPPrint_nextWLAN(void)
{
    if (CFGCXT.networkType == WF_INFRASTRUCTURE) {
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "infrastructure (BSS)");
    } else if (CFGCXT.networkType == WF_ADHOC) {
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "adhoc (IBSS)");
    } else if (CFGCXT.networkType == WF_SOFT_AP) {
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "softAP (BSS)");
    } else {
        TCPPutROMString(sktHTTP, (ROM uint8_t *) "unknown");
    }
}

#endif
