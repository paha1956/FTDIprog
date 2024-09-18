#include <QThread>

#include "ATmegaSerialProg.h"

CATmegaSerialProg::CATmegaSerialProg(QObject *parent) : QObject(parent)
{
    m_pFTDIspi = nullptr;

    m_tSendNextWriteFLASHCommandTimer.setSingleShot(true);
    connect(&m_tSendNextWriteFLASHCommandTimer, &QTimer::timeout, this, &CATmegaSerialProg::SendNexWriteFLASHCommandTimerSlot);

    m_tSendNextReadFLASHCommandTimer.setSingleShot(true);
    connect(&m_tSendNextReadFLASHCommandTimer, &QTimer::timeout, this, &CATmegaSerialProg::SendNexReadFLASHCommandTimerSlot);

    m_tSendNextWriteEEPROMCommandTimer.setSingleShot(true);
    connect(&m_tSendNextWriteEEPROMCommandTimer, &QTimer::timeout, this, &CATmegaSerialProg::SendNexWriteEEPROMCommandTimerSlot);

    m_tSendNextReadEEPROMCommandTimer.setSingleShot(true);
    connect(&m_tSendNextReadEEPROMCommandTimer, &QTimer::timeout, this, &CATmegaSerialProg::SendNexReadEEPROMCommandTimerSlot);
}

CATmegaSerialProg::~CATmegaSerialProg()
{
}

enum SPI_EVENTS CATmegaSerialProg::Init(void)
{
    FT_STATUS ftStatus;                        // Объявляем переменную типа FT_STATUS. Содержит результат работы функции
    FT_DEVICE_LIST_INFO_NODE *devInfo;         // Объявляем указатель на массив структур
    DWORD numDevs;                             // Предварительно объявляем переменную, в которой будет храниться число
                                               // подключенных D2XX утройств
    // Создаем список устройств
    ftStatus = FT_CreateDeviceInfoList(&numDevs);
    if (ftStatus != FT_OK) {
        // Функция FT_CreateDeviceInfoList завершилась неудачей
        printf("Create Device List with error %d\n", ftStatus);
        return SPI_NOFTDI;
    }

    printf("Number of devices is %d\n",numDevs);             // Печатаем количество устройств

    // Если подключено хотя бы одно устройство
    if (numDevs > 0)
    {
      // Выделить память для хранения массива в зависимости от количества элементов
      devInfo = new FT_DEVICE_LIST_INFO_NODE[numDevs];
      // Получить информацию о каждом устройстве в массиве
      ftStatus = FT_GetDeviceInfoList(devInfo, &numDevs);
      if (ftStatus != FT_OK)
      {
          delete devInfo;
          printf("Get Device Info List with error %d\n", ftStatus);
          return SPI_NOFTDI;
      }

      for (DWORD i = 0; i < numDevs; i++)
      {
        printf("Dev %d:\n",i);                                  // Напечатать номер устройства, начиная с 0
        printf("  Flags=0x%x\n",devInfo[i].Flags);              // Напечатать все поля каждой структуры
        printf("  Type=0x%x\n",devInfo[i].Type);
        printf("  ID=0x%x\n",devInfo[i].ID);
        printf("  LocId=0x%x\n",devInfo[i].LocId);
        printf("  SerialNumber=%s\n",devInfo[i].SerialNumber);
        printf("  Description=%s\n",devInfo[i].Description);
        printf("  ftHandle=0x%x\n",devInfo[i].ftHandle);
      }

      m_pFTDIspi = new CFTDIspi(devInfo[0].SerialNumber);

      delete devInfo;
      if (m_pFTDIspi->getStatus() != FT_OK)
      {
          printf("Set Bit Mode with error %d\n", ftStatus);
          return SPI_ERROR;
      }
    }
    else
    {
        m_pFTDIspi = nullptr;
        return SPI_NOFTDI;
    }

    QByteArray outData;

