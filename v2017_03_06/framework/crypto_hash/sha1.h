/******************************************************************************
 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the "Company") for its PICmicro(r) Microcontroller is intended and
 supplied to you, the Company's customer, for use solely and
 exclusively on Microchip PICmicro Microcontroller products. The
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
********************************************************************/

#if !defined (_SHA1_H)
#define _SHA1_H

#include <stdint.h>

/****************************************************************************
  Section:
    Data Types
  ***************************************************************************/

// Context storage for a hash operation
typedef struct
{
    uint32_t h0;                                            // Hash state h0
    uint32_t h1;                                            // Hash state h1
    uint32_t h2;                                            // Hash state h2
    uint32_t h3;                                            // Hash state h3
    uint32_t h4;                                            // Hash state h4
    uint32_t bytesSoFar;                                    // Total number of bytes hashed so far
    uint32_t * workingBuffer;                               // Pointer to a working buffer for hash calculation
    uint8_t partialBlock[64] __attribute__((aligned(4)));   // Beginning of next 64 byte block
} SHA1_CONTEXT;

/****************************************************************************
  Section:
    Function Prototypes
  ***************************************************************************/
  
// *****************************************************************************
/*
  Function:
    void SHA1_Initialize(SHA1_CONTEXT* context, uint8_t * workingBuffer);

  Summary:
    Initializes a SHA-1 context to perform a SHA-1 hash.

  Description:
    This routine initializes a hash context for the SHA-1 hash.
    
  Precondition:
    None.

  Parameters:
    context - The context to initialize.
    workingBuffer - A working buffer used by the module to calculate the hash.  If 
        the CRYPTO_HASH_CONFIG_SHA_SMALL_RAM macro is defined in sha_config.h, this 
        buffer must contain 16 uint32_t words.  Otherwise, this buffer must contain 
        80 32-bit words, but performance will be slightly improved.

  Returns:
    None.

  Example:
    <code>
    // Initialization for CRYPTO_HASH_CONFIG_SHA_SMALL_RAM
    uint32_t buffer[16];
    SHA1_CONTEXT context;
    SHA1_Initialize (&context, buffer);
    </code>

  Remarks:
    You must initialize a context before calculating a SHA-1 hash.
*/
void SHA1_Initialize (SHA1_CONTEXT* context, uint32_t * workingBuffer);

// *****************************************************************************
/*
  Function:
    void SHA1_DataAdd (SHA1_CONTEXT* context, uint8_t * data, uint16_t len);

  Summary:
    Adds data to a hash being calculated.

  Description:
    This routine adds data to a SHA-1 hash being calculated.  When the data 
    length reaches a block size (64 bytes), this function will calculate the hash 
    over that block and store the current hash value in the hash context.
    
  Precondition:
    The hash context must be initialized with SHA1_Initialize.

  Parameters:
    context - The context of the hash being calculated.
    data - The data being added.
    len - The length of the data being added.

  Returns:
    None.

  Example:
    <code>
    // Initialization for CRYPTO_HASH_CONFIG_SHA_SMALL_RAM
    uint8_t data[] = "Hello.";
    uint32_t buffer[16];
    SHA1_CONTEXT context;

    SHA1_Initialize (&context, buffer);

    SHA1_DataAdd (&context, data, 6);
    </code>

  Remarks:
    None.
*/
void SHA1_DataAdd (SHA1_CONTEXT* context, uint8_t * data, uint16_t len);

// *****************************************************************************
/*
  Function:
    void SHA1_Calculate(SHA1_CONTEXT* context, uint8_t * result);

  Summary:
    Finishes calculating a hash.

  Description:
    This routine finishes calculating a SHA-1 hash.  It will automatically add 
    the padding required by the hashing algorithm and return the hash digest.
    
  Precondition:
    The hash context must be initialized with SHA1_Initialize.

  Parameters:
    context - The context of the hash being calculated.
    result - A 20-byte buffer to store the calculated hash digest.

  Returns:
    None.

  Example:
    <code>
    // Initialization for CRYPTO_HASH_CONFIG_SHA_SMALL_RAM
    uint8_t data[] = "Hello.";
    uint32_t buffer[16];
    SHA1_CONTEXT context;
    uint8_t digest[20];

    SHA1_Initialize (&context, buffer);

    SHA1_DataAdd (&context, data, 6);
    SHA1_Calculate (&context, digest);
    </code>

  Remarks:
    None.
*/
void SHA1_Calculate(SHA1_CONTEXT* context, uint8_t * result);


#if defined(__18CXX)
// *****************************************************************************
/*
  Function:
    void SHA1_ROMDataAdd(SHA1_CONTEXT* context, ROM uint8_t * data, uint16_t len);
    
  Summary:
    Adds rom data to a hash being calculated.

  Description:
    This routine adds data to a SHA-1 hash being calculated.  When the data 
    length reaches a block size (64 bytes), this function will calculate the hash 
    over that block and store the current hash value in the hash context.
    
    This routine is used when adding ROM data on 8-bit microcontrollers, as they 
    require a different pointer type to address ROM variables.
    
  Precondition:
    The hash context must be initialized with MD5_Initialize.

  Parameters:
    context - The context of the hash being calculated.
    data - The data being added.
    len - The length of the data being added.

  Returns:
    None.

  Example:
    <code>
    rom uint8_t data[] = "Hello.";
    uint32_t buffer[16];
    SHA1_CONTEXT context;

    SHA1_Initialize (&context, buffer);
    SHA1_ROMDataAdd (&context, data, 6);
    </code>

  Remarks:
    None.
*/
    void SHA1_ROMDataAdd(SHA1_CONTEXT* context, ROM uint8_t * data, uint16_t len);
#else
    // Non-ROM variant for C30 / C32
    #define SHA1_ROMDataAdd(a,b,c)    SHA1_DataAdd(a,(uint8_t*)b,c)
#endif

#endif      // _SHA1_H
