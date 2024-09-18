#ifndef CFTDISPI_H
#define CFTDISPI_H

#include "FTDIhandler.h"

#define SPI_MOSI_PIN              FTDI_PIN_TX
#define SPI_MISO_PIN              FTDI_PIN_RX
#define SPI_SCK_PIN               FTDI_PIN_RTS
#define RESET_PIN                 FTDI_PIN_CTS

#define SPI_OUTPUT_PINS           (SPI_MOSI_PIN | SPI_SCK_PIN | RESET_PIN)

enum SPI_PIN_STATES
{
    PIN_ON  = true,
    PIN_OFF = false
};

class CFTDIspi : public CFTDIhandler
{
    Q_OBJECT

public:

    CFTDIspi(char *pFTDIserial_number);

    FT_STATUS setPin(unsigned char pin_mask, SPI_PIN_STATES state);
    FT_STATUS sendData(QByteArray data);

    QByteArray getData(void) {return m_bReadData;}

private:

    QByteArray                    m_bReadData;

};

#endif // CFTDISPI_H
