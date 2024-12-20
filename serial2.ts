
/**
 * Reading and writing data over a serial connection.
 */
//% weight=2 color=#0fbc11 icon="\uf287"
//% advanced=true
namespace serial2 {

    /**
     * The string used to mark a new line, default is \r\n
     */
    export let NEW_LINE = "\r\n";
    export let NEW_LINE_DELIMITER: Delimiters = Delimiters.NewLine;
    let writeLinePadding = 32;

    /**
    * Print a line of text to the serial port
    * @param value to send over serial
    */
    //% weight=90
    //% help=serial/write-line blockGap=8
    //% blockId=serial2_writeline block="serial2|write line %text"
    //% text.shadowOptions.toString=true
    export function writeLine(text: string): void {
        if (!text) text = "";
        serial2.writeString(text);
        // pad data to the 32 byte boundary
        // to ensure apps receive the packet
        if (writeLinePadding > 0) {
            let r = (writeLinePadding - (text.length + NEW_LINE.length) % writeLinePadding) % writeLinePadding;
            for (let i = 0; i < r; ++i)
                serial2.writeString(" ");
        }
        serial2.writeString(NEW_LINE);
    }

    /**
     * Sets the padding length for lines sent with "write line".
     * @param length the number of bytes alignment, eg: 0
     *
     */
    //% weight=1
    //% help=serial/set-write-line-padding
    //% blockId=serial2WriteNewLinePadding block="serial2 set write line padding to $length"
    //% advanced=true
    //% length.min=0 length.max=128
    export function setWriteLinePadding(length: number) {
        writeLinePadding = length | 0;
    }

    /**
     * Print a numeric value to the serial port
     */
    //% help=serial/write-number
    //% weight=89 blockGap=8
    //% blockId=serial2_writenumber block="serial2|write number %value"
    export function writeNumber(value: number): void {
        writeString(value.toString());
    }

    /**
     * Print an array of numeric values as CSV to the serial port
     */
    //% help=serial/write-numbers
    //% weight=86
    //% blockId=serial2_writenumbers block="serial2|write numbers %values"
    export function writeNumbers(values: number[]): void {
        if (!values) return;
        for (let i = 0; i < values.length; ++i) {
            if (i > 0) writeString(",");
            writeNumber(values[i]);
        }
        writeLine("")
    }

    /**
     * Write a name:value pair as a line to the serial port.
     * @param name name of the value stream, eg: x
     * @param value to write
     */
    //% weight=88 blockGap=8
    //% help=serial/write-value
    //% blockId=serial2_writevalue block="serial2|write value %name|= %value"
    export function writeValue(name: string, value: number): void {
        writeLine((name ? name + ":" : "") + value);
    }

    /**
     * Read a line of text from the serial port.
     */
    //% help=serial/read-line
    //% blockId=serial2_read_line block="serial2|read line"
    //% weight=20 blockGap=8
    export function readLine(): string {
        return serial2.readUntil(serial.delimiters(NEW_LINE_DELIMITER));
    }
}
