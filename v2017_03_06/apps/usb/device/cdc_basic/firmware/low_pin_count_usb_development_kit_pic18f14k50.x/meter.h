/* 
 * File:   meter.h
 * Author: maina
 *
 * Created on 2019/06/02, 16:53
 */

#ifndef METER_H
#define	METER_H

#ifdef	__cplusplus
extern "C" {
#endif
    int freqHz;
    void meter_enable();
    void meter_do_pulse();
    void meter_set_cpuusage(int);
    long getInterval();


#ifdef	__cplusplus
}
#endif

#endif	/* METER_H */

