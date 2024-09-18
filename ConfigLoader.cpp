#include "ConfigLoader.h"

CConfigLoader::CConfigLoader(QString &FileName)
{
    CQsJsonFileLoader JsonFileLoader(FileName);

    int res = JsonFileLoader.LoadFromFile();
    if (res != FILE_OK)
    {
        m_FLASHfileName   = DEFAULT_FLASHFILE_NAME;
        m_EEPROMfileName.clear();
        m_ucLowFuseByte   = QByteArray::fromHex(DEFAULT_LOW_FUSE_BYTE).at(0);
        m_ucHiFuseByte    = QByteArray::fromHex(DEFAULT_HI_FUSE_BYTE).at(0);
        m_ucExtFuseByte   = QByteArray::fromHex(DEFAULT_EXT_FUSE_BYTE).at(0);
        m_bMemoryTestFlag = DEFAULT_MEMORY_TEST_FLAG;
        m_bMemoryLockFlag = DEFAULT_MEMORY_LOCK_FLAG;

        if (res == FILE_OPEN_ERROR) { m_ErrorsList.append(QString::fromLocal8Bit("Ошибка открытия файла конфигурации %0.").arg(FileName));
                                    }
        else                        { m_ErrorsList.append(QString::fromLocal8Bit("Ошибка формата файла конфигурации %0.").arg(FileName));
                                    }
        m_ErrorsList.append(QString::fromLocal8Bit("Установлены следующие параметры по умолчанию:"));
        m_ErrorsList.append(QString::fromLocal8Bit("  - имя файла прошивки FLASH: %0").arg(m_FLASHfileName));
        m_ErrorsList.append(QString::fromLocal8Bit("  - FUSE биты, младший байт: 0x%0").arg(QString::fromUtf8(QByteArray(1, m_ucLowFuseByte).toHex())));
        m_ErrorsList.append(QString::fromLocal8Bit("  - FUSE биты, старший байт: 0x%0").arg(QString::fromUtf8(QByteArray(1, m_ucHiFuseByte).toHex())));
        m_ErrorsList.append(QString::fromLocal8Bit("  - FUSE биты, расширенный байт: 0x%0").arg(QString::fromUtf8(QByteArray(1, m_ucExtFuseByte).toHex())));
        m_ErrorsList.append(QString::fromLocal8Bit("  - проверка FLASH после программирования: %0").arg(QString::fromLocal8Bit((m_bMemoryTestFlag) ? "да" : "нет")));
        m_ErrorsList.append(QString::fromLocal8Bit("  - защита памяти от копирования: %0").arg(QString::fromLocal8Bit((m_bMemoryLockFlag) ? "да" : "нет")));
        m_ErrorsList.append("\n\n\n");

        return;
    }

    m_FLASHfileName   = JsonFileLoader.GetParameter("FLASHfileName", DEFAULT_FLASHFILE_NAME).toString(DEFAULT_FLASHFILE_NAME);
    m_EEPROMfileName  = JsonFileLoader.GetParameter("EEPROMfileName", "").toString("");
    m_ucLowFuseByte   = QByteArray::fromHex(JsonFileLoader.GetParameter("LowFuseByte", DEFAULT_LOW_FUSE_BYTE).toString(DEFAULT_LOW_FUSE_BYTE).toUtf8()).at(0);
    m_ucHiFuseByte    = QByteArray::fromHex(JsonFileLoader.GetParameter("HiFuseByte",  DEFAULT_HI_FUSE_BYTE).toString(DEFAULT_HI_FUSE_BYTE).toUtf8()).at(0);
    m_ucExtFuseByte   = QByteArray::fromHex(JsonFileLoader.GetParameter("ExtFuseByte", DEFAULT_EXT_FUSE_BYTE).toString(DEFAULT_EXT_FUSE_BYTE).toUtf8()).at(0);
    m_bMemoryTestFlag = JsonFileLoader.GetParameter("MemoryTest", DEFAULT_MEMORY_TEST_FLAG).toBool(DEFAULT_MEMORY_TEST_FLAG);
    m_bMemoryLockFlag = JsonFileLoader.GetParameter("MemoryLock", DEFAULT_MEMORY_LOCK_FLAG).toBool(DEFAULT_MEMORY_LOCK_FLAG);

}