    outData.resize(4);
    outData[0] = (char)((INSTR_PROGRAMMING_ENABLE >> 8) & 0xff);
    outData[1] = (char)((INSTR_PROGRAMMING_ENABLE     ) & 0xff);
    outData[2] = 0x00;
    outData[3] = 0x00;

    m_pFTDIspi->setPin(RESET_PIN, PIN_OFF);
    QThread::msleep(25);

    m_pFTDIspi->sendData(outData);

    QThread::msleep(50);

    if (m_pFTDIspi->getData().at(2) != (INSTR_PROGRAMMING_ENABLE & 0xff))
        return SPI_ERROR;

    return SPI_OK;
}

void CATmegaSerialProg::Deinit(void)
{
    if (m_pFTDIspi == nullptr) return;

    QThread::msleep(25);
    m_pFTDIspi->setPin(RESET_PIN, PIN_ON);

    delete m_pFTDIspi;
}

struct DEVICE_INFO CATmegaSerialProg::ReadDeviceInfo(void)
{
    m_DeviceInfo.status = SPI_ERROR;
    m_DeviceInfo.signature.resize(3);
    m_DeviceInfo.signature.fill(0);
    m_DeviceInfo.name = QString::fromLocal8Bit("не определено");
    m_DeviceInfo.flash_size  = ATMEGA_DEFAULT_FLASH_SIZE;
    m_DeviceInfo.page_size   = ATMEGA_DEFAULT_PAGE_SIZE;
    m_DeviceInfo.eeprom_size = ATMEGA_DEFAULT_EEPROM_SIZE;

    if (m_pFTDIspi == nullptr) return m_DeviceInfo;

    QByteArray outData;

    outData.resize(4);
    outData[0] = (char)((INSTR_READ_SIGNATURE_BYTE >> 8) & 0xff);
    outData[1] = (char)((INSTR_READ_SIGNATURE_BYTE     ) & 0xff);
    outData[3] = 0x00;

    for (int iSignatureByteIndex = 0; iSignatureByteIndex < ATMEGA_SIGNATURE_DATA_SIZE; iSignatureByteIndex++)
    {
        outData[2] = iSignatureByteIndex;
        if (m_pFTDIspi->sendData(outData) != FT_OK) return m_DeviceInfo;

        QThread::msleep(50);

        m_DeviceInfo.signature[iSignatureByteIndex] = m_pFTDIspi->getData().at(INSTRUCTION_DATA_INDEX);
    }

    m_DeviceInfo.status = SPI_OK;

    if (m_DeviceInfo.signature.at(0) == ATMEGA48_88_168_SIGNATURE_FIRST_BYTE)
    { if (m_DeviceInfo.signature.at(1) == ATMEGA48_SIGNATURE_SECOND_BYTE && m_DeviceInfo.signature.at(2) == ATMEGA48_SIGNATURE_THIRD_BYTE)
      { m_DeviceInfo.name        = "ATmega48";
        m_DeviceInfo.flash_size  = ATMEGA48_FLASH_SIZE;
        m_DeviceInfo.page_size   = ATMEGA48_PAGE_SIZE;
        m_DeviceInfo.eeprom_size = ATMEGA48_EEPROM_SIZE;
      }
      else
      { if (m_DeviceInfo.signature.at(1) == ATMEGA88_SIGNATURE_SECOND_BYTE && m_DeviceInfo.signature.at(2) == ATMEGA88_SIGNATURE_THIRD_BYTE)
        { m_DeviceInfo.name        = "ATmega88";
          m_DeviceInfo.flash_size  = ATMEGA88_FLASH_SIZE;
          m_DeviceInfo.page_size   = ATMEGA88_PAGE_SIZE;
          m_DeviceInfo.eeprom_size = ATMEGA88_EEPROM_SIZE;
        }
        else
        { if (m_DeviceInfo.signature.at(1) == ATMEGA168_SIGNATURE_SECOND_BYTE && m_DeviceInfo.signature.at(2) == ATMEGA168_SIGNATURE_THIRD_BYTE)
          { m_DeviceInfo.name        = "ATmega168";
            m_DeviceInfo.flash_size  = ATMEGA168_FLASH_SIZE;
            m_DeviceInfo.page_size   = ATMEGA168_PAGE_SIZE;
            m_DeviceInfo.eeprom_size = ATMEGA168_EEPROM_SIZE;
          }
        }
      }
    }

