#include "IntelHEXparcer.h"

#pragma pack(push, 1)

struct INTEL_HEX_LINE_HEADER
{
    unsigned char                 ucSize;
    unsigned short                usAddress;
    unsigned char                 ucType;
};

#pragma pack(pop)

CIntelHEXparcer::CIntelHEXparcer(QByteArray &data, unsigned long max_RAW_data_size, unsigned char RAWdata_init_value, bool chop_flag)
{
    m_IntelHEXdata       = data;
    m_ulAddrOffset       = 0;
    m_ulMaxRAWdataSize   = max_RAW_data_size;
    m_ucRAWdataInitValue = RAWdata_init_value;

    clear();
    parcing(chop_flag);
}

void CIntelHEXparcer::clear(void)
{
    m_RAWdata.resize(m_ulMaxRAWdataSize);
    m_RAWdata.fill(m_ucRAWdataInitValue);
}

void CIntelHEXparcer::parcing(bool chop_flag)
{
    int iCurrentPos = 0;
    int iEndLinePos = 0;
    int iLineIndex  = 0;
    m_ErrorsList.clear();
    do
    {
        iEndLinePos = m_IntelHEXdata.indexOf('\n', iCurrentPos);
        if (iEndLinePos < 0) continue;

        iLineIndex++;

        QByteArray subData = m_IntelHEXdata.left(iEndLinePos);
        subData = subData.right(subData.size() - iCurrentPos);

        iCurrentPos = iEndLinePos + 1;

        subData.remove(0, subData.indexOf(':') + 1);

        QByteArray lineRAWdata = QByteArray::fromHex(subData);
        if (lineRAWdata.size() == 0)
        {
            m_ErrorsList.insert(iLineIndex, QString::fromLocal8Bit("нет данных"));
            continue;
        }

        char CheckSumm = 0;
        foreach (char data, lineRAWdata)
        {   CheckSumm += data;
        }
        if (CheckSumm != 0)
        {
            m_ErrorsList.insert(iLineIndex, QString::fromLocal8Bit("ошибка контрольной суммы"));
            continue;
        }

        struct INTEL_HEX_LINE_HEADER LineHeader = *((struct INTEL_HEX_LINE_HEADER *)lineRAWdata.constData());
        LineHeader.usAddress = ((LineHeader.usAddress >> 8) & 0x00ff) | ((LineHeader.usAddress << 8) & 0xff00);

        lineRAWdata.remove(0, sizeof(struct INTEL_HEX_LINE_HEADER));
        lineRAWdata.chop(1);

        if (LineHeader.ucSize != lineRAWdata.size())
        {
            m_ErrorsList.insert(iLineIndex, QString::fromLocal8Bit("ошибка размера данных"));
            continue;
        }

        switch (LineHeader.ucType)
        {
        case IHEX_BIN_DATA:
        {
            if (LineHeader.usAddress + m_ulAddrOffset + LineHeader.ucSize > (unsigned)m_RAWdata.size())
            {
                m_ErrorsList.insert(iLineIndex, QString::fromLocal8Bit("данные находятся вне зоны адресного пространства FLASH памяти"));
            }
            else
            {
                m_RAWdata.replace(LineHeader.usAddress + m_ulAddrOffset, LineHeader.ucSize, lineRAWdata);
            }
            break;
        }

        case IHEX_EOF:
        {
            iEndLinePos = -1;
            break;
        }

        case IHEX_SEG_ADDR:
        {
            m_ulAddrOffset = (((lineRAWdata[1] >> 8) & 0x00ff) | ((lineRAWdata[0] << 8) & 0xff00)) << 4;
            break;
        }

        case IHEX_EXT_ADDR:
        {
            m_ulAddrOffset = (((lineRAWdata[1] >> 8) & 0x00ff) | ((lineRAWdata[0] << 8) & 0xff00)) << 16;
            break;
        }

        case IHEX_RUN_ADDR:
        case IHEX_RUN32_ADDR:
        default:
        {
            break;
        }
        }
    } while (iEndLinePos >= 0);

    if (m_RAWdata.size() > 0)
    {
        while (chop_flag && m_RAWdata.back() == (char)0xff)
        {
            m_RAWdata.chop(1);
        }
    }
}
