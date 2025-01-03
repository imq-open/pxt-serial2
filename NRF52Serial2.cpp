#include "./NRF52Serial2.h"
#include "peripheral_alloc.h"
#include "NotifyEvents.h"
#include "CodalDmesg.h"

using namespace codal;

extern int8_t target_get_irq_disabled();

// Same as uBit.serial
#define _DEFAULT_IRQ_PRIORITY 2

namespace imqopen
{

    /**
     * Constructor
     *
     * @param tx the pin instance to use for transmission
     *
     * @param rx the pin instance to use for reception
     *
     **/
    NRF52Serial2::NRF52Serial2(Pin &tx, Pin &rx, uint16_t id, NRF_UARTE_Type *device)
        : Serial(tx, rx, CODAL_SERIAL_DEFAULT_BUFFER_SIZE, CODAL_SERIAL_DEFAULT_BUFFER_SIZE, id),
          is_tx_in_progress_(false), bytesProcessed(0), p_uarte_(NULL)
    {
        if (device != NULL)
            p_uarte_ = (NRF_UARTE_Type *)allocate_peripheral((void *)device);
        else
            p_uarte_ = (NRF_UARTE_Type *)allocate_peripheral(PERI_MODE_UARTE);

        if (p_uarte_ == NULL)
            target_panic(DEVICE_HARDWARE_CONFIGURATION_ERROR);

        nrf_uarte_config_t hal_config;
        hal_config.hwfc = NRF_UARTE_HWFC_DISABLED;
        hal_config.parity = NRF_UARTE_PARITY_EXCLUDED;
#if defined(UARTE_CONFIG_STOP_Msk)
        hal_config.stop = NRF_UARTE_STOP_ONE;
#endif
#if defined(UARTE_CONFIG_PARITYTYPE_Msk)
        hal_config.paritytype = NRF_UARTE_PARITYTYPE_EVEN;
#endif

        nrf_uarte_baudrate_set(p_uarte_, NRF_UARTE_BAUDRATE_115200);
        nrf_uarte_configure(p_uarte_, &hal_config);

        // To be compatible with Serial.redirect()
        rx.setPull(PullMode::Up);

        configurePins(tx, rx);

        nrf_uarte_event_clear(p_uarte_, NRF_UARTE_EVENT_RXDRDY);
        nrf_uarte_event_clear(p_uarte_, NRF_UARTE_EVENT_ENDRX);
        nrf_uarte_event_clear(p_uarte_, NRF_UARTE_EVENT_ENDTX);
        nrf_uarte_event_clear(p_uarte_, NRF_UARTE_EVENT_ERROR);
        nrf_uarte_event_clear(p_uarte_, NRF_UARTE_EVENT_RXTO);
        nrf_uarte_event_clear(p_uarte_, NRF_UARTE_EVENT_TXSTOPPED);
        nrf_uarte_event_clear(p_uarte_, NRF_UARTE_EVENT_RXSTARTED);
        nrf_uarte_shorts_enable(p_uarte_, NRF_UARTE_SHORT_ENDRX_STARTRX);

        nrf_uarte_int_enable(p_uarte_, NRF_UARTE_INT_RXDRDY_MASK |
                                           NRF_UARTE_INT_RXSTARTED_MASK |
                                           NRF_UARTE_INT_ENDRX_MASK |
                                           NRF_UARTE_INT_ENDTX_MASK |
                                           NRF_UARTE_INT_ERROR_MASK |
                                           NRF_UARTE_INT_RXTO_MASK |
                                           NRF_UARTE_INT_TXSTOPPED_MASK);

        set_alloc_peri_irq(p_uarte_, &_irqHandler, this);

        IRQn_Type IRQn = get_alloc_peri_irqn(p_uarte_);

        NVIC_SetPriority(IRQn, _DEFAULT_IRQ_PRIORITY);
        NVIC_ClearPendingIRQ(IRQn);
        NVIC_EnableIRQ(IRQn);

        nrf_uarte_enable(p_uarte_);
    }