    return m_DeviceInfo;
}

enum SPI_EVENTS CATmegaSerialProg::ChipErase(void)
{
    if (m_pFTDIspi == nullptr) return SPI_ERROR;

    QByteArray outData;

    outData.resize(4);
    outData[0] = (char)((INSTR_CHIP_ERASE >> 8) & 0xff);
    outData[1] = (char)((INSTR_CHIP_ERASE     ) & 0xff);
    outData[2] = 0x00;
    outData[3] = 0x00;

    if (m_pFTDIspi->sendData(outData) != FT_OK) return SPI_ERROR;

    QThread::msleep(100);

    return SPI_OK;
}

enum SPI_EVENTS CATmegaSerialProg::WriteFLASH(unsigned short address, QByteArray *p_data)
{
    unsigned short d_address = address % m_DeviceInfo.page_size;
    if (d_address != 0)
    {
        address -= d_address;
        p_data->prepend(d_address, 0xff);
    }

    m_pRawData = p_data;
    m_iCurrentIndex = 0;
    m_iPageCurrentIndex = 0;
    m_iAddress = address >> 1;
    m_iDataSize = p_data->size();

    m_tSendNextWriteFLASHCommandTimer.start(1);

    return SPI_OK;
}

void CATmegaSerialProg::SendNexWriteFLASHCommandTimerSlot(void)
{
    if (m_pFTDIspi == nullptr) return;

    bool page_empty_flag = true;
    for (int data_index = 0; data_index < m_DeviceInfo.page_size; data_index++)
    {
        if (m_pRawData->at((m_iCurrentIndex << 1) + data_index) != (char)0xff)
        {
            page_empty_flag = false;
            break;
        }
    }

    if (page_empty_flag)
    {
        m_iCurrentIndex += m_DeviceInfo.page_size >> 1;
        spi_page_programming_completed_signal((m_iCurrentIndex << 1) * 100 / m_pRawData->size());
        m_tSendNextWriteFLASHCommandTimer.start(1);
        return;
    }

    QByteArray outData;
    outData.clear();
    do
    {
        outData.append(((INSTR_LOAD_PROGRAM_MEMORY_PAGE >> 8) | LOW_BYTE_MASK) & 0xff);
        outData.append( (INSTR_LOAD_PROGRAM_MEMORY_PAGE     )                  & 0xff);
        outData.append(m_iPageCurrentIndex);

        int raw_data_index = (m_iCurrentIndex + m_iPageCurrentIndex) << 1;
        char raw_data;
        if (raw_data_index >= m_pRawData->size()) raw_data = 0xff;
        else                                      raw_data = m_pRawData->at(raw_data_index);

        outData.append(raw_data);

        outData.append(((INSTR_LOAD_PROGRAM_MEMORY_PAGE >> 8) | HIGH_BYTE_MASK) & 0xff);
        outData.append( (INSTR_LOAD_PROGRAM_MEMORY_PAGE     )                   & 0xff);
        outData.append(m_iPageCurrentIndex);

        raw_data_index++;
        if (raw_data_index >= m_pRawData->size()) raw_data = 0xff;
        else                                      raw_data = m_pRawData->at(raw_data_index);

        outData.append(raw_data);

        m_iPageCurrentIndex++;
        if (m_iPageCurrentIndex >= (m_DeviceInfo.page_size >> 1))
        {
            m_iPageCurrentIndex = 0;

            if (m_pFTDIspi->sendData(outData) != FT_OK)
            {
                spi_error_signal();
                return;
            }

            outData.prepend(m_pFTDIspi->getData().at(0));
            outData.chop(1);
            if (outData.compare(m_pFTDIspi->getData()) != 0)
            {
                spi_error_signal();
                return;
            }

            outData.clear();
            outData.append((INSTR_WRITE_PROGRAM_MEMORY_PAGE >> 8) & 0xff);
            outData.append(((m_iAddress + m_iCurrentIndex)  >> 8) & 0xff);
            outData.append( (m_iAddress + m_iCurrentIndex)        & 0xff);
            outData.append((char)0);

            if (m_pFTDIspi->sendData(outData) != FT_OK)
            {
                spi_error_signal();
                return;
            }

            m_iCurrentIndex += m_DeviceInfo.page_size >> 1;
            if ((m_iCurrentIndex << 1) >= m_pRawData->size())
            {
                spi_alldata_programming_completed_signal();
                return;
            }

            break;
        }
    } while (true);

    spi_page_programming_completed_signal((m_iCurrentIndex << 1) * 100 / m_iDataSize);
    m_tSendNextWriteFLASHCommandTimer.start(1);
}

