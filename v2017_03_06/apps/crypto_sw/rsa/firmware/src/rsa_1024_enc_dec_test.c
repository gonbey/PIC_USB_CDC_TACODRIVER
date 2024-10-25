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

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "crypto_sw/crypto_sw.h"
#include "crypto_sw/src/drv_common.h"

#if defined (__PIC24F__)
#pragma config POSCMOD = XT, FNOSC = PRIPLL, IESO = OFF, OSCIOFNC = OFF, FCKSM = CSDCMD, FWDTEN = OFF
#endif

/* This unused array forces DSRPAG to 0x0200 on 33E devices, so it allows
 * testing the setting of DSRPAG to access ymemory.
 */
static unsigned char const dummy[] = {
    0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd, 0xdd,
};

/* RSA public exponent */
uint8_t rsa_e[] __attribute__ ((aligned(4))) =
{
    0x01, 0x00, 0x01
};

unsigned char e_bad[] = {
    0x00, 0x00, 0x00,
};

/*********************************************************
 RSA Key Strength:  1024
 *********************************************************/
// RSA Modulus (1024 bit)
uint8_t rsa_n_1024[] __attribute__ ((aligned(4))) =
{
    0x61, 0x38, 0xf7, 0xff, 0x65, 0x0a, 0x19, 0xed,
    0xfd, 0x40, 0x2a, 0x99, 0xb2, 0xd5, 0xf7, 0x66,
    0x9f, 0x24, 0x5a, 0x15, 0x99, 0x93, 0xed, 0x4c,
    0xb7, 0x69, 0x61, 0x89, 0xad, 0x35, 0xfa, 0x0a,
    0x9e, 0x43, 0x0c, 0xdc, 0x79, 0x2a, 0x75, 0x3b,
    0x0f, 0xf8, 0x0f, 0x4c, 0xa2, 0xf4, 0xbe, 0x2d,
    0xe8, 0x13, 0x16, 0xcd, 0xc8, 0xcd, 0xf4, 0x3c,
    0xad, 0x4e, 0x7f, 0xa8, 0x0d, 0xec, 0x21, 0x0c,
    0x4a, 0xda, 0xb9, 0xe0, 0x14, 0xb0, 0x35, 0x3a,
    0x65, 0x02, 0x54, 0xb0, 0x45, 0xec, 0x81, 0x66,
    0x9c, 0x7c, 0xab, 0x72, 0x06, 0x29, 0xa0, 0x34,
    0x78, 0x8c, 0x6d, 0x8c, 0xbd, 0x29, 0x13, 0xdf,
    0xad, 0x9e, 0x0c, 0xbf, 0x02, 0xaf, 0x90, 0x2c,
    0x6d, 0xda, 0x34, 0x86, 0x30, 0xe0, 0xc4, 0x97,
    0x3a, 0x39, 0x34, 0x40, 0x5b, 0xa0, 0xa6, 0xc6,
    0x61, 0x77, 0x92, 0xcb, 0xf4, 0x67, 0x18, 0xb7,
};

// RSA Private Key Prime P (1024 bit)
uint8_t rsa_p_1024[] __attribute__ ((aligned(4))) =
{
    0x35, 0x42, 0x0e, 0x82, 0x65, 0xf9, 0xb1, 0x9f,
    0x1e, 0xcc, 0xef, 0xc7, 0x77, 0x86, 0x0a, 0x41,
    0x7d, 0xd0, 0x54, 0x44, 0x9c, 0xa4, 0x6d, 0x88,
    0x5c, 0x1f, 0xdf, 0xa6, 0x07, 0x18, 0xc0, 0xaa,
    0x62, 0x64, 0x24, 0xe7, 0x8e, 0x43, 0xe0, 0xab,
    0xf1, 0x52, 0x55, 0x88, 0x2e, 0x6d, 0x5e, 0x2a,
    0x60, 0xaa, 0x27, 0x89, 0x59, 0xed, 0xae, 0x51,
    0xa5, 0x5e, 0x9a, 0xdf, 0xe1, 0x62, 0xcb, 0xff,
};

