/*******************************************************************************
  Company:
    Microchip Technology Inc.

  File Name:
    drv_wifi_config.h

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

#ifndef __WF_CONFIG_H_
#define __WF_CONFIG_H_

#define WF_EASY_CONFIG_DEMO

/****************************************************************************
 * MRF24WB0MA/B  (supports only 1/2 Mbps)
 *      Client in infrastructure network                     (ALL security)
 *      Adhoc network                                        (OPEN, WEP security)
 *
 * MRF24WG0MA/B
 *      Client in infrastructure network                     (ALL security)
 *      Adhoc network                                        (OPEN, WEP security)
 *      Group client (GC) in WFii direct (P2P) network
 *      SoftAP                                               (OPEN, WEP security)
 *      Supports WPS security connection
 *
 * Available documentation
 * DS52108A  Microchip MRF24W Getting started Guide for MRF24WB0MAB, MRF24WG0MA/B for MLA v5
 *****************************************************************************/

/*
 *********************************************************************************************************
 *                                           DEFINES
 *********************************************************************************************************
 */

/*= WF_CONSOLE =======================================================================*/
/* Customize whether the WiFi Driver supports a serial console application            */
/*   To DISABLE the serial console application: Comment out '#define WF_CONSOLE'      */
/*   To ENABLE the serial console application: Uncomment out '#define WF_CONSOLE'     */
/*====================================================================================*/
#define WF_CONSOLE           /* needed for console demo */
#define WF_CONSOLE_IFCFGUTIL /* needed for console demo */

#define MY_DEFAULT_DOMAIN  WF_DOMAIN_FCC

#define CFG_WF_ADHOC    2
#define CFG_WF_SOFT_AP  4

#if defined (MRF24WG)
#define MY_DEFAULT_NETWORK_TYPE  CFG_WF_SOFT_AP /* CFG_WF_ADHOC, CFG_WF_SOFT_AP */
#else /* MRF24WG is not defined, which means the device is MRF24WB */
#define MY_DEFAULT_NETWORK_TYPE  CFG_WF_ADHOC /* only CFG_WF_ADHOC is supported */
#endif /* defined (MRF24WG) */

//
// Retries
//
#define ADHOC_RETRY_COUNT  (3)
#define MY_DEFAULT_LIST_RETRY_COUNT_INFRASTRUCTURE  WF_RETRY_FOREVER

