#ifndef CINTELHEXDECODER_H
#define CINTELHEXDECODER_H

#include <QByteArray>
#include <QMap>

enum IHEX_DATA_TYPES
{
    IHEX_BIN_DATA   = 0x00,       // ������ �������� ������ ��������� �����
    IHEX_EOF        = 0x01,       // ������ �������� ������ �����
    IHEX_SEG_ADDR   = 0x02,       // ������ ������ ��������
    IHEX_RUN_ADDR   = 0x03,       // ��������� ����� ������� ��������� � ������� 80x86 (�������� ��������� CS:IP �������� ������ � �������� ��������)
    IHEX_EXT_ADDR   = 0x04,       // ������ ������������ ������
    IHEX_RUN32_ADDR = 0x05        // 32-������ ����� ������� ��������� � ��������� �������� ������������
};

class CIntelHEXparcer
{

public:

    CIntelHEXparcer(QByteArray &data, unsigned long max_RAW_data_size, unsigned char RAWdata_init_value = 0xff, bool chop_flag = true);

    QByteArray getRAWdata(void) {return m_RAWdata;}
    QMap<int, QString> getErrorsList(void) {return m_ErrorsList;}


private:

    QByteArray                    m_IntelHEXdata;
    QByteArray                    m_RAWdata;

    QMap<int, QString>            m_ErrorsList;

    unsigned long                 m_ulAddrOffset;

    unsigned long                 m_ulMaxRAWdataSize;
    unsigned char                 m_ucRAWdataInitValue;

    void clear(void);
    void parcing(bool chop_flag);

};

#endif // CINTELHEXDECODER_H
