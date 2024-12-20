
# pxt-serial2

MakeCode extension for BBC Micro:bit v2 which provides a second serial component 
(`serial2`) in addition to the built-in one.

## Use as Extension

This repository can be added as an **extension** in MakeCode.

* open [https://makecode.microbit.org/](https://makecode.microbit.org/)
* click on **New Project**
* click on **Extensions** under the gearwheel menu
* search for **https://github.com/imq-open/pxt-serial2** and import

## API

Blocks API is in the “Serial2” group. 

The TypeScript API is exposed through the `serial2` namespace, and the functions and usage 
are basically the same as for `serial`. The differences are described here and in sections below.

- Default pins. The default pins are P13 (TX) and P14 (RX).
- Pull-up on RX. To be compatible with Micro:bit V1 as well as `serial.redirect()`, the
 internal pull-up resistor of RX pin is enabled.
- Supports baud rate below 9600: 1200, 2400, 4800

### Device ID and Events

The device ID of serial2 is `SERIAL2_DEVICE_ID` (`70`).

The following events are provided:

| Event | Value |   &nbsp;
------|-------| ----
`SERIAL2_EVT_DELIM_MATCH` | `CODAL_SERIAL_EVT_DELIM_MATCH` (`1`) | 
`SERIAL2_EVT_HEAD_MATCH` | `CODAL_SERIAL_EVT_DELIM_MATCH` (`2`) | 
`SERIAL2_EVT_RX_FULL` | `CODAL_SERIAL_EVT_DELIM_MATCH` (`3`) | 
`SERIAL2_EVT_DATA_RECEIVED` | `CODAL_SERIAL_EVT_DELIM_MATCH` (`4`) | 
`SERIAL2_EVT_ERROR_OVERRUN` | `10` |  Fired when an overrun error occurs
`SERIAL2_EVT_ERROR_FRAMING` | `12` | Fired when a frame error occurs
`SERIAL2_EVT_ERROR_BREAK` | `13` | Fired when a break condition occurs

The device ID and events may be used with `control.onEvent()`. For example

```TypeScript
control.onEvent(EventBusSource.SERIAL2_DEVICE_ID, EventBusValue.SERIAL2_EVT_ERROR_FRAMING, function () {
    serial2.writeString('!!frame error!!\n')
})
```

### Enable and Disable

The serial2 device can be enabled or disabled.

```TypeScript
serial2.setEnabled(true);
serial2.isEnabled();
```


## License

MIT.

See also [the Third Party Notices](./THIRD_PARTY_NOTICES.md).

## Metadata (used for search, rendering)

* for PXT/microbit
(Micro:bit V2 only)
