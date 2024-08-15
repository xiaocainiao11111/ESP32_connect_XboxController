# arduino-XboxSeriesXControllerESP32 (XboxSeriesXControllerESP32_asukiaaa)

A library to use xbox controller on ESP32.

## Known issue.

The following error occurs on ESP32.
```
lld_pdu_get_tx_flush_nb HCI packet count mismatch (1, 2)
```

It does not occur on ESP32C3 (may be also S3) because BLE5 may be needed to communicate with xbox controller.
If you want to use this library without the error, please use ESP32 which supports BLE5.

Related issues:
- [Connection is not the first time](https://github.com/asukiaaa/arduino-XboxSeriesXControllerESP32/issues/3)
- [BLE service discovery fails on low BLE connections](https://github.com/espressif/esp-idf/issues/8303)
- [Request of some advice to solve lld_pdu_get_tx_flush_nb HCI packet count mismatch (1, 2)](https://github.com/h2zero/NimBLE-Arduino/issues/293)
- [Reset scanning state without resetting esp32](https://github.com/h2zero/NimBLE-Arduino/issues/417)

## Update firmware of controller

This libary is checked with firmware version `5.13.3143.0` for xbox series X controller.
You can check and udpate firmware with using [Xbox accessories](https://www.microsoft.com/en-us/p/xbox-accessories/9nblggh30xj3#activetab=pivot:overviewtab).

## Setup

### Arduino IDE

Install XboxSeriesXControllerESP32_asukiaaa from library manager.

### PlatformIO

Specify XboxSeriesXControllerESP32_asukiaaa as lib_deps on platformio.ini

```
lib_deps = XboxSeriesXControllerESP32_asukiaaa
```

## Usage

See [examples](./examples).

## License

MIT

## References

- [NimBLE_Client](https://github.com/h2zero/NimBLE-Arduino/blob/master/examples/NimBLE_Client/NimBLE_Client.ino)
- [asukiaaa/arduino-XboxControllerNotificationParser](https://github.com/asukiaaa/arduino-XboxControllerNotificationParser)
- [asukiaaa/arduino-XboxSeriesXHIDReportBuilder](https://github.com/asukiaaa/arduino-XboxSeriesXHIDReportBuilder)
