

let serial2_break = false

basic.forever(function () {

    if (serial2.isEnabled()) {
        serial2.writeString(input.runningTime() + '\n')
        basic.pause(1000)
    }

})

control.onEvent(EventBusSource.SERIAL2_DEVICE_ID, EventBusValue.SERIAL2_EVT_DATA_RECEIVED, function () {
    let buf = serial2.readBuffer(0)
    if (buf.length) {
        serial2.writeString('>>')
        serial2.writeBuffer(buf)
        serial2.writeString('<<\n')
    }
})

control.onEvent(EventBusSource.SERIAL2_DEVICE_ID, EventBusValue.SERIAL2_EVT_ERROR_FRAMING, function () {
    serial2.writeString('!!frame!!\n')
})

control.onEvent(EventBusSource.SERIAL2_DEVICE_ID, EventBusValue.SERIAL2_EVT_ERROR_BREAK, function () {
    serial2_break = true
    serial2.writeString('!!break!!\n')

    serial2.setEnabled(false)
    basic.pause(1)
    pins.digitalWritePin(DigitalPin.P13, 0)
    basic.pause(5000)
    serial2.setEnabled(true)
    serial2.writeString('!!break done!!\n')

    if (serial2_break) {
        serial2_break = false
        serial2.writeString('!!RX hi!! (pins.onPulsed() missed)\n')
    }

})

serial2.setBaudRate(BaudRate.BaudRate2400)
serial2.readString()

pins.onPulsed(DigitalPin.P14, PulseValue.High, function () {
    if (serial2_break) {
        serial2_break = false
        serial2.writeString('!!RX hi!!\n')
    }
})
