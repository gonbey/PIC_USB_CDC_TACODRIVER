/********************************************************************
 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the "Company") for its PIC(R) Microcontroller is intended and
 supplied to you, the Company's customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN "AS IS" CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *******************************************************************/
#include <leds.h>
#include <buttons.h>

#define LED_USB_HOST_HID_KEYBOARD_DEVICE_READY     LED_NONE

#define ANDROID_DEMO_LED_0              LED_D3
#define ANDROID_DEMO_LED_1              LED_D4
#define ANDROID_DEMO_LED_2              LED_D5
#define ANDROID_DEMO_LED_3              LED_D6
#define ANDROID_DEMO_LED_4              LED_D7
#define ANDROID_DEMO_LED_5              LED_D8
#define ANDROID_DEMO_LED_6              LED_D9
#define ANDROID_DEMO_LED_7              LED_D10

#define ANDROID_DEMO_BUTTON_1           BUTTON_S3
#define ANDROID_DEMO_BUTTON_2           BUTTON_S6
#define ANDROID_DEMO_BUTTON_3           BUTTON_S5
#define ANDROID_DEMO_BUTTON_4           BUTTON_S4

