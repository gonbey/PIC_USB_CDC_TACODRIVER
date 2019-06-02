/*
 * File:   myisr.c
 * Author: maina
 *
 * Created on 2019/06/02, 14:21
 */


#include <xc.h>
#include "meter.h"

void myisr(void) {
    // タイマー割り込み
    if (INTCONbits.TMR0IF)
    {
        meter_dopulse();
    }
    return;
}
