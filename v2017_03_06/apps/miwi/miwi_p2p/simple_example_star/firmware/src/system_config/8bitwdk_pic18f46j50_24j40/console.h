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
#ifndef  _CONSOLE_H_
#define  _CONSOLE_H_

/************************ HEADERS **********************************/
#include "system_config.h"


#define BAUD_RATE 19200


/*********************************************************************/
// ENABLE_CONSOLE will enable the print out on the hyper terminal
// this definition is very helpful in the debugging process
// Defined in miwi_config.h
/*********************************************************************/

#if defined(ENABLE_CONSOLE)

//DEFINITIONS

#define CONSOLE_IsPutReady()     (TXSTA2bits.TRMT2)
#define CONSOLE_IsGetReady()     (PIR3bits.RC2IF)


//Define UART TX and RX registers
#define TREG        TXREG2
#define RREG        RCREG2
#define TSTA        TXSTA2
#define RSTA        RCSTA2
#define UART_SPBRG  SPBRG2
#define RSTA_OERR   RCSTA2bits.OERR
#define RSTA_CREN   RCSTA2bits.CREN

#define UART_TX_PIN TRISCbits.TRISC6
#define UART_RX_PIN TRISDbits.TRISD2

#define INPUT_PIN   1
#define OUPUT_PIN   0

/************************************MCC_UART ***********************/
// get/set TX2 aliases
#define TX2_TRIS               TRISC6
#define TX2_LAT                LATC6
#define TX2_PORT               PORTCbits.RC6
#define TX2_ANS                ANSC6
#define TX2_SetHigh()    do { LATC6 = 1; } while(0)
#define TX2_SetLow()   do { LATC6 = 0; } while(0)
#define TX2_Toggle()   do { LATC6 = ~LATC6; } while(0)
#define TX2_GetValue()         PORTCbits.RC6
#define TX2_SetDigitalInput()    do { TRISC6 = 1; } while(0)
#define TX2_SetDigitalOutput()   do { TRISC6 = 0; } while(0)

#define TX2_SetAnalogMode()   do { ANSC6 = 1; } while(0)
#define TX2_SetDigitalMode()   do { ANSC6 = 0; } while(0)
// get/set RX2 aliases
#define RX2_TRIS               TRISD2
#define RX2_LAT                LATD2
#define RX2_PORT               PORTDbits.RD2
#define RX2_ANS                ANSD2
#define RX2_SetHigh()    do { LATD2 = 1; } while(0)
#define RX2_SetLow()   do { LATD2 = 0; } while(0)
#define RX2_Toggle()   do { LATD2 = ~LATD2; } while(0)
#define RX2_GetValue()         PORTDbits.RD2
#define RX2_SetDigitalInput()    do { TRISD2 = 1; } while(0)
#define RX2_SetDigitalOutput()   do { TRISD2 = 0; } while(0)

#define RX2_SetAnalogMode()   do { ANSD2 = 1; } while(0)
#define RX2_SetDigitalMode()   do { ANSD2 = 0; } while(0)
/**********************************************************************/
#define INPUT_PIN   1
#define OUPUT_PIN   0
/******************************** MCC_UART  *******************/
#define EUSART2_DataReady  (eusart2RxCount)
extern volatile uint8_t eusart2TxBufferRemaining;
extern volatile uint8_t eusart2RxCount;
void EUSART2_Initialize(void);      // MCC_UART

uint8_t EUSART2_Read(void);         // MCC_UART
void EUSART2_Write(uint8_t txData); // MCC_UART

//FUNCTION PROTOTYPES
/*********************************************************************
* Function:         void CONSOLE_Initialize(void)
*
* PreCondition:     none
*
* Input:	    none
*
* Output:	    none
*
* Side Effects:	    UART is configured
*
* Overview:         This function will configure the UART for use at
*                   in 8 bits, 1 stop, no flowcontrol mode
*
* Note:             None
********************************************************************/
 void CONSOLE_Initialize(void);


/*********************************************************************
* Function:         void CONSOLE_Put(uint8_t c)
*
* PreCondition:     none
*
* Input:            c - character to be printed
*
* Output:           none
*
* Side Effects:	    c is printed to the console
*
* Overview:	    This function will print the inputed character
*
* Note:		    Do not power down the microcontroller until
*                   the transmission is complete or the last
*                   transmission of the string can be corrupted.
********************************************************************/
 void CONSOLE_Put(uint8_t c);



/*********************************************************************
* Function:         void CONSOLE_PutString(ROM char* str)
*
* PreCondition:     none
*
* Input:            str - String that needs to be printed
*
* Output:           none
*
* Side Effects:	    str is printed to the console
*
* Overview:         This function will print the inputed ROM string
*
* Note:             Do not power down the microcontroller until
*                   the transmission is complete or the last
*                   transmission of the string can be corrupted.
********************************************************************/
void CONSOLE_PutString(char* str);

/*********************************************************************
* Function:         uint8_t CONSOLE_Get(void)
*
* PreCondition:     none
*
* Input:            none
*
* Output:           one byte received by UART
*
* Side Effects:	    none
*
* Overview:         This function will receive one byte from UART
*
* Note:             Do not power down the microcontroller until
*                   the transmission is complete or the last
*                   transmission of the string can be corrupted.
********************************************************************/
uint8_t CONSOLE_Get(void);



/*********************************************************************
* Function:         void CONSOLE_PrintHex(uint8_t toPrint)
*
* PreCondition:     none
*
* Input:            toPrint - character to be printed
*
* Output:           none
*
* Side Effects:	    toPrint is printed to the console
*
* Overview:         This function will print the inputed char to
*                   the console in hexidecimal form
*
* Note:             Do not power down the microcontroller until
*                   the transmission is complete or the last
*                   transmission of the string can be corrupted.
********************************************************************/
void CONSOLE_PrintHex(uint8_t);

/*********************************************************************
* Function:         void CONSOLE_PrintDec(uint8_t toPrint)
*
* PreCondition:     none
*
* Input:		    toPrint - character to be printed. Range is 0-99
*
* Output:		    none
*
* Side Effects:	    toPrint is printed to the console in decimal
*
*
* Overview:		    This function will print the inputed uint8_t to
*                   the console in decimal form
*
* Note:			    Do not power down the microcontroller until
*                   the transmission is complete or the last
*                   transmission of the string can be corrupted.
********************************************************************/
void CONSOLE_PrintDec(uint8_t);

//If console is not enabled
#else

#define CONSOLE_Initialize()
#define CONSOLE_IsPutReady()                 1
#define CONSOLE_IsGetReady()                 1
#define CONSOLE_Put(c)
#define CONSOLE_PutString(str)
#define CONSOLE_Get()                        'a'
#define CONSOLE_PrintHex(a)
#define CONSOLE_PrintDec(a)


#endif

#define Printf(x) CONSOLE_PutString((char*)x)

#endif