    NRF52Serial2::~NRF52Serial2()
    {
        nrf_uarte_int_disable(p_uarte_, NRF_UARTE_INT_RXDRDY_MASK |
                                            NRF_UARTE_INT_ENDRX_MASK |
                                            NRF_UARTE_INT_ENDTX_MASK |
                                            NRF_UARTE_INT_ERROR_MASK |
                                            NRF_UARTE_INT_RXTO_MASK |
                                            NRF_UARTE_INT_TXSTOPPED_MASK);
        NVIC_DisableIRQ(get_alloc_peri_irqn(p_uarte_));

        // Make sure all transfers are finished before UARTE is disabled
        // to achieve the lowest power consumption.
        nrf_uarte_shorts_disable(p_uarte_, NRF_UARTE_SHORT_ENDRX_STARTRX);
        nrf_uarte_task_trigger(p_uarte_, NRF_UARTE_TASK_STOPRX);
        nrf_uarte_event_clear(p_uarte_, NRF_UARTE_EVENT_TXSTOPPED);
        nrf_uarte_task_trigger(p_uarte_, NRF_UARTE_TASK_STOPTX);
        while (!nrf_uarte_event_check(p_uarte_, NRF_UARTE_EVENT_TXSTOPPED))
        {
        }

        nrf_uarte_disable(p_uarte_);
        nrf_uarte_txrx_pins_disconnect(p_uarte_);

        free_alloc_peri(p_uarte_);
    }