enum SPI_EVENTS CATmegaSerialProg::ReadFLASH(unsigned short address, QByteArray *p_data, unsigned short size)
{
    m_pRawData = p_data;
    m_iCurrentIndex = 0;
    m_iAddress = address >> 1;
    m_iDataSize = size;

    m_pRawData->clear();

    m_tSendNextReadFLASHCommandTimer.start(1);

    return SPI_OK;
}

void CATmegaSerialProg::SendNexReadFLASHCommandTimerSlot(void)
{
    if (m_pFTDIspi == nullptr) return;

    QByteArray outData;
    outData.clear();
    do
    {
        outData.append(((INSTR_READ_PROGRAM_MEMORY >> 8) | LOW_BYTE_MASK) & 0xff);
        outData.append(((m_iAddress + m_iCurrentIndex) >> 8) & 0xff);
        outData.append( (m_iAddress + m_iCurrentIndex)       & 0xff);
        outData.append((char)0);

        outData.append(((INSTR_READ_PROGRAM_MEMORY >> 8) | HIGH_BYTE_MASK) & 0xff);
        outData.append(((m_iAddress + m_iCurrentIndex) >> 8) & 0xff);
        outData.append( (m_iAddress + m_iCurrentIndex)       & 0xff);
        outData.append((char)0);

        m_iCurrentIndex++;
        bool end_of_data = (m_iCurrentIndex >= (m_iDataSize >> 1));
        if ((m_iCurrentIndex % (m_DeviceInfo.page_size >> 1)) == 0 || end_of_data)
        {
            if (m_pFTDIspi->sendData(outData) != FT_OK)
            {
                spi_error_signal();
                return;
            }

            bool compare_flag = true;
            int data_index = 0;
            foreach (char data, outData)
            {
                if (data_index < outData.size() - 1 && ((data_index + 2) % 4) != 0)
                {
                    if (data != m_pFTDIspi->getData().at(data_index + 1))
                    {
                        compare_flag = false;
                        break;
                    }
                }

                data_index++;
            }

            if (!compare_flag)
            {
                spi_error_signal();
                return;
            }

            for (int rec_byte_index = INSTRUCTION_DATA_INDEX; rec_byte_index < m_pFTDIspi->getData().size(); rec_byte_index += 4)
            {
                m_pRawData->append(m_pFTDIspi->getData().at(rec_byte_index));
            }

            if (end_of_data)
            {
                spi_alldata_programming_completed_signal();
            }
            else
            {
                spi_page_programming_completed_signal((m_iCurrentIndex << 1) * 100 / m_iDataSize);
                m_tSendNextReadFLASHCommandTimer.start(1);
            }

            return;
        }
    } while (true);
}

