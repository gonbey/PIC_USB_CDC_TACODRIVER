#include "app_device_cdc_basic.h"
#include "my_util.h"

/**
 * 毎回メインループに戻してから読んでね！連続駄目よ！
 * @param buffer
 * @return 
 */
int readData(char* buffer) {
    if(essentialCheck() == 0) {
        // 読み込み！
        int numBytesRead = getsUSBUSART(buffer, sizeof(buffer));
        return numBytesRead;
    } else {
        return -1;
    }
}

/**
 * 毎回メインループに戻してから読んでね！連続駄目よ！
 * @param buffer
 * @param numBytes
 * @return 
 */
int writeData(char* buffer, int numBytes) {
    if(essentialCheck() == 0) {
        // システムに値をセット
        putUSBUSART(buffer, numBytes);
        return 0;
    }
    else {
        return -1;
    }
}

int essentialCheck() {
     /* If the USB device isn't configured yet, we can't really do anything
     * else since we don't have a host to talk to.  So jump back to the
     * top of the while loop. */
    if( USBGetDeviceState() < CONFIGURED_STATE )
    {
        return -1;
    }

    /* If we are currently suspended, then we need to see if we need to
     * issue a remote wakeup.  In either case, we shouldn't process any
     * keyboard commands since we aren't currently communicating to the host
     * thus just continue back to the start of the while loop. */
    if( USBIsDeviceSuspended()== true )
    {
        return -1;
    }
    if (USBUSARTIsTxTrfReady() != 1)
    {
        return -1;
    }
    return 0;
}