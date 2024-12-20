//%
namespace serial2 {

    /**
     * Read a line of text from the serial port and return the buffer when the delimiter is met.
     * @param delimiter text delimiter that separates each text chunk
     */
    //% help=serial/read-until
    //% blockId=serial2_read_until block="serial2|read until %delimiter=serial_delimiter_conv"
    //% weight=19 shim=serial2::readUntil
    export function readUntil(delimiter: string): string {
        return ""
    }

    /**
     * Read the buffered received data as a string
     */
    //% help=serial/read-string
    //% blockId=serial2_read_buffer block="serial2|read string"
    //% weight=18 shim=serial2::readString
    export function readString(): string {
        return ""
    }

    /**
     * Register an event to be fired when one of the delimiter is matched.
     * @param delimiters the characters to match received characters against.
     */
    //% help=serial/on-data-received
    //% weight=18 blockId=serial2_on_data_received block="serial2|on data received %delimiters=serial_delimiter_conv" shim=serial2::onDataReceived
    export function onDataReceived(delimiters: string, body: () => void): void {
        return
    }

    /**
     * Send a piece of text through the serial connection.
     */
    //% help=serial/write-string
    //% weight=87 blockGap=8
    //% blockId=serial2_writestring block="serial2|write string %text"
    //% text.shadowOptions.toString=true shim=serial2::writeString
    export function writeString(text: string): void {
        return
    }

    /**
     * Send a buffer through serial connection
     */
    //% blockId=serial2_writebuffer block="serial2|write buffer %buffer=serial_readbuffer"
    //% help=serial/write-buffer advanced=true weight=6 shim=serial2::writeBuffer
    export function writeBuffer(buffer: Buffer): void {
        return
    }

    /**
     * Read multiple characters from the receive buffer.
     * If length is positive, pauses until enough characters are present.
     * @param length default buffer length
     */
    //% blockId=serial2_readbuffer block="serial2|read buffer %length"
    //% help=serial/read-buffer advanced=true weight=5 shim=serial2::readBuffer
    export function readBuffer(length: number): Buffer {
        return null
    }

    /**
     * Set the serial input and output to use pins instead of the USB connection.
     * @param tx the new transmission pin, eg: SerialPin.P0
     * @param rx the new reception pin, eg: SerialPin.P1
     * @param rate the new baud rate. eg: 115200
     */
    //% weight=10
    //% help=serial/redirect
    //% blockId=serial2_redirect block="serial2|redirect to|TX %tx|RX %rx|at baud rate %rate"
    //% blockExternalInputs=1
    //% tx.fieldEditor="gridpicker" tx.fieldOptions.columns=3
    //% tx.fieldOptions.tooltips="false"
    //% rx.fieldEditor="gridpicker" rx.fieldOptions.columns=3
    //% rx.fieldOptions.tooltips="false"
    //% blockGap=8 shim=serial2::redirect
    export function redirect(tx: SerialPin, rx: SerialPin, rate: BaudRate): void {
        return
    }

    /**
     * Set the baud rate of the serial port
     */
    //% weight=10
    //% blockId=serial2_setbaudrate block="serial2|set baud rate %rate"
    //% blockGap=8 inlineInputMode=inline
    //% help=serial/set-baud-rate
    //% group="Configuration" advanced=true shim=serial2::setBaudRate
    export function setBaudRate(rate: BaudRate): void {
        return
    }


    /**
     * Check whether the serial2 device is enabled or not
     */
    //% blockId=serial2_is_enabled block="serial2|is enabled"
    //% advanced=true  
    //% group="Configuration"
    //% shim=serial2::isEnabled
    export function isEnabled(): boolean {
        return true
    }

    /**
     * Enable or disable the serial2 device. 
     * If a transfer is currently in progress, the operation is rejected and this function returns false.
     * @param enabled enable or disable the device
     * @returns whether the operation was successful
     */
    //% blockId=serial2_set_enabled block="serial2|set enabled $enabled"
    //% advanced=true 
    //% group="Configuration"
    //% shim=serial2::setEnabled
    export function setEnabled(enabled: boolean): boolean {
        return true
    }

    /**
     * Direct the serial input and output to use the USB connection.
     */
    //% weight=9 help=serial/redirect-to-usb
    //% blockId=serial2_redirect_to_usb block="serial2|redirect to USB" shim=serial2::redirectToUSB
    export function redirectToUSB(): void {
        return
    }

    /**
     * Sets the size of the RX buffer in bytes
     * @param size length of the rx buffer in bytes, eg: 32
     */
    //% help=serial/set-rx-buffer-size
    //% blockId=serial2SetRxBufferSize block="serial set rx buffer size to $size"
    //% advanced=true shim=serial2::setRxBufferSize
    export function setRxBufferSize(size: number): void {
        return
    }

    /**
     * Sets the size of the TX buffer in bytes
     * @param size length of the tx buffer in bytes, eg: 32
     */
    //% help=serial/set-tx-buffer-size
    //% blockId=serial2SetTxBufferSize block="serial set tx buffer size to $size"
    //% advanced=true shim=serial2::setTxBufferSize
    export function setTxBufferSize(size: number): void {
        return
    }


}