enum SPI_EVENTS CATmegaSerialProg::WriteEEPROM(unsigned short address, QByteArray *p_data)
{
    m_pRawData = p_data;
    m_iCurrentIndex = 0;
    m_iAddress = address;
    m_iDataSize = p_data->size();

    m_tSendNextWriteEEPROMCommandTimer.start(1);

    return SPI_OK;
}

void CATmegaSerialProg::SendNexWriteEEPROMCommandTimerSlot(void)
{
    if (m_pFTDIspi == nullptr) return;

    QByteArray outData;
    do
    {
        outData.clear();
        outData.append(((((unsigned int)INSTR_WRITE_EEPROM_MEMORY) >> 8) | LOW_BYTE_MASK) & 0xff);
        outData.append(((m_iAddress + m_iCurrentIndex) >> 8) & 0xff);
        outData.append( (m_iAddress + m_iCurrentIndex)       & 0xff);
        outData.append(m_pRawData->at(m_iCurrentIndex));

        if (m_pFTDIspi->sendData(outData) != FT_OK)
        {
            spi_error_signal();
            return;
        }

        outData.prepend(m_pFTDIspi->getData().at(0));
        outData.chop(1);
        if (outData.compare(m_pFTDIspi->getData()) != 0)
        {
            spi_error_signal();
            return;
        }

        m_iCurrentIndex++;
        if (m_iCurrentIndex >= m_pRawData->size())
        {
            spi_alldata_programming_completed_signal();
            return;
        }

        if (m_iCurrentIndex % (m_pRawData->size() / 100) == 0)
        {
            break;
        }
    } while (true);

    spi_page_programming_completed_signal(m_iCurrentIndex * 100 / m_iDataSize);
    m_tSendNextWriteEEPROMCommandTimer.start(1);
}

enum SPI_EVENTS CATmegaSerialProg::ReadEEPROM(unsigned short address, QByteArray *p_data, unsigned short size)
{
    m_pRawData = p_data;
    m_iCurrentIndex = 0;
    m_iAddress = address;
    m_iDataSize = size;

    m_pRawData->clear();

    m_tSendNextReadEEPROMCommandTimer.start(1);

    return SPI_OK;
}

void CATmegaSerialProg::SendNexReadEEPROMCommandTimerSlot(void)
{
    if (m_pFTDIspi == nullptr) return;

    QByteArray outData;
    outData.clear();
    do
    {
        outData.append(((unsigned int)INSTR_READ_EEPROM_MEMORY >> 8) & 0xff);
        outData.append(((m_iAddress + m_iCurrentIndex) >> 8) & 0xff);
        outData.append( (m_iAddress + m_iCurrentIndex)       & 0xff);
        outData.append((char)0x00);

        m_iCurrentIndex++;
        bool end_of_data = (m_iCurrentIndex >= m_iDataSize);
        if (m_iCurrentIndex % (m_iDataSize / 100) == 0 || end_of_data)
        {
            if (m_pFTDIspi->sendData(outData) != FT_OK)
            {
                spi_error_signal();
                return;
            }

            bool compare_flag = true;
            int data_index = 0;
            foreach (char data, outData)
            {
                if (data_index < outData.size() - 1 && ((data_index + 2) % 4) != 0)
                {
                    if (data != m_pFTDIspi->getData().at(data_index + 1))
                    {
                        compare_flag = false;
                        break;
                    }
                }

                data_index++;
            }

            if (!compare_flag)
            {
                spi_error_signal();
                return;
            }

            for (int rec_byte_index = INSTRUCTION_DATA_INDEX; rec_byte_index < m_pFTDIspi->getData().size(); rec_byte_index += 4)
            {
                m_pRawData->append(m_pFTDIspi->getData().at(rec_byte_index));
            }

            if (end_of_data)
            {
                spi_alldata_programming_completed_signal();
            }
            else
            {
                spi_page_programming_completed_signal(m_iCurrentIndex * 100 / m_iDataSize);
                m_tSendNextReadEEPROMCommandTimer.start(1);
            }

            return;
        }
    } while (true);
}