// RSA Private Key Prime Q (1024 bit)
uint8_t rsa_q_1024[] __attribute__ ((aligned(4))) =
{
    0xfd, 0xe2, 0x59, 0x09, 0x6f, 0x68, 0xd5, 0xcd,
    0xe0, 0xa8, 0x64, 0x82, 0xfb, 0xbd, 0x52, 0x25,
    0x24, 0xe9, 0xeb, 0x15, 0xa6, 0x7c, 0x9b, 0x91,
    0xc8, 0x97, 0x8c, 0xa2, 0x47, 0x81, 0x9d, 0xb5,
    0xeb, 0xdb, 0x7c, 0x22, 0x4f, 0xa9, 0x28, 0x09,
    0x26, 0x17, 0x81, 0xe5, 0x57, 0x2f, 0x0c, 0xb9,
    0x99, 0x28, 0x55, 0xbb, 0xe9, 0x21, 0x68, 0xb4,
    0x56, 0x11, 0xe0, 0xe1, 0x06, 0x11, 0x3e, 0xb7,
};

// RSA Private Key Prime Exponent dP (1024 bit)
uint8_t rsa_dp_1024[] __attribute__ ((aligned(4))) =
{
    0xd9, 0x70, 0x70, 0x61, 0xaf, 0x66, 0x67, 0x17,
    0x36, 0x55, 0x21, 0x1d, 0xf9, 0x32, 0x71, 0x12,
    0xf3, 0x7a, 0x2a, 0x37, 0xbd, 0x5a, 0xaa, 0x01,
    0x05, 0x42, 0x3d, 0x34, 0x7e, 0x2c, 0xa2, 0x13,
    0x21, 0xeb, 0x0d, 0xcb, 0x4e, 0x9e, 0x01, 0x13,
    0x87, 0x39, 0xd9, 0x64, 0x48, 0xb5, 0x98, 0x6a,
    0xa0, 0x67, 0x42, 0x7a, 0x0d, 0xda, 0x8b, 0x27,
    0xb3, 0x36, 0xa3, 0x52, 0xbe, 0x85, 0x78, 0x66,
};

// RSA Private Key Prime Exponent dQ (1024 bit)
uint8_t rsa_dq_1024[] __attribute__ ((aligned(4))) =
{
    0x6d, 0xc7, 0x0d, 0x8e, 0xe4, 0x3c, 0xf1, 0x0e,
    0xd2, 0xef, 0x6f, 0x63, 0x34, 0x42, 0x6d, 0x9f,
    0x62, 0x54, 0x20, 0xf2, 0xe5, 0x95, 0x77, 0xfd,
    0x9d, 0x1a, 0x3a, 0xa3, 0xa5, 0xc3, 0xd4, 0x27,
    0x14, 0x39, 0x37, 0xeb, 0xdc, 0xc9, 0xc0, 0x1a,
    0xe1, 0x20, 0x22, 0xbe, 0xbb, 0x71, 0x4f, 0xe5,
    0x7c, 0xa1, 0x62, 0x99, 0x22, 0x42, 0x44, 0x09,
    0x46, 0x3b, 0x50, 0xb3, 0xd7, 0xfc, 0xce, 0x0c,
};

// RSA Private Key Coefficient qInv (1024 bit)
uint8_t rsa_qinv_1024[] __attribute__ ((aligned(4))) =
{
    0xa1, 0x00, 0x57, 0x6c, 0x43, 0x78, 0xd1, 0x17,
    0xe7, 0x6f, 0xaf, 0xe4, 0x8f, 0x80, 0xba, 0x65,
    0xa3, 0x25, 0x1a, 0xf4, 0xf5, 0xdc, 0xac, 0xc0,
    0x01, 0x74, 0x31, 0x04, 0xb3, 0xef, 0xa2, 0xa0,
    0x66, 0x5a, 0x88, 0xa6, 0xe5, 0x0f, 0x97, 0x51,
    0xf5, 0x38, 0xcf, 0xe7, 0xb4, 0xaa, 0xc5, 0x9a,
    0xc5, 0xa7, 0xaa, 0xe2, 0x2b, 0xf4, 0xe6, 0xf0,
    0x54, 0x37, 0x04, 0x18, 0x1f, 0x86, 0xe8, 0x4d,
};

/* plaintext message */