/*--------------------------------------------*/
/* Default settings for Adhoc mode */
/*--------------------------------------------*/
#if MY_DEFAULT_NETWORK_TYPE == CFG_WF_ADHOC
#define MY_DEFAULT_SSID_NAME                "MicrochipDemoAdHoc"
#define MY_DEFAULT_BEACON_TIMEOUT           (40)           /* Number of beacon periods          */
#define MY_DEFAULT_SCAN_TYPE                WF_ACTIVE_SCAN /* WF_ACTIVE_SCAN or WF_PASSIVE_SCAN */
#define MY_DEFAULT_CHANNEL_LIST             {}             /* Desired channel list              */
#define MY_DEFAULT_CHANNEL_LIST_PRESCAN     {}             /* Applicable for SoftAP only        */
#define MY_DEFAULT_CHANNEL_LIST_POSTSCAN    {}             /* Applicable for SoftAP only        */
#define MY_DEFAULT_LIST_RETRY_COUNT         ADHOC_RETRY_COUNT
#define MY_DEFAULT_PS_POLL                  WF_DISABLED    /* PS is not supported in Adhoc      */
#define MY_DEFAULT_WIFI_SECURITY_MODE       WF_SECURITY_OPEN
/*-------------------------------------------*/
/* else if starting this demo in SoftAP mode */
/*-------------------------------------------*/
#elif MY_DEFAULT_NETWORK_TYPE == CFG_WF_SOFT_AP
/* SoftAP function has the full "EasyConfig" function.
 * Your STA can connect to the SoftAP as a client, get the DHCP IP, run ping, and run web browser to connect to Web Server
 * of SoftAP. It will allow you to re-connect / redirect to another AP in infrastructure mode.
 * The reason this demo sits here is simply A) borrow DHCP server; B) borrow HTTP server.
 *
 * Before starting up as SoftAP, prescan (MY_DEFAULT_CHANNEL_LIST_PRESCAN) will be executed.
 * For SoftAP, default channel is assigned as MY_DEFAULT_CHANNEL_LIST i.e single channel 6. This means SoftAP
 * will start up in channel 6.
 * When scan option is selected in EZConfig web browser in SoftAP mode, the prescan results will be displayed.
 * Repeated selection of this scan option will always display the prescan results. From this prescan list, the user can select
 * an AP to be redirected to.
 * When redirected to another AP, the channel list will change to allow more channel listings in infrastructure mode,
 * i.e all channels MY_DEFAULT_CHANNEL_LIST_POSTSCAN. This means AP will scan MY_DEFAULT_CHANNEL_LIST_POSTSCAN
 * channel list.
 *
 * SoftAP's function is limited as,
 * A) Routing is not supported
 * B) Only 1 client allowed (RF module FW version 0x3107 / 0x310B)
 *    Max 4 clients allowed (RF module FW version 0x3108 / 0x310C)
 * C) supports open mode and WEP security.
 *
 * SoftAP's default IP is 192.168.1.3 and its Network Mask is 255.255.0.0
 * SoftAP on certain setups with IP adress 192.168.1.1 has problem with DHCP client assigning new IP address on redirection.
 * 192.168.1.1 is a common IP address with most APs. For now, assign this as 192.168.1.3
 * Reason:
 *    Conflict arises when there are 2 active DHCP servers in the same network (i.e. AP DHCP server and MRF24W EasyConfig DHCP Server).
 *    When network redirection is executed, the TCPIP SW may still have the device DHCP server still active.
 *    This may require change in TCPIP SW to be able to disable the local DHCP server after network redirection.
 *
 * SoftAP has support for ZeroConf/mDNS. Seach for keyword SOFTAP_ZEROCONF_SUPPORT.
 *
 */
#if defined (MRF24WG)
#define MY_DEFAULT_WIFI_SECURITY_MODE       WF_SECURITY_OPEN  /* Open & WEP security supported.
                                                               * For WEP security, RF Module FW version 3107
                                                               * or the later needs.
                                                               */
#define MY_DEFAULT_SCAN_TYPE                WF_ACTIVE_SCAN    /* Dummy, Not used                  */
#define MY_DEFAULT_SSID_NAME                "MicrochipSoftAP" /* Set SoftAP ssid                  */
#define MY_DEFAULT_LIST_RETRY_COUNT         ADHOC_RETRY_COUNT /* Dummy, Not used                  */
#define MY_DEFAULT_CHANNEL_LIST             {6}               /* Set SoftAP network channel       */
#define MY_DEFAULT_CHANNEL_LIST_PRESCAN     {}                /* SoftAP: Pre-scan channel list WF_PRESCAN */
#define MY_DEFAULT_CHANNEL_LIST_POSTSCAN    {}                /* SoftAP: Post-scan channel list           */
#define MY_DEFAULT_BEACON_TIMEOUT           (40)              /* Dummy, Not used                  */
#define MY_DEFAULT_PS_POLL                  WF_DISABLED       /* Dummy, Not used                  */
#else /* !defined (MRF24WG) */
#error "MRF24WB does not support SoftAP"
#endif /* defined (MRF24WG) */
#endif /* MY_DEFAULT_NETWORK_TYPE == CFG_WF_ADHOC */

