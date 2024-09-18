#include "FTDIhandler.h"

CFTDIhandler::CFTDIhandler(char *pFTDIserial_number, unsigned char pinout_mask)
{
    m_ucOutputMask = pinout_mask;

    m_ftStatus = FT_OpenEx((PVOID*)pFTDIserial_number, FT_OPEN_BY_SERIAL_NUMBER, &m_ftHandle);                // Открытие устройства по его серийному номеру
    m_ftStatus |= FT_SetUSBParameters(m_ftHandle, FTDI_RECEIVING_BUFFER_SIZE, FTDI_TRANSMITTING_BUFFER_SIZE); // Установка объёма буферов приёма/передачи
    m_ftStatus |= FT_SetBaudRate(m_ftHandle, FT_BAUD_115200);                                                 // Установка скорости передачи
    m_ftStatus |= FT_SetChars(m_ftHandle, false, 0, false, 0);                                                // Запрет событий
    m_ftStatus |= FT_SetTimeouts(m_ftHandle, FTDI_READ_TIMEOUT, FTDI_WRITE_TIMEOUT);                          // Установка таймаута записи/чтения, мс
    m_ftStatus |= FT_SetLatencyTimer(m_ftHandle, FTDI_LATENCY);                                               // Установка задержки между передачей массивов данных, мс
    m_ftStatus |= FT_SetFlowControl(m_ftHandle, FT_FLOW_NONE, FTDI_CHAR_Xon, FTDI_CHAR_Xoff);                 // Отключение режима контроля границ сообщения (символы Xon(0x11)/Xoff(0x13))
    m_ftStatus |= FT_SetBitMode(m_ftHandle, pinout_mask, FT_BITMODE_RESET);                                   // Сброс режима
    m_ftStatus |= FT_SetBitMode(m_ftHandle, pinout_mask, FT_BITMODE_SYNC_BITBANG);                            // Установка режима "Синхронный BitBang", когда чтение из порта
                                                                                                              // производится непосредственно перед записью данных,
                                                                                                              //т.е. синхронизировано с записью
    if (m_ftStatus != FT_OK) return;

    QList<FTDI_OUTPUT_PIN_STATES> pin_states;
    QByteArray in_pins_data;
    pin_states.append({pinout_mask, false});
    setOutput(pin_states, in_pins_data);
}

CFTDIhandler::~CFTDIhandler(void)
{
    FT_Close(m_ftHandle);
}

FT_STATUS CFTDIhandler::setOutput(QList<FTDI_OUTPUT_PIN_STATES> out_pin_states, QByteArray &in_pin_states)
{
    DWORD lpBytesWritten;
    DWORD lpBytesReceived;

    QByteArray baOutputState(out_pin_states.size(), 0);

    int byte_index = 0;
    foreach (FTDI_OUTPUT_PIN_STATES out_pin_state, out_pin_states)
    {
        if (out_pin_state.state) m_ucOutputState |= out_pin_state.pin_mask;
        else                     m_ucOutputState &= ~out_pin_state.pin_mask;

        m_ucOutputState &= m_ucOutputMask;

        baOutputState[byte_index++] = m_ucOutputState;
    }

    // Запись данных из выходного буфера в FTDI
    // Интерфейс находится в режиме синхронного BitBang'а. Передаче каждого байта предшествует чтение входных сигналов
    m_ftStatus = FT_Write(m_ftHandle, (LPVOID *)baOutputState.constData(), baOutputState.size(), &lpBytesWritten);

    in_pin_states.clear();
    in_pin_states.resize(out_pin_states.size() + 1);
    DWORD RxBytes;
    do
    {
        FT_GetQueueStatus(m_ftHandle, &RxBytes);
    } while (RxBytes != (DWORD)baOutputState.size());   // Ожидание приёма требуемого количесва байт

    // Чтение и запись в буфер приёма данных, полученных от FTDI
    if (RxBytes > 0)
    {
        m_ftStatus |= FT_Read(m_ftHandle, (LPVOID *)in_pin_states.constData(), RxBytes, &lpBytesReceived);
    }

    return m_ftStatus;
}
