#ifndef CFTDIHANDLER_H
#define CFTDIHANDLER_H

#include <QObject>

#include "ftd2xx.h"

#define FTDI_RECEIVING_BUFFER_SIZE    4096
#define FTDI_TRANSMITTING_BUFFER_SIZE 4096

#define FTDI_READ_TIMEOUT         5000
#define FTDI_WRITE_TIMEOUT        5000

#define FTDI_LATENCY              8

#define FTDI_CHAR_Xon             0x11
#define FTDI_CHAR_Xoff            0x13

enum FTDI_PINs
{
    FTDI_PIN_TX  = 0x01,          // Оранжевый провод FTDI кабеля
    FTDI_PIN_RX  = 0x02,          // Жёлтый провод FTDI кабеля
    FTDI_PIN_RTS = 0x04,          // Зелёный провод FTDI кабеля
    FTDI_PIN_CTS = 0x08,          // Коричневый провод FTDI кабеля
    FTDI_PIN_DTR = 0x10,
    FTDI_PIN_DSR = 0x20,
    FTDI_PIN_DCD = 0x40,
    FTDI_PIN_RI  = 0x80,
    FTDI_PIN_ALL = 0xff
};

struct FTDI_OUTPUT_PIN_STATES
{
    unsigned char                 pin_mask;
    bool                          state;
};

class CFTDIhandler: public QObject
{
    Q_OBJECT

private:

    FT_HANDLE                     m_ftHandle;
    FT_STATUS                     m_ftStatus;

    unsigned char                 m_ucOutputMask;
    unsigned char                 m_ucOutputState;

public:

    CFTDIhandler(char *pFTDIserial_number, unsigned char pinout_mask);
    ~CFTDIhandler(void);

    FT_STATUS getStatus(void) {return m_ftStatus;}

    FT_STATUS setOutput(QList<FTDI_OUTPUT_PIN_STATES> out_pin_states, QByteArray &in_pin_states);

};

#endif // CFTDIHANDLER_H
