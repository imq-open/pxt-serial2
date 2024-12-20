#ifndef IMQOPEN_NRF52SERIAL2_H
#define IMQOPEN_NRF52SERIAL2_H

#include "Pin.h"
#include "nrf.h"
#include "CodalComponent.h"
#include "CodalConfig.h"
#include "Serial.h"
#include "hal/nrf_uarte.h"

#ifndef CONFIG_SERIAL_DMA_BUFFER_SIZE
#define CONFIG_SERIAL_DMA_BUFFER_SIZE 32
#endif

// Suggested range for device-specific IDs: 50-79
#define IMQOPEN_NRF52SERIAL2_DEFAULT_DEVICE_ID 70

#define IMQOPEN_NRF52SERIAL2_EVT_DELIM_MATCH CODAL_SERIAL_EVT_DELIM_MATCH
#define IMQOPEN_NRF52SERIAL2_EVT_HEAD_MATCH CODAL_SERIAL_EVT_HEAD_MATCH
#define IMQOPEN_NRF52SERIAL2_EVT_RX_FULL CODAL_SERIAL_EVT_RX_FULL
#define IMQOPEN_NRF52SERIAL2_EVT_DATA_RECEIVED CODAL_SERIAL_EVT_DATA_RECEIVED
#define IMQOPEN_NRF52SERIAL2_EVT_ERROR_OVERRUN 10
// #define IMQOPEN_NRF52SERIAL2_EVT_ERROR_PARITY 11
#define IMQOPEN_NRF52SERIAL2_EVT_ERROR_FRAMING 12
#define IMQOPEN_NRF52SERIAL2_EVT_ERROR_BREAK 13

namespace imqopen
{

  using namespace codal;

  class NRF52Serial2 : public Serial
  {
    volatile bool is_tx_in_progress_;
    volatile int bytesProcessed;
    uint8_t dmaBuffer[CONFIG_SERIAL_DMA_BUFFER_SIZE];

    NRF_UARTE_Type *p_uarte_;
    static void _irqHandler(void *self);

    /**
     * Ensures all characters have been processed once a DMA buffer is fully received.
     *
     * This function is called upon an ENDRX hardware event, which is raised when a RX DMA buffer
     * has been filled. Normally there is nothing to do, but in the eventuality that an interrupt has been
     * missed (typically due to CPU contention), this function ensures the codal Serial ringbuffer is synchronised
     * and no characters are lost.
     */
    void updateRxBufferAfterENDRX();

    /**
     * Update DMA RX buffer pointer.
     *
     * UARTE generates an RXSTARTED event once the DMA buffer geometry has been read.
     * This function implements a DMA enabled double buffer within the codal Serial ringbuffer.
     **/
    void updateRxBufferAfterRXSTARTED();

    /**
     * DMA version of Serial::dataReceviced()
     *
     * In the A function, only the part of getting data through the getc() function
     * and storing it in the ring buffer is removed.
     * Because DMA stores data directly in the buffer,
     * this function only manages the ring buffer. (With Event function)
     **/
    void dataReceivedDMA();

    void errorDetected(uint32_t src);

  protected:
    virtual int enableInterrupt(SerialInterruptType t) override;
    virtual int disableInterrupt(SerialInterruptType t) override;
    virtual int configurePins(Pin &tx, Pin &rx) override;

  public:
    /**
     * Constructor
     *
     * @param tx the pin instance to use for transmission
     *
     * @param rx the pin instance to use for reception
     *
     **/
    NRF52Serial2(Pin &tx, Pin &rx, uint16_t id = IMQOPEN_NRF52SERIAL2_DEFAULT_DEVICE_ID, NRF_UARTE_Type *device = NULL);

    virtual int putc(char) override;
    virtual int getc() override;
    virtual int setBaudrate(uint32_t baudrate) override;

    /**
     * Puts the component in (or out of) sleep (low power) mode.
     */
    virtual int setSleep(bool doSleep) override;

    bool isEnabled();
    int setEnabled(bool enabled);

    ~NRF52Serial2();
  };
}

#endif // IMQOPEN_NRF52SERIAL2_H
