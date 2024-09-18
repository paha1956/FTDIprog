#include <QThread>

#include "FTDIspi.h"

CFTDIspi::CFTDIspi(char *pFTDIserial_number) : CFTDIhandler(pFTDIserial_number, SPI_OUTPUT_PINS)
{
}

FT_STATUS CFTDIspi::setPin(unsigned char pin_mask, SPI_PIN_STATES state)
{
    QList<FTDI_OUTPUT_PIN_STATES> OutData;
    QByteArray in_pins_data;

    OutData.append({pin_mask, (bool)state});
    setOutput(OutData, in_pins_data);

    return getStatus();
}

FT_STATUS CFTDIspi::sendData(QByteArray out_data)
{
    m_bReadData.clear();

    QList<FTDI_OUTPUT_PIN_STATES> OutPinsData;
    QByteArray InputPinsData;

    OutPinsData.append({SPI_SCK_PIN, false});

    for (int iCurrentByte = 0; iCurrentByte < out_data.size(); iCurrentByte++)
    {
        for (int iCurrentBit = 0x80; iCurrentBit > 0; iCurrentBit >>= 1)
        {
            OutPinsData.append({SPI_MOSI_PIN, (out_data.at(iCurrentByte) & iCurrentBit) != 0});
            OutPinsData.append({SPI_SCK_PIN, true});
            OutPinsData.append({SPI_SCK_PIN, false});
        }
    }

    setOutput(OutPinsData, InputPinsData);

    unsigned short bit_mask = 0x80;
    char data = 0;
    foreach (char input_state, InputPinsData)
    {
        if (input_state & SPI_SCK_PIN)
        {
            if (input_state & SPI_MISO_PIN)
            {
                data |= bit_mask;
            }

            bit_mask >>= 1;
            if (bit_mask == 0x00)
            {
                bit_mask = 0x80;
                m_bReadData.append(data);
                data = 0;
            }
        }
    }

    return getStatus();
}