#define WF_CHECK_LINK_STATUS           WF_ENABLED /* Gets the MRF to check the link status relying on Tx failures. */
#define WF_LINK_LOST_THRESHOLD         40 /* Consecutive Tx transmission failures to be considered the AP is gone away. */
#if defined(MRF24WG)
#define WF_SOFTAP_SEND_KEEP_ALIVE      WF_DISABLED /* Gets SoftAP to send keep alive packets to clients. */
#define WF_SOFTAP_LINK_LOST_THRESHOLD  40 /* Consecutive null packet transmission failures to be considered the client STA is gone away. */
#endif

#define MY_DEFAULT_EVENT_NOTIFICATION_LIST  (WF_NOTIFY_CONNECTION_ATTEMPT_SUCCESSFUL | \
                                             WF_NOTIFY_CONNECTION_ATTEMPT_FAILED     | \
                                             WF_NOTIFY_CONNECTION_TEMPORARILY_LOST   | \
                                             WF_NOTIFY_CONNECTION_PERMANENTLY_LOST   | \
                                             WF_NOTIFY_CONNECTION_REESTABLISHED)

//#define USE_MRF24W_HOST_BUFFER

#define STACK_USE_EZ_CONFIG
#define EZ_CONFIG_SCAN
#define EZ_CONFIG_STALL
#define EZ_CONFIG_STORE

#if MY_DEFAULT_NETWORK_TYPE == CFG_WF_ADHOC
#define WF_PRE_SCAN_IN_ADHOC
#endif

/* 
 * MRF24W FW has a built-in connection manager, and it is enabled by default.
 * If you want to run your own connection manager in host side, you should
 * disable the FW connection manager to avoid possible conflict between the two.
 * Especially these two APIs can be affected if you do not disable it.
 * A) uint16_t WF_CMDisconnect(void)
 * B) uint16_t WF_Scan(uint8_t CpId)
 * These APIs will return failure when the conflict occurs.
 */
#define WF_MODULE_CONNECTION_MANAGER WF_ENABLED

/*****************************************************************************/
/*****************************************************************************/
/*                   WIFI SECURITY COMPILE-TIME DEFAULTS                     */
/*****************************************************************************/
/*****************************************************************************
 * Security modes available on WiFi network:
 *   WF_SECURITY_OPEN                      : No security
 *   WF_SECURITY_WEP_40                    : WEP Encryption using 40 bit keys
 *   WF_SECURITY_WEP_104                   : WEP Encryption using 104 bit keys
 ******************************************************************************/

#if defined(__XC32)
/* This is optional, which results faster key derivation */
#define WF_HOST_DERIVE_KEY_FROM_PASSPHRASE WF_ENABLED
#else
#define WF_HOST_DERIVE_KEY_FROM_PASSPHRASE WF_DISABLED
#endif /* defined(__XC32) */

//-----------------------------------------------------------------------------------
// WEP
// Default WEP keys used in WF_SECURITY_WEP_40  and WF_SECURITY_WEP_104 security mode
// Only WEP key index 0 is valid
//-----------------------------------------------------------------------------------
#define MY_DEFAULT_WEP_PHRASE "WEP Phrase"

// string 4 40-bit WEP keys -- corresponding to passphrase of "WEP Phrase"
#define MY_DEFAULT_WEP_KEYS_40 "\
\x5a\xfb\x6c\x8e\x77\
\xc1\x04\x49\xfd\x4e\
\x43\x18\x2b\x33\x88\
\xb0\x73\x69\xf4\x78"

// string containing 4 104-bit WEP keys -- corresponding to passphrase of "WEP Phrase"
#define MY_DEFAULT_WEP_KEYS_104 "\
\x90\xe9\x67\x80\xc7\x39\x40\x9d\xa5\x00\x34\xfc\xaa\
\x77\x4a\x69\x45\xa4\x3d\x66\x63\xfe\x5b\x1d\xb9\xfd\
\x82\x29\x87\x4c\x9b\xdc\x6d\xdf\x87\xd1\xcf\x17\x41\
\xcc\xd7\x62\xde\x92\xad\xba\x3b\x62\x2f\x7f\xbe\xfb"

#endif /* __WF_CONFIG_H_ */
