/*
 * File:   meter.c
 * Author: maina
 *
 * Created on 2019/06/02, 16:53
 */


#include <xc.h>
#include "meter.h"
#include <string.h>
#include <stdlib.h>

#define TRIS TRISCbits.TRISC0
#define PIN_INPUT  1
#define PIN_OUTPUT 0

#define LAT LATCbits.LATC0
#define LAT_ON  1
#define LAT_OFF 0

#define TMR0_MAX 255;
#define T0CS_INTERNAL 0;



static int freqHz = 1000;

void meter_enable() {
    T0IF = 0; // TMR0割り込みフラグ初期化クリア
    ANSELHbits.ANS8 = 0; // アナログ出力オフ
    TRIS = PIN_OUTPUT; 
    LAT = LAT_ON; // 出力ON
    RC0 = 1;
    T08BIT = 0; // 16bitカウンタ
    TMR0 = TMR0_MAX;      // TMR0のカウンタ、256でON
    T0CS = T0CS_INTERNAL; // TMR0のオシレータは内部
    PSA  = 0;
    T0PS0 = 1; // 1/256プリスケーラ
    T0PS1 = 1;
    T0PS2 = 1;
    T0IE = 1; // TMR0割り込み許可
    meter_set_cpuusage(50);
}

void meter_set_freq(int hz) {
    freqHz = hz;
}

void meter_set_cpuusage(int percent) {
    meter_set_freq(percent * 1.66);
}

long getInterval() {
    double waitClock = 0.0;
    double wantClock = freqHz;/**freqHz;*/
    double actualClock = 12000000.0/256.0; /** 12Mhz/256 */
    double waitClockDivided = 0.0;
    waitClock = actualClock/wantClock;
    waitClockDivided = waitClock / 2; /** 立ち上がり、立下りのため*/
    double ret = 65536-waitClockDivided;
    return ret;
}

void meter_do_pulse() {
    // 前回ONならば
    if (LAT == 1) {
        // 今回はOFF
        LAT = 0;
        // 次ONにするタイミング設定
        TMR0 = getInterval();           // サイクルごとにインクリメント、TMR256カウントで割込み
    }
    // 前回OFFならば
    else
    {
        // 今回はON
        LAT = 1;
        // 次OFFにするタイミング設定
        TMR0 = getInterval();           // サイクルごとにインクリメント、TMR256カウントで割込み
    }
    // 割り込みフラグ初期化
    INTCONbits.TMR0IF = 0; // 割り込みフラグクリア
}