unsigned char m[]  __attribute__((aligned (4))) ={
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
    0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17,
    0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d, 0x1e, 0x1f,
    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27,
    0x28, 0x29, 0x2a, 0x2b, 0x2c, 0x2d, 0x2e, 0x2f,
    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37,
    0x38, 0x39, 0x3a, 0x3b, 0x3c, 0x3d, 0x3e, 0x3f,
    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47,
    0x48, 0x49, 0x4a, 0x4b, 0x4c, 0x4d, 0x4e, 0x4f,
    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57,
    0x58, 0x59, 0x5a, 0x5b, 0x5c, 0x5d, 0x5e, 0x5f,
    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67,
    0x68, 0x69, 0x6a, 0x6b, 0x6c, 0x6d, 0x6e, 0x6f,
    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
    0x78, 0x79, 0x7a, 0x7b, 0x7c, 0x7d, 0x7e, 0x7f,
    0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
    0x88, 0x89, 0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
    0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
    0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
    0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7,
    0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
    0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7,
    0xb8, 0xb9, 0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
    0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7,
    0xc8, 0xc9, 0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
    0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7,
    0xd8, 0xd9, 0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
    0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7,
    0xe8, 0xe9, 0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
    0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7,
    0xf8, 0xf9, 0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff,
};

/* the typical message that will be encrypted/decrypted is the first
   10 bytes of m[] */

unsigned short int m_byte_len = 10;

/* other test lengths include: */

/* 0 bytes of m[] - for the null message */
unsigned short int null_m_byte_len = 0;

/* 128-11 bytes of m[] - for the maximum RSA-1024 message size */
unsigned short int full_m_byte_len_1024 = 128-11;

/* RSA 1024 ciphertext of the null string */

static unsigned char c_null_m_1024[]   __attribute__((aligned (4))) = {
    0x54, 0x52, 0x15, 0x3B, 0xA1, 0x35, 0xFD, 0x2F, 
    0xE6, 0x52, 0xE8, 0x4D, 0x96, 0xE8, 0xFA, 0x5C, 
    0xD3, 0x50, 0xA4, 0x86, 0x5B, 0x70, 0x2F, 0xE0, 
    0x1C, 0x96, 0xF0, 0x1F, 0x3E, 0xAE, 0x26, 0xFA, 
    0xB3, 0x32, 0x30, 0xF9, 0x61, 0x86, 0xFF, 0x82, 
    0xCC, 0x44, 0xA4, 0x26, 0x0A, 0xE3, 0xE9, 0x53, 
    0x99, 0xA4, 0xFD, 0x76, 0x0A, 0x4F, 0x8C, 0xDC, 
    0xCE, 0x8C, 0x1F, 0xD0, 0x2B, 0x0E, 0x20, 0xB0, 
    0x95, 0x8E, 0x74, 0x86, 0x26, 0xCD, 0x22, 0x5C, 
    0xCF, 0x17, 0x1E, 0x93, 0xA3, 0xCA, 0xA2, 0x6E, 
    0x62, 0x56, 0x2B, 0x8F, 0xBC, 0x1F, 0xB5, 0xFC, 
    0xA3, 0x7D, 0x65, 0xE6, 0xE2, 0x96, 0xCC, 0x62, 
    0xEE, 0x93, 0x20, 0x9D, 0x8C, 0xF2, 0x6D, 0x0E, 
    0x88, 0xA2, 0x27, 0x70, 0xE3, 0xEE, 0x25, 0xCA, 
    0x6E, 0xCB, 0xB1, 0x32, 0x50, 0xDA, 0x93, 0x96, 
    0x44, 0xE4, 0xCD, 0xC1, 0x42, 0x3C, 0x5C, 0xEE, 
};

/* RSA 1024 ciphertext of first 10 bytes of m[] */

static unsigned char c_1024[]   __attribute__((aligned (4))) = {
    0x9C, 0xB3, 0x03, 0x31, 0x6B, 0xEA, 0xBB, 0x8C, 
    0xB2, 0x8A, 0xAC, 0x62, 0x08, 0xBD, 0xC3, 0x34, 
    0x0C, 0xDE, 0xB5, 0x6A, 0x91, 0xBE, 0xBA, 0x9D, 
    0x7D, 0x06, 0x33, 0x5C, 0x34, 0x03, 0x89, 0x2C, 
    0xC5, 0x9C, 0x3B, 0x39, 0x91, 0xC1, 0xBA, 0xF4, 
    0xEE, 0x34, 0xAF, 0x04, 0x79, 0x1B, 0x79, 0x3D, 
    0x81, 0xAE, 0x03, 0x0A, 0x3F, 0x6E, 0x2B, 0xAD, 
    0xB3, 0xD1, 0x84, 0xE0, 0x9A, 0xA6, 0xE7, 0x71, 
    0x51, 0xBB, 0x42, 0x23, 0xD6, 0xD6, 0x4D, 0xFA, 
    0xED, 0x82, 0xCA, 0x25, 0x74, 0xEE, 0x7F, 0x29, 
    0xD2, 0x84, 0x6F, 0x87, 0x3B, 0xBC, 0xAD, 0xF2, 
    0x80, 0x56, 0xEA, 0xC7, 0x5E, 0x14, 0xF1, 0x6A, 
    0x2D, 0x75, 0xE1, 0x8E, 0xB5, 0x6C, 0x36, 0xB7, 
    0xBC, 0x6F, 0xA5, 0xC7, 0x3B, 0x31, 0x41, 0x09, 
    0x98, 0x47, 0x04, 0x9B, 0x72, 0x30, 0x6D, 0x3D, 
    0xD2, 0x75, 0x68, 0x64, 0x85, 0x10, 0xCF, 0x67, 
};