    void NRF52Serial2::_irqHandler(void *self_)
    {
        NRF52Serial2 *self = (NRF52Serial2 *)self_;
        NRF_UARTE_Type *p_uarte = self->p_uarte_;

        while (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_RXDRDY) && self->bytesProcessed < CONFIG_SERIAL_DMA_BUFFER_SIZE)
        {
            nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_RXDRDY);
            self->dataReceivedDMA();
        }

        if (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_ENDRX))
        {
            nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_ENDRX);
            self->updateRxBufferAfterENDRX();
        }

        if (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_RXSTARTED))
        {
            nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_RXSTARTED);
            self->updateRxBufferAfterRXSTARTED();
        }

        if (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_ERROR))
        {
            nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_ERROR);
            uint32_t src = nrf_uarte_errorsrc_get_and_clear(p_uarte);
            self->errorDetected(src);
        }

        if (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_RXTO))
        {
            nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_RXTO);
        }

        if (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_ENDTX))
        {
            nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_ENDTX);

            self->is_tx_in_progress_ = false;
            if (self->txBufferedSize() > 0)
            {
                self->dataTransmitted();
            }
            else
            {
                // Transmitter has to be stopped by triggering STOPTX task to achieve
                // the lowest possible level of the UARTE power consumption.
                nrf_uarte_task_trigger(p_uarte, NRF_UARTE_TASK_STOPTX);
            }
        }

        if (nrf_uarte_event_check(p_uarte, NRF_UARTE_EVENT_TXSTOPPED))
        {
            nrf_uarte_event_clear(p_uarte, NRF_UARTE_EVENT_TXSTOPPED);
            self->is_tx_in_progress_ = false;
        }
    }

    int NRF52Serial2::enableInterrupt(SerialInterruptType t)
    {
        if (t == RxInterrupt)
        {
            if (!(status & CODAL_SERIAL_STATUS_RX_BUFF_INIT))
                initialiseRx();

            if (status & CODAL_SERIAL_STATUS_RX_BUFF_INIT)
            {
                nrf_uarte_rx_buffer_set(p_uarte_, dmaBuffer, CONFIG_SERIAL_DMA_BUFFER_SIZE);
                bytesProcessed = 0;
                nrf_uarte_int_enable(p_uarte_, NRF_UARTE_INT_ERROR_MASK |
                                                   NRF_UARTE_INT_ENDRX_MASK);
                nrf_uarte_task_trigger(p_uarte_, NRF_UARTE_TASK_STARTRX);
            }
        }
        else if (t == TxInterrupt)
        {
            if (!is_tx_in_progress_ && txBufferedSize())
            {
                // To prevent the same data from being sent by the TX_DONE event
                // of the UARTE interrupt before processing the ring buffer.
                // Only the order in the Serial.dataTransmitted() function is different.
                uint16_t pre_txBuffTail = txBuffTail;
                txBuffTail = (txBuffTail + 1) % txBuffSize;
                putc((char)txBuff[pre_txBuffTail]);
                if (txBuffTail == txBuffHead)
                {
                    Event(DEVICE_ID_NOTIFY, CODAL_SERIAL_EVT_TX_EMPTY);
                }
            }
        }

        return DEVICE_OK;
    }

    int NRF52Serial2::disableInterrupt(SerialInterruptType t)
    {
        if (t == RxInterrupt)
        {
            nrf_uarte_int_disable(p_uarte_, NRF_UARTE_INT_ERROR_MASK |
                                                NRF_UARTE_INT_ENDRX_MASK);
        }
        else if (t == TxInterrupt)
        {
            // IDLE:
            // Since UARTE (DMA) is used, there is no need to turn off and turn off interrupts.
            // In addition, using a function that does not use the codal::Serial structure,
            // such as printf and putc, causes problems,
            // so it is not right to turn on and off the driver interrupts in this function.
            // NRF52Serial2::configurePins assumes the interrupt is not disabled
        }

        return DEVICE_OK;
    }

    int NRF52Serial2::setBaudrate(uint32_t baudrate)
    {
        nrf_uarte_baudrate_t baud = NRF_UARTE_BAUDRATE_115200;

        switch (baudrate)
        {
        case 1200:
            baud = NRF_UARTE_BAUDRATE_1200;
            break;
        case 2400:
            baud = NRF_UARTE_BAUDRATE_2400;
            break;
        case 4800:
            baud = NRF_UARTE_BAUDRATE_4800;
            break;
        case 9600:
            baud = NRF_UARTE_BAUDRATE_9600;
            break;
        case 31250:
            baud = NRF_UARTE_BAUDRATE_31250;
            break;
        case 38400:
            baud = NRF_UARTE_BAUDRATE_38400;
            break;
        case 57600:
            baud = NRF_UARTE_BAUDRATE_57600;
            break;
        case 115200:
            baud = NRF_UARTE_BAUDRATE_115200;
            break;
        case 230400:
            baud = NRF_UARTE_BAUDRATE_230400;
            break;
        case 921600:
            baud = NRF_UARTE_BAUDRATE_921600;
            break;
        case 1000000:
            baud = NRF_UARTE_BAUDRATE_1000000;
            break;
        }

        nrf_uarte_baudrate_set(p_uarte_, baud);
        return DEVICE_OK;
    }

    int NRF52Serial2::configurePins(Pin &tx, Pin &rx)
    {
        // Serial::redirect surrounds its call to this function with
        // disableInterrupt(TxInterrupt) and enableInterrupt(TxInterrupt)
        // but NRF52Serial2's implementation of those doesn't change the interrupt.
        // When we get here tx is locked, but the tx interrupt is still working to empty the buffer
        while (txBufferedSize() > 0 || is_tx_in_progress_) /*wait*/
            ;

        nrf_uarte_txrx_pins_set(p_uarte_, tx.name, rx.name);

        return DEVICE_OK;
    }

    int NRF52Serial2::putc(char c)
    {
        int res = DEVICE_OK;

        while (!target_get_irq_disabled() && is_tx_in_progress_)
            ;

        if (target_get_irq_disabled())
        {
            nrf_uarte_event_clear(p_uarte_, NRF_UARTE_EVENT_ENDTX);
            nrf_uarte_event_clear(p_uarte_, NRF_UARTE_EVENT_TXSTOPPED);
        }
        is_tx_in_progress_ = true;
        nrf_uarte_tx_buffer_set(p_uarte_, (const uint8_t *)&c, 1);
        nrf_uarte_task_trigger(p_uarte_, NRF_UARTE_TASK_STARTTX);

        // Block for when not using Interrupt. (like codal::Serial::prtinf())
        if (target_get_irq_disabled())
        {
            bool endtx;
            bool txstopped;
            do
            {
                endtx = nrf_uarte_event_check(p_uarte_, NRF_UARTE_EVENT_ENDTX);
                txstopped = nrf_uarte_event_check(p_uarte_, NRF_UARTE_EVENT_TXSTOPPED);
            } while ((!endtx) && (!txstopped));
            if (txstopped)
            {
                res = DEVICE_INVALID_STATE;
            }
            else
            {
                // Transmitter has to be stopped by triggering the STOPTX task to achieve
                // the lowest possible level of the UARTE power consumption.
                nrf_uarte_task_trigger(p_uarte_, NRF_UARTE_TASK_STOPTX);
                while (!nrf_uarte_event_check(p_uarte_, NRF_UARTE_EVENT_TXSTOPPED))
                {
                }
            }
            is_tx_in_progress_ = false;
        }

        return res;
    }

    int NRF52Serial2::getc()
    {
        return this->getChar(codal::SerialMode::ASYNC);
    }

    void NRF52Serial2::errorDetected(uint32_t src)
    {
        if (src & NRF_UARTE_ERROR_OVERRUN_MASK)
        {
            Event(this->id, IMQOPEN_NRF52SERIAL2_EVT_ERROR_OVERRUN);
        }
        // if (src & NRF_UARTE_ERROR_PARITY_MASK)
        // {
        //     Event(this->id, IMQOPEN_NRF52SERIAL2_EVT_ERROR_PARITY);
        // }
        if (src & NRF_UARTE_ERROR_FRAMING_MASK)
        {
            Event(this->id, IMQOPEN_NRF52SERIAL2_EVT_ERROR_FRAMING);
        }
        if (src & NRF_UARTE_ERROR_BREAK_MASK)
        {
            Event(this->id, IMQOPEN_NRF52SERIAL2_EVT_ERROR_BREAK);
        }
    }

    void NRF52Serial2::dataReceivedDMA()
    {
        dataReceived(dmaBuffer[bytesProcessed++]);
    }

    void NRF52Serial2::updateRxBufferAfterENDRX()
    {
        // A DMA transfer has been completed.
        // Determine the number of bytes the UART hardware sucessfuly transferred.
        // This is normally the size of the DMA buffer, but may be shorter if any exceptions occurred.
        int rxBytes = nrf_uarte_rx_amount_get(p_uarte_);

        // Occasionally we may detect an ENDRX event before the RXRDY associated with the last byte in the DMA buffer..
        // Clear the RXRDY event if we still have outstanding bytes to process - this protects against us accedentally
        // processing the same byte twice.
        if (bytesProcessed < rxBytes)
            nrf_uarte_event_clear(p_uarte_, NRF_UARTE_EVENT_RXDRDY);

        // Flush any unprocessed bytes in the DMA buffer.
        while (bytesProcessed < rxBytes)
            dataReceivedDMA();

        // Reset received byte counter, as we have completed processing the last DMA buffer
        // and will have started receiving into a new DMA buffer.
        bytesProcessed = 0;
    }

    void NRF52Serial2::updateRxBufferAfterRXSTARTED()
    {
        nrf_uarte_rx_buffer_set(p_uarte_, dmaBuffer, CONFIG_SERIAL_DMA_BUFFER_SIZE);
    }

    /**
     * Puts the component in (or out of) sleep (low power) mode.
     *
     * If CODAL_SERIAL_STATUS_DEEPSLEEP is set, then the peripheral will remain active during deep
     * sleep, and will wake the processor if new data is received.
     */
    int NRF52Serial2::setSleep(bool doSleep)
    {
        IRQn_Type IRQn = get_alloc_peri_irqn(p_uarte_);

        if (doSleep && !(status & CODAL_SERIAL_STATUS_DEEPSLEEP))
        {
            // Disable the RX interrupt and IRQ. Clear the buffers
            disableInterrupt(RxInterrupt);

            // wait...
            while (txBufferedSize() > 0 || is_tx_in_progress_)
                ;

            NVIC_DisableIRQ(IRQn);
        }
        else
        {
            // Reconnect IRQs and re-set settings
            NVIC_ClearPendingIRQ(IRQn);
            NVIC_EnableIRQ(IRQn);

            enableInterrupt(RxInterrupt);

            if (txBufferedSize() > 0)
                enableInterrupt(TxInterrupt);

            this->setBaud(this->baudrate);
        }

        return DEVICE_OK;
    }

    bool NRF52Serial2::isEnabled()
    {
        return !!p_uarte_->ENABLE;
    }

    int NRF52Serial2::setEnabled(bool enabled)
    {

        if (enabled == isEnabled())
        {
            return DEVICE_OK;
        }

        if (!enabled)
        {
            if (txInUse() || rxInUse())
                return DEVICE_SERIAL_IN_USE;

            lockTx();
            lockRx();

            if (txBufferedSize() > 0)
                disableInterrupt(TxInterrupt);

            disableInterrupt(RxInterrupt);

            // When we get here tx is locked, but the tx interrupt is still working to empty the buffer
            while (txBufferedSize() > 0 || is_tx_in_progress_) /*wait*/
                ;

            nrf_uarte_disable(p_uarte_);
        }
        else
        {
            nrf_uarte_enable(p_uarte_);

            enableInterrupt(RxInterrupt);

            if (txBufferedSize() > 0)
                enableInterrupt(TxInterrupt);

            setBaud(baudrate);

            unlockRx();
            unlockTx();
        }

        return DEVICE_OK;
    }

} // namespace imqopen
