#ifndef CCONFIGLOADER_H
#define CCONFIGLOADER_H

#include "../../../Qt5/Common/QsJsonFileLoader.h"

#define DEFAULT_FLASHFILE_NAME    "flash.hex"

#define DEFAULT_LOW_FUSE_BYTE     "EC"
#define DEFAULT_HI_FUSE_BYTE      "DC"
#define DEFAULT_EXT_FUSE_BYTE     "FF"

#define DEFAULT_MEMORY_TEST_FLAG  true
#define DEFAULT_MEMORY_LOCK_FLAG  false

#define DEFAULT_CHIP_LOCK_BITS    0xfc

class CConfigLoader
{

public:

    CConfigLoader(void){;}
    CConfigLoader(QString &FileName);

    QString getFLASHfileName(void)     {return m_FLASHfileName;}
    QString getEEPROMfileName(void)    {return m_EEPROMfileName;}
    unsigned char getLowFuseByte(void) {return m_ucLowFuseByte;}
    unsigned char getHiFuseByte(void)  {return m_ucHiFuseByte;}
    unsigned char getExtFuseByte(void) {return m_ucExtFuseByte;}
    bool getMemoryTestFlag(void) {return m_bMemoryTestFlag;}
    bool getMemoryLockFlag(void) {return m_bMemoryLockFlag;}

    QList<QString> getErrorsList(void) {return m_ErrorsList;}

private:

    QString                       m_FLASHfileName;
    QString                       m_EEPROMfileName;
    unsigned char                 m_ucLowFuseByte;
    unsigned char                 m_ucHiFuseByte;
    unsigned char                 m_ucExtFuseByte;
    bool                          m_bMemoryTestFlag;
    bool                          m_bMemoryLockFlag;

    QList<QString>                m_ErrorsList;

};

#endif // CCONFIGLOADER_H