/* RSA 1024 ciphertext of first 10 bytes of m[] with seed that produces
   a zero in the padding that must be replaced */

static unsigned char c_1024_pad0[]   __attribute__((aligned (4))) = {
    0x9C, 0xB3, 0x03, 0x31, 0x6B, 0xEA, 0xBB, 0x8C, 
    0xB2, 0x8A, 0xAC, 0x62, 0x08, 0xBD, 0xC3, 0x34, 
    0x0C, 0xDE, 0xB5, 0x6A, 0x91, 0xBE, 0xBA, 0x9D, 
    0x7D, 0x06, 0x33, 0x5C, 0x34, 0x03, 0x89, 0x2C, 
    0xC5, 0x9C, 0x3B, 0x39, 0x91, 0xC1, 0xBA, 0xF4, 
    0xEE, 0x34, 0xAF, 0x04, 0x79, 0x1B, 0x79, 0x3D, 
    0x81, 0xAE, 0x03, 0x0A, 0x3F, 0x6E, 0x2B, 0xAD, 
    0xB3, 0xD1, 0x84, 0xE0, 0x9A, 0xA6, 0xE7, 0x71, 
    0x51, 0xBB, 0x42, 0x23, 0xD6, 0xD6, 0x4D, 0xFA, 
    0xED, 0x82, 0xCA, 0x25, 0x74, 0xEE, 0x7F, 0x29, 
    0xD2, 0x84, 0x6F, 0x87, 0x3B, 0xBC, 0xAD, 0xF2, 
    0x80, 0x56, 0xEA, 0xC7, 0x5E, 0x14, 0xF1, 0x6A, 
    0x2D, 0x75, 0xE1, 0x8E, 0xB5, 0x6C, 0x36, 0xB7, 
    0xBC, 0x6F, 0xA5, 0xC7, 0x3B, 0x31, 0x41, 0x09, 
    0x98, 0x47, 0x04, 0x9B, 0x72, 0x30, 0x6D, 0x3D, 
    0xD2, 0x75, 0x68, 0x64, 0x85, 0x10, 0xCF, 0x67, 
};

/* RSA 1024 ciphertext of first 128-11 bytes of m[] */

static unsigned char c_full_m_1024[]   __attribute__((aligned (4))) = {
    0x2A, 0x07, 0x14, 0x13, 0xAC, 0x61, 0xA2, 0xEF, 
    0x84, 0x3C, 0xFF, 0x2E, 0x21, 0x35, 0xFE, 0x0A, 
    0x07, 0x61, 0x5D, 0xB2, 0x17, 0x17, 0x3B, 0xC0, 
    0x04, 0x8F, 0x04, 0x46, 0xC9, 0x6C, 0x75, 0x51, 
    0x41, 0xF6, 0xE7, 0xDD, 0x35, 0x00, 0x0B, 0xB4, 
    0x16, 0xF6, 0x26, 0xB4, 0xB2, 0x96, 0xAE, 0xCC, 
    0x67, 0xE0, 0xCE, 0x8E, 0xDE, 0xDF, 0x30, 0xF7, 
    0x96, 0x1F, 0xA5, 0x9D, 0xBD, 0xB2, 0x02, 0x60, 
    0xDC, 0x9E, 0xDB, 0xAD, 0x9F, 0x49, 0x29, 0x84, 
    0x74, 0x4F, 0x54, 0x02, 0x18, 0xBC, 0xC2, 0x13, 
    0x4E, 0xED, 0xEB, 0x06, 0x17, 0x05, 0xFA, 0xD4, 
    0x7E, 0x6D, 0xA3, 0xC0, 0xA3, 0x27, 0x16, 0x0C, 
    0x32, 0x04, 0xF4, 0x48, 0x34, 0x2E, 0xDF, 0x89, 
    0xE2, 0xF0, 0xA9, 0x98, 0x9E, 0x2D, 0xED, 0xD9, 
    0xA6, 0xC6, 0x13, 0x10, 0x9B, 0xA5, 0xB9, 0xCD, 
    0xD2, 0xFE, 0x60, 0x80, 0x54, 0x3E, 0x69, 0xCC, 
};

