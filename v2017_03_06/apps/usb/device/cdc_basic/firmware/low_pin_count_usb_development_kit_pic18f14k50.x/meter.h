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

    void meter_enable();
    void meter_set_freq(int hz);
    void meter_do_pulse();


#ifdef	__cplusplus
}
#endif

#endif	/* METER_H */

