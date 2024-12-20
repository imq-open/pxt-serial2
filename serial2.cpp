#include "pxt.h"
#include "./NRF52Serial2.h"

#define MICROBIT_SERIAL_READ_BUFFER_LENGTH 64

// make sure USB_TX and USB_RX don't overlap with other pin ids
// also, 1001,1002 need to be kept in sync with getPin() function
enum SerialPin
{
};

enum BaudRate
{
};

// Macto expansion not allowed for enum values: this is how pxt works
#if 0
enum EventBusSource
{
    //% blockIdentity="control.eventSourceId"
    SERIAL2_DEVICE_ID = IMQOPEN_NRF52SERIAL2_DEFAULT_DEVICE_ID,
};

enum EventBusValue
{
    //% blockIdentity="control.eventValueId"
    SERIAL2_EVT_DELIM_MATCH = IMQOPEN_NRF52SERIAL2_EVT_DELIM_MATCH,
    //% blockIdentity="control.eventValueId"
    SERIAL2_EVT_HEAD_MATCH = IMQOPEN_NRF52SERIAL2_EVT_HEAD_MATCH,
    //% blockIdentity="control.eventValueId"
    SERIAL2_EVT_RX_FULL = IMQOPEN_NRF52SERIAL2_EVT_RX_FULL,
    //% blockIdentity="control.eventValueId"
    SERIAL2_EVT_DATA_RECEIVED = IMQOPEN_NRF52SERIAL2_EVT_DATA_RECEIVED,
    //% blockIdentity="control.eventValueId"
    SERIAL2_EVT_ERROR_OVERRUN = IMQOPEN_NRF52SERIAL2_EVT_ERROR_OVERRUN,
    //% blockIdentity="control.eventValueId"
    SERIAL2_EVT_ERROR_FRAMING = IMQOPEN_NRF52SERIAL2_EVT_ERROR_FRAMING,
    //% blockIdentity="control.eventValueId"
    SERIAL2_EVT_ERROR_BREAK = IMQOPEN_NRF52SERIAL2_EVT_ERROR_BREAK,
};
#else
enum EventBusSource
{
    //% blockIdentity="control.eventSourceId"
    SERIAL2_DEVICE_ID = 70,
};

enum EventBusValue
{
    //% blockIdentity="control.eventValueId"
    SERIAL2_EVT_DELIM_MATCH = 1,
    //% blockIdentity="control.eventValueId"
    SERIAL2_EVT_HEAD_MATCH = 2,
    //% blockIdentity="control.eventValueId"
    SERIAL2_EVT_RX_FULL = 3,
    //% blockIdentity="control.eventValueId"
    SERIAL2_EVT_DATA_RECEIVED = 4,
    //% blockIdentity="control.eventValueId"
    SERIAL2_EVT_ERROR_OVERRUN = 10,
    //% blockIdentity="control.eventValueId"
    SERIAL2_EVT_ERROR_FRAMING = 12,
    //% blockIdentity="control.eventValueId"
    SERIAL2_EVT_ERROR_BREAK = 13,
};
#endif

//%
namespace serial2
{

    imqopen::NRF52Serial2 serial2(uBit.io.P13, uBit.io.P14);
    // bool is_redirected;

    // note that at least one // followed by % is needed per declaration!

    //%
    bool isEnabled()
    {
        return serial2.isEnabled();
    }

    //%
    bool setEnabled(bool enabled)
    {
        return DEVICE_OK == serial2.setEnabled(enabled);
    }

    //%
    String readUntil(String delimiter)
    {
        return PSTR(serial2.readUntil(MSTR(delimiter)));
    }

    //%
    String readString()
    {
        int n = serial2.getRxBufferSize();
        if (n == 0)
            return mkString("", 0);
        return PSTR(serial2.read(n, MicroBitSerialMode::ASYNC));
    }

    //%
    void onDataReceived(String delimiters, Action body)
    {
        serial2.eventOn(MSTR(delimiters));
        registerWithDal(SERIAL2_DEVICE_ID, MICROBIT_SERIAL_EVT_DELIM_MATCH, body);
        // lazy initialization of serial buffers
        serial2.read(MicroBitSerialMode::ASYNC);
    }

    //%
    void writeString(String text)
    {
        if (!text)
            return;

        serial2.send(MSTR(text));
    }

    //%
    void writeBuffer(Buffer buffer)
    {
        if (!buffer)
            return;

        serial2.send(buffer->data, buffer->length);
    }

    //%
    Buffer readBuffer(int length)
    {
        auto mode = SYNC_SLEEP;
        if (length <= 0)
        {
            length = serial2.getRxBufferSize();
            mode = ASYNC;
        }

        auto buf = mkBuffer(NULL, length);
        auto res = buf;
        registerGCObj(buf); // make sure buffer is pinned, while we wait for data
        int read = serial2.read(buf->data, buf->length, mode);
        if (read != length)
        {
            res = mkBuffer(buf->data, read);
        }
        unregisterGCObj(buf);

        return res;
    }

    //%
    void redirect(SerialPin tx, SerialPin rx, BaudRate rate)
    {

        if (getPin(tx) && getPin(rx))
        {
            serial2.redirect(*getPin(tx), *getPin(rx));
            // is_redirected = 1;
        }
        serial2.setBaud(rate);
    }

    //%
    void setBaudRate(BaudRate rate)
    {
        serial2.setBaud(rate);
    }

    //%
    void redirectToUSB()
    {
        // is_redirected = false;
        serial2.redirect(uBit.io.usbTx, uBit.io.usbRx);
        serial2.setBaud(115200);
    }

    //%
    void setRxBufferSize(uint8_t size)
    {
        serial2.setRxBufferSize(size);
    }

    //%
    void setTxBufferSize(uint8_t size)
    {
        serial2.setTxBufferSize(size);
    }

} // namespace serial2