/* an invalid RSA-1024 ciphertext */

static unsigned char bad_c_1024[]   __attribute__((aligned (4))) ={
    0x72, 0x61, 0x48, 0x5a, 0xab, 0x1d, 0x18, 0xb6, 
    0xc8, 0x76, 0x9a, 0x01, 0x00, 0x99, 0x5c, 0x9c, 
    0x8e, 0x93, 0x70, 0xcf, 0xf0, 0x25, 0xc2, 0x52, 
    0x55, 0x17, 0x51, 0x97, 0x3e, 0x94, 0xf0, 0xb6, 
    0x77, 0x37, 0xe1, 0x1f, 0xfc, 0x92, 0x6f, 0x23, 
    0xfa, 0xc9, 0x12, 0x63, 0xbd, 0x9e, 0xf4, 0x21, 
    0x65, 0x0f, 0xdd, 0x7a, 0xdc, 0x2f, 0x9f, 0xba, 
    0xef, 0x5c, 0xf0, 0xd5, 0xe0, 0x5b, 0xbf, 0x75, 
    0x44, 0xa2, 0xfb, 0x72, 0x29, 0x7c, 0x3a, 0x49, 
    0xca, 0xdb, 0x4e, 0x8d, 0x58, 0x54, 0x2a, 0x92, 
    0xa2, 0xf1, 0x30, 0x5c, 0xe0, 0x82, 0x0e, 0xba, 
    0xec, 0xa1, 0x66, 0x70, 0x23, 0xbb, 0x6d, 0xaf, 
    0xa3, 0x3b, 0x92, 0xc0, 0x66, 0x57, 0x31, 0x71, 
    0xca, 0x5b, 0x2c, 0x4b, 0xcb, 0x6b, 0x53, 0xf4, 
    0x63, 0xae, 0x1b, 0x28, 0x07, 0xf1, 0xa4, 0x38, 
    0xfa, 0x36, 0x13, 0x57, 0x92, 0xaf, 0xd7, 0xee,
};

/* these are the maximum sizes of xbuf and ybuf, i.e. needed when
   encrypting with RSA-2048 */

#if defined (__dsPIC33F__) || defined (__dsPIC33E__) || defined (__dsPIC30F__)
    static unsigned char __attribute__ ((aligned (64), space(xmemory))) xbuf[512];
    static unsigned char __attribute__ ((aligned (2), space(ymemory), eds)) ybuf[768];
#elif defined (__PIC24F__) || defined (__PIC24H__)
    uint8_t      xbuf[(1024*2)/8] __attribute__((aligned (4)));
    uint8_t      ybuf[1024*2/8] __attribute__((aligned (4)));
#endif

/* here are buffers for the decrypted message and ciphertext */
/* (large enough to handle RSA-2048 functions) */

static unsigned char dec_m[256-11]  __attribute__((aligned (4)));
static unsigned char c[256]  __attribute__((aligned (4)));

/* Keys */
RSA_SW_PUBLIC_KEY rsaPublicKey = 
{
    128,
    rsa_n_1024,
    3,
    rsa_e
};

RSA_SW_PRIVATE_KEY_CRT rsaPrivateKey = 
{
    128,
    rsa_p_1024,
    rsa_q_1024,
    rsa_dp_1024,
    rsa_dq_1024,
    rsa_qinv_1024
};

/* Fake Random Number Generation */
uint32_t getNotRandom (void);
uint32_t getZeroThenRandom(void);
void seedZero (void);

bool getZeros = true;

uint32_t getNotRandom (void)
{
    return 0x55555555;
}

uint32_t getZeroThenRandom (void)
{
    if (getZeros)
    {
        getZeros = false;
        return 0x00000000;
    }
    else
    {
        return 0x55555555;
    }
}

void seedZero (void)
{
    getZeros = true;
}

