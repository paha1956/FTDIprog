#ifndef CATMEGASERIALPROG_H
#define CATMEGASERIALPROG_H

#include <QObject>
#include <QTimer>

#include "FTDIspi.h"

#define LOW_BYTE_MASK             0x00
#define HIGH_BYTE_MASK            0x08
#define EXTENDED_BYTE_MASK        0x04

#define INSTRUCTION_DATA_INDEX    3

enum SERIAL_PROGRAMMING_INSTRUCTIONS
{
    INSTR_PROGRAMMING_ENABLE        = 0xac53,
    INSTR_CHIP_ERASE                = 0xac80,

    // Инструкции загрузки
    INSTR_LOAD_PROGRAM_MEMORY_PAGE  = 0x4000,

    // Инструкции чтения
    INSTR_READ_PROGRAM_MEMORY       = 0x2000,
    INSTR_READ_EEPROM_MEMORY        = 0xa000,
    INSTR_READ_SIGNATURE_BYTE       = 0x3000,
    INSTR_READ_LOW_FUSE_BITS        = 0x5000,
    INSTR_READ_HIGH_FUSE_BITS       = 0x5808,
    INSTR_READ_EXT_FUSE_BITS        = 0x5008,

    // Инструкции записи
    INSTR_WRITE_PROGRAM_MEMORY_PAGE = 0x4c00,
    INSTR_WRITE_EEPROM_MEMORY       = 0xc000,
    INSTR_WRITE_LOCK_BITS           = 0xace0,
    INSTR_WRITE_FUSE_BITS           = 0xaca0

};

#define ATMEGA_SIGNATURE_DATA_SIZE           3
#define ATMEGA48_88_168_SIGNATURE_FIRST_BYTE 0x1e

#define ATMEGA48_SIGNATURE_SECOND_BYTE       ((char)0x92)
#define ATMEGA88_SIGNATURE_SECOND_BYTE       ((char)0x93)
#define ATMEGA168_SIGNATURE_SECOND_BYTE      ((char)0x94)

#define ATMEGA48_SIGNATURE_THIRD_BYTE        0x05
#define ATMEGA88_SIGNATURE_THIRD_BYTE        0x0a
#define ATMEGA168_SIGNATURE_THIRD_BYTE       0x06

#define ATMEGA48_FLASH_SIZE       ( 4 << 10)                          // Объём FLAH памяти, байт
#define ATMEGA88_FLASH_SIZE       ( 8 << 10)
#define ATMEGA168_FLASH_SIZE      (16 << 10)
#define ATMEGA_DEFAULT_FLASH_SIZE (16 << 10)

#define ATMEGA48_PAGE_SIZE        64                                  // Объём страницы FLAH памяти, байт
#define ATMEGA88_PAGE_SIZE        64
#define ATMEGA168_PAGE_SIZE       128
#define ATMEGA_DEFAULT_PAGE_SIZE  128

#define ATMEGA48_EEPROM_SIZE      256                                 // Объём EEPROM памяти, байт
#define ATMEGA88_EEPROM_SIZE      512
#define ATMEGA168_EEPROM_SIZE     512
#define ATMEGA_DEFAULT_EEPROM_SIZE 512

enum SPI_EVENTS
{
    SPI_OK     = 0,
    SPI_ERROR  = -1,
    SPI_NOFTDI = -2
};

enum FUSE_INDEX
{
    LOW_FUSE_BYTE_INDEX = 0,
    HI_FUSE_BYTE_INDEX  = 1,
    EXT_FUSE_BYTE_INDEX = 2,
    FUSE_BYTES_NUMBER
};

struct DEVICE_INFO
{
    int                           status;
    QString                       name;
    QByteArray                    signature;
    int                           flash_size;
    int                           page_size;
    int                           eeprom_size;
};

class CATmegaSerialProg : public QObject
{

    Q_OBJECT

public:

    CATmegaSerialProg(QObject *parent = nullptr);
    ~CATmegaSerialProg();

    struct DEVICE_INFO getDeviceInfo(void) {return m_DeviceInfo;}

    enum SPI_EVENTS Init(void);
    void Deinit(void);

    struct DEVICE_INFO ReadDeviceInfo(void);

    enum SPI_EVENTS ChipErase(void);
    enum SPI_EVENTS WriteFLASH(unsigned short address, QByteArray *p_data);
    enum SPI_EVENTS ReadFLASH(unsigned short address, QByteArray *p_data, unsigned short size);
    enum SPI_EVENTS WriteEEPROM(unsigned short address, QByteArray *p_data);
    enum SPI_EVENTS ReadEEPROM(unsigned short address, QByteArray *p_data, unsigned short size);
    enum SPI_EVENTS WriteFuseBytes(QByteArray fuses);
    enum SPI_EVENTS ReadFuseBytes(QByteArray &fuses);
    enum SPI_EVENTS WriteLockBits(unsigned char lock_bits);

    void OperationCancel(void);

private:

    CFTDIspi                     *m_pFTDIspi;

    struct DEVICE_INFO            m_DeviceInfo;

    QByteArray                   *m_pRawData;
    int                           m_iCurrentIndex;
    int                           m_iPageCurrentIndex;
    int                           m_iAddress;
    int                           m_iDataSize;

    QTimer                        m_tSendNextWriteFLASHCommandTimer;
    QTimer                        m_tSendNextReadFLASHCommandTimer;
    QTimer                        m_tSendNextWriteEEPROMCommandTimer;
    QTimer                        m_tSendNextReadEEPROMCommandTimer;


private slots:

    void SendNexWriteFLASHCommandTimerSlot(void);
    void SendNexReadFLASHCommandTimerSlot(void);
    void SendNexWriteEEPROMCommandTimerSlot(void);
    void SendNexReadEEPROMCommandTimerSlot(void);

signals:

    void spi_error_signal(void);
    void spi_alldata_programming_completed_signal(void);
    void spi_page_programming_completed_signal(int index);

};

#endif // CATMEGASERIALPROG_H
