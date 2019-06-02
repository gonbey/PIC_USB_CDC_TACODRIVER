/*
 * File:   meter.c
 * Author: maina
 *
 * Created on 2019/06/02, 16:53
 */


#include <xc.h>
#include "meter.h"

int freqHz;

#define TRIS TRISCbits.TRISC0
#define PIN_INPUT  1
#define PIN_OUTPUT 0

#define LAT LATCbits.LATC0
#define LAT_ON  1
#define LAT_OFF 0

#define TMR0_MAX 255;
#define T0CS_INTERNAL 0;



int freqHz = 1000;

void meter_enable() {
    T0IF = 0; // TMR0割り込みフラグ初期化クリア
    ANSELHbits.ANS8 = 0; // アナログ出力オフ
    TRIS = PIN_OUTPUT; 
    LAT = LAT_ON; // 出力ON
    RC0 = 1;
    TMR0 = TMR0_MAX;      // TMR0のカウンタ、256でON
    T0CS = T0CS_INTERNAL; // TMR0のオシレータは内部
    PSA  = 0x101; // TMR0のプリスケーラ
    T0PS0 = 1; // 1/256プリスケーラ
    T0PS1 = 1;
    T0PS2 = 1;
    T0IE = 1; // TMR0割り込み許可
}

void meter_set_freq(int hz) {
    freqHz = hz;
}

void meter_do_pulse() {
    // 前回ONならば
    if (LAT == 1) {
        // 今回はOFF
        LAT = 0;
        // 次ONにするタイミング設定
        TMR0 = 0;           // サイクルごとにインクリメント、TMR256カウントで割込み
    }
    // 前回OFFならば
    else
    {
        // 今回はON
        LAT = 1;
        // 次OFFにするタイミング設定
        TMR0 = 0;           // サイクルごとにインクリメント、TMR256カウントで割込み
    }
    // 割り込みフラグ初期化
    INTCONbits.TMR0IF = 0; // 割り込みフラグクリア
}