int main(void)
{
    SYS_MODULE_OBJ sys_obj;
    DRV_HANDLE drv_handle;
    int cfg;
    RSA_SW_STATUS status;
    uint16_t dec_m_byte_len;
    int i;

/*************************************************************************/
/* Test RSA encryption and decryption                                    */
/*************************************************************************/

    /**********************************************************/
    /* Test RSA-1024 encryption error conditions */
    /**********************************************************/

    sys_obj = RSA_SW_Initialize (RSA_SW_INDEX, NULL);
    if (sys_obj != SYS_MODULE_OBJ_STATIC)
    {
        return false;
    }

    drv_handle = RSA_SW_Open (RSA_SW_INDEX, DRV_IO_INTENT_EXCLUSIVE | DRV_IO_INTENT_BLOCKING);
    if (drv_handle != RSA_SW_HANDLE)
    {
        return false;
    }

    cfg = RSA_SW_Configure (drv_handle, xbuf, ybuf, sizeof (xbuf), sizeof (ybuf), (RSA_SW_RandomGet)getNotRandom, RSA_SW_PAD_DEFAULT);
    if (cfg == -1)
    {
        return false;
    }

    /* the byte length of the modulus n must be either 128 or 256 */
    /* make sure any other value returns an error */

    rsaPublicKey.nLen = 129;

    if ((status = RSA_SW_Encrypt (drv_handle, c, m, m_byte_len, &rsaPublicKey)) != RSA_SW_STATUS_ERROR) 
    {
        return 2;
    }

    rsaPublicKey.nLen = 128;

    /* make sure the msb and lsb are set for the modulus n for RSA-1024 */

    rsa_n_1024[127] &= 0x7f;                /* clear msb of n */

    if ((status = RSA_SW_Encrypt (drv_handle, c, m, m_byte_len, &rsaPublicKey)) != RSA_SW_STATUS_BAD_PARAM) 
    {
        return 3;
    }

    rsa_n_1024[127] |= 0x80;                  /* set msb of n */
    rsa_n_1024[0] &= 0xfe;                   /* clear lsb of n */

    if ((status = RSA_SW_Encrypt (drv_handle, c, m, m_byte_len, &rsaPublicKey)) != RSA_SW_STATUS_BAD_PARAM) 
    {
        return 4;
    }

    rsa_n_1024[0] |= 0x01;                   /* set lsb of n */

    /* make sure that oversized messages are caught for RSA-1024 */

    if ((status = RSA_SW_Encrypt (drv_handle, c, m, full_m_byte_len_1024 + 1, &rsaPublicKey)) != RSA_SW_STATUS_ERROR) 
    {
        return 5;
    }

    /* make sure that zero-sized exponents are caught */

    rsaPublicKey.eLen = 0;

    if ((status = RSA_SW_Encrypt (drv_handle, c, m, m_byte_len, &rsaPublicKey)) != RSA_SW_STATUS_ERROR) 
    {
        return 6;
    }

    rsaPublicKey.eLen = 129;

    /* make sure that oversized exponents are caught for RSA-1024 */

    if ((status = RSA_SW_Encrypt (drv_handle, c, m, m_byte_len, &rsaPublicKey)) != RSA_SW_STATUS_ERROR) 
    {
        return 7;
    }

    rsaPublicKey.eLen = 3;
    rsaPublicKey.exp = e_bad;

    /* make sure that e is odd and e > 1 */
    
    if ((status = RSA_SW_Encrypt (drv_handle, c, m, m_byte_len, &rsaPublicKey)) != RSA_SW_STATUS_BAD_PARAM) 
    {
        return 8;
    }

    e_bad[0] = 1;
    if ((status = RSA_SW_Encrypt (drv_handle, c, m, m_byte_len, &rsaPublicKey)) != RSA_SW_STATUS_BAD_PARAM) 
    {
        return 9;
    }
    e_bad[0] = 0;

    rsaPublicKey.exp = rsa_e;

    /* make sure that we detect when the ciphertext buffer overlaps with
       top k bytes of xbuf, when k = 128 (for RSA-1024)*/
    #if defined (__dsPIC33F__) || defined(__dsPIC33E__)
    if ((status = RSA_SW_Encrypt (drv_handle, xbuf + 1, m, m_byte_len, &rsaPublicKey)) != RSA_SW_STATUS_BAD_PARAM) 
    {
        return 10;
    }
    
    if ((status = RSA_SW_Encrypt (drv_handle, xbuf + 255, m, m_byte_len, &rsaPublicKey)) != RSA_SW_STATUS_BAD_PARAM) 
    {
        return 11;
    }
    #endif    

    /**********************************************************/
    /* Test RSA-1024 decryption error conditions */
    /**********************************************************/

    /* the byte length of the modulus n must be either 128 or 256 */
    /* make sure any other value returns an error */

    rsaPrivateKey.nLen = 129;

    if ((status = RSA_SW_Decrypt (drv_handle, dec_m, c, &dec_m_byte_len, &rsaPrivateKey)) != RSA_SW_STATUS_ERROR)
    {
        return 12;
    }

    rsaPrivateKey.nLen = 128;
    
    /* make sure that we detect when the message buffer overlaps with
       bottom k bytes of xbuf, when k = 128 (for RSA-1024)*/

    #if defined (__dsPIC33F__) || defined(__dsPIC33E__)
    if ((status = RSA_SW_Decrypt (drv_handle, xbuf - full_m_byte_len_1024 + 1, c, &dec_m_byte_len, &rsaPrivateKey)) != RSA_SW_STATUS_BAD_PARAM)
    {
        return 13;
    }

    if ((status = RSA_SW_Decrypt (drv_handle, xbuf + full_m_byte_len_1024 - 1, c, &dec_m_byte_len, &rsaPrivateKey)) != RSA_SW_STATUS_BAD_PARAM)
    {
        return 14;
    }
    #endif

    /* make sure the msb and lsb are set for the modulus p for RSA-1024 */

    rsa_p_1024[0] &= 0xfe;                 /* clear lsb of p */

    if ((status = RSA_SW_Decrypt (drv_handle, dec_m, c, &dec_m_byte_len, &rsaPrivateKey)) != RSA_SW_STATUS_BAD_PARAM)
    {
        return 15;
    }

    rsa_p_1024[0] |= 0x01;                 /* set lsb of p */
    rsa_p_1024[63] &= 0x7f;                /* clear msb of p */

    if ((status = RSA_SW_Decrypt (drv_handle, dec_m, c, &dec_m_byte_len, &rsaPrivateKey)) != RSA_SW_STATUS_BAD_PARAM)
    {
        return 16;
    }

    rsa_p_1024[63] |= 0x80;                /* set msb of p */

    /* make sure the msb and lsb are set for the modulus q for RSA-1024 */

    rsa_q_1024[0] &= 0xfe;                /* clear lsb of q */

    if ((status = RSA_SW_Decrypt (drv_handle, dec_m, c, &dec_m_byte_len, &rsaPrivateKey)) != RSA_SW_STATUS_BAD_PARAM)
    {
        return 17;
    }

    rsa_q_1024[0] |= 0x01;                /* set lsb of q */
    rsa_q_1024[63] &= 0x7f;               /* clear msb of q */

    if ((status = RSA_SW_Decrypt (drv_handle, dec_m, c, &dec_m_byte_len, &rsaPrivateKey)) != RSA_SW_STATUS_BAD_PARAM)
    {
        return 18;
    }

    rsa_q_1024[63] |= 0x80;               /* set msb of q */

    /*************************************************/
    /* Test valid RSA-1024 encryption and decryption */
    /*************************************************/

    /* seed the DRBG */
    cfg = RSA_SW_Configure (drv_handle, xbuf, ybuf, sizeof (xbuf), sizeof (ybuf), (RSA_SW_RandomGet)getZeroThenRandom, RSA_SW_PAD_DEFAULT);
    if (cfg == -1)
    {
        return false;
    }

    /* encrypt a short message */
    if ((status = RSA_SW_Encrypt (drv_handle, c, m, m_byte_len, &rsaPublicKey)) != RSA_SW_STATUS_READY) 
    {
        return 20;
    }

    cfg = RSA_SW_Configure (drv_handle, xbuf, ybuf, sizeof (xbuf), sizeof (ybuf), (RSA_SW_RandomGet)getNotRandom, RSA_SW_PAD_DEFAULT);
    if (cfg == -1)
    {
        return false;
    }

    /* check against the known ciphertext (when the rng produces zeros) */

    for (i = 0; i < 128; i++) {
        if (c[i] != c_1024[i]) {
            return 21;
        }
    }
    
    /* decrypt the ciphertext */
    if ((status = RSA_SW_Decrypt (drv_handle, dec_m, c, &dec_m_byte_len, &rsaPrivateKey)) != RSA_SW_STATUS_READY)
    {
        return 22;
    }

    /* and make sure the message retrieved is equal to the original one */

    if (dec_m_byte_len != m_byte_len)
       return 23;

    for (i = 0; i < m_byte_len; i++) {
        if (dec_m[i] != m[i]) {
            return 24;
        }
    }

    /* flip a byte of the ciphertext, and attempt to decrypt it */
    /* - make sure MCL_INVALID_CIPHERTEXT is returned */

    c[10] ^= 0xff;
    if ((status = RSA_SW_Decrypt (drv_handle, dec_m, c, &dec_m_byte_len, &rsaPrivateKey)) != RSA_SW_STATUS_ERROR)
    {
        return 25;
    }

    /* attempt to decrypt the given invalid ciphertext */
    /* - make sure MCL_INVALID_CIPHERTEXT is returned */

    if ((status = RSA_SW_Decrypt (drv_handle, dec_m, bad_c_1024, &dec_m_byte_len, &rsaPrivateKey)) != RSA_SW_STATUS_ERROR)
    {
        return 26;
    }

    /* -- test with padding that produces a zero to be replaced -- */

    /* seed the DRBG */
    cfg = RSA_SW_Configure (drv_handle, xbuf, ybuf, sizeof (xbuf), sizeof (ybuf), (RSA_SW_RandomGet)getZeroThenRandom, RSA_SW_PAD_DEFAULT);
    if (cfg == -1)
    {
        return 27;
    }

    /* encrypt a short message */
    if ((status = RSA_SW_Encrypt (drv_handle, c, m, m_byte_len, &rsaPublicKey)) != RSA_SW_STATUS_READY) 
    {
        return 28;
    }

    cfg = RSA_SW_Configure (drv_handle, xbuf, ybuf, sizeof (xbuf), sizeof (ybuf), (RSA_SW_RandomGet)getNotRandom, RSA_SW_PAD_DEFAULT);
    if (cfg == -1)
    {
        return false;
    }

    /* check against the known ciphertext (when the rnd is seeded as above) */

    for (i = 0; i < 128; i++) {
        if (c[i] != c_1024_pad0[i]) {
            return 29;
        }
    }
    
    /* decrypt the ciphertext */
    if ((status = RSA_SW_Decrypt (drv_handle, dec_m, c, &dec_m_byte_len, &rsaPrivateKey)) != RSA_SW_STATUS_READY)
    {
        return 30;
    }

    /* and make sure the message retrieved is equal to the original one */

    if (dec_m_byte_len != m_byte_len)
        return 31;

    for (i = 0; i < m_byte_len; i++) {
        if (dec_m[i] != m[i]) {
            return 32;
        }
    }

    /* -- test a long message -- */

    /* encrypt the message */
    if ((status = RSA_SW_Encrypt (drv_handle, c, m, full_m_byte_len_1024, &rsaPublicKey)) != RSA_SW_STATUS_READY) 
    {
        return 34;
    }

    /* check against the known ciphertext (when the drbg is seeded as above) */

    for (i = 0; i < 128; i++) {
        if (c[i] != c_full_m_1024[i]) {
            return 35;
        }
    }

    /* decrypt the message */
    if ((status = RSA_SW_Decrypt (drv_handle, dec_m, c, &dec_m_byte_len, &rsaPrivateKey)) != RSA_SW_STATUS_READY)
    {
        return 36;
    }

    /* confirm the original message is retrieved */

    if (dec_m_byte_len != full_m_byte_len_1024)
        return 37;

    for (i = 0; i < full_m_byte_len_1024; i++) {
        if (dec_m[i] != m[i]) {
            return 38;
        }
    }

    /* -- test the null message -- */

    /* encrypt the null message */
    if ((status = RSA_SW_Encrypt (drv_handle, c, m, null_m_byte_len, &rsaPublicKey)) != RSA_SW_STATUS_READY) 
    {
        return 40;
    }

    /* check against the known ciphertext (when the rng is seeded as above) */

    for (i = 0; i < 128; i++) {
        if (c[i] != c_null_m_1024[i]) {
            return 41;
        }
    }

    /* decrypt the null message */
    if ((status = RSA_SW_Decrypt (drv_handle, dec_m, c, &dec_m_byte_len, &rsaPrivateKey)) != RSA_SW_STATUS_READY)
    {
        return 42;
    }

    /* confirm that the decrypted byte length is zero */

    if (dec_m_byte_len != null_m_byte_len)
        return 43;

    RSA_SW_Close (drv_handle);
    RSA_SW_Deinitialize (sys_obj);

    return 0;
}