enum SPI_EVENTS CATmegaSerialProg::WriteFuseBytes(QByteArray fuses)
{
    QByteArray outData;
    outData.resize(4);
    outData[0] = (char)((INSTR_WRITE_FUSE_BITS >> 8)                  & 0xff);
    outData[1] = (char)(((INSTR_WRITE_FUSE_BITS    ) | LOW_BYTE_MASK) & 0xff);
    outData[2] = 0x00;
    outData[3] = fuses.at(LOW_FUSE_BYTE_INDEX);

    if (m_pFTDIspi->sendData(outData) != FT_OK) return SPI_ERROR;

    outData[1] = (char)(((INSTR_WRITE_FUSE_BITS    ) | HIGH_BYTE_MASK) & 0xff);
    outData[3] = fuses.at(HI_FUSE_BYTE_INDEX);

    if (m_pFTDIspi->sendData(outData) != FT_OK) return SPI_ERROR;

    outData[1] = (char)(((INSTR_WRITE_FUSE_BITS    ) | EXTENDED_BYTE_MASK) & 0xff);
    outData[3] = fuses.at(EXT_FUSE_BYTE_INDEX);

    if (m_pFTDIspi->sendData(outData) != FT_OK) return SPI_ERROR;

    return SPI_OK;
}

enum SPI_EVENTS CATmegaSerialProg::ReadFuseBytes(QByteArray &fuses)
{
    QByteArray outData;
    outData.resize(4);
    outData[0] = (char)((INSTR_READ_LOW_FUSE_BITS >> 8) & 0xff);
    outData[1] = (char)((INSTR_READ_LOW_FUSE_BITS     ) & 0xff);
    outData[2] = 0x00;
    outData[3] = 0x00;

    fuses.clear();
    if (m_pFTDIspi->sendData(outData) != FT_OK) return SPI_ERROR;
    fuses.append(m_pFTDIspi->getData().at(INSTRUCTION_DATA_INDEX));

    outData[0] = (char)((INSTR_READ_HIGH_FUSE_BITS >> 8) & 0xff);
    outData[1] = (char)((INSTR_READ_HIGH_FUSE_BITS     ) & 0xff);

    if (m_pFTDIspi->sendData(outData) != FT_OK) return SPI_ERROR;
    fuses.append(m_pFTDIspi->getData().at(INSTRUCTION_DATA_INDEX));

    outData[0] = (char)((INSTR_READ_EXT_FUSE_BITS >> 8) & 0xff);
    outData[1] = (char)((INSTR_READ_EXT_FUSE_BITS     ) & 0xff);

    if (m_pFTDIspi->sendData(outData) != FT_OK) return SPI_ERROR;
    fuses.append(m_pFTDIspi->getData().at(INSTRUCTION_DATA_INDEX));

    return SPI_OK;
}

enum SPI_EVENTS CATmegaSerialProg::WriteLockBits(unsigned char lock_bits)
{
    QByteArray outData;
    outData.resize(4);
    outData[0] = (char)((INSTR_WRITE_LOCK_BITS >> 8)                   & 0xff);
    outData[1] = (char)(((INSTR_WRITE_LOCK_BITS     ) | LOW_BYTE_MASK) & 0xff);
    outData[2] = 0x00;
    outData[3] = lock_bits;

    if (m_pFTDIspi->sendData(outData) != FT_OK) return SPI_ERROR;

    return SPI_OK;
}

void CATmegaSerialProg::OperationCancel(void)
{
    m_tSendNextReadFLASHCommandTimer.stop();
    m_tSendNextWriteFLASHCommandTimer.stop();
    m_tSendNextReadEEPROMCommandTimer.stop();
    m_tSendNextWriteEEPROMCommandTimer.stop();
}
