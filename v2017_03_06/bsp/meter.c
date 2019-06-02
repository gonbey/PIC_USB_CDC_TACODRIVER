/*
 * File:   meter.c
 * Author: maina
 *
 * Created on 2019/06/02, 14:12
 */


#include <xc.h>
#include <leds.h>
#include <stdbool.h>
#include "meter.h"

#define METER_LAT LATCbits.LATC0

#define METER_TRIS TRISCbits.TRISC0

#define LED_ON  1
#define LED_OFF 0

#define PIN_INPUT  1
#define PIN_OUTPUT 0

int current_value = 0;

void meter_set(int value) {
    current_value = value;
    return;
}

// 一定時間ごとに呼ばれる
// メーターにパルス出力    
void meter_dopulse() {
    // 前回ONならば
    if (METER_LAT == 1) {
        // 今回はOFF
        METER_LAT = 0;
        // 次ONにするタイミング設定
        TMR0 = 0;           // サイクルごとにインクリメント、TMR256カウントで割込み
    }
    // 前回OFFならば
    else
    {
        // 今回はON
        METER_LAT = 1;
        // 次OFFにするタイミング設定
        TMR0 = 0;           // サイクルごとにインクリメント、TMR256カウントで割込み
    }
    // 割り込みフラグ初期化
     INTCONbits.TMR0IF = 0; // 割り込みフラグクリア
}

void meter_enable(void) {
    T0IF = 0; // TMR0割り込みクリア
    ANSELHbits.ANS8 = 0; // アナログ出力オフ
    METER_TRIS = 0; // 出力
    PORTCbits.RC0 = 1;  // ポートをONにしておく、ラッチで制御する。
    METER_LAT = 1;  // 出力オフ
    // 次のサイクルで初回割込みに入る
    TMR0 = 255;// サイクルごとにインクリメント、TMR256カウントで割込み
    T0CONbits.T0CS = 0; // timer0を駆動する内部クロックを選択します
    T0CONbits.PSA = 0x101; //プリスケーラをtimer0に割り当てます。64 101 
    T0CONbits.T0PS = 0; // DOUBLE CHECK = / 2 //
    T0CONbits.T0PS1 = 1; // T0CONbits.T0PS0 = 1; //クロックを8分周します
    T0IE = 1; // TMR0割り込み許可
}