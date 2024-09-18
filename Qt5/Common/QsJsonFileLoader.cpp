#include "QsJsonFileLoader.h"

CQsJsonFileLoader::CQsJsonFileLoader(QString FileName)
{
    SetFileName(FileName);
}

int CQsJsonFileLoader::LoadFromFile(void)
{
    if (!m_File.open(QIODevice::ReadOnly)) return FILE_OPEN_ERROR;

    QByteArray FileData = m_File.readAll();

    m_File.close();

    UnicodeJSONencode(&FileData);

    QJsonParseError  parseError;

    m_JdocFileData = QJsonDocument::fromJson(FileData, &parseError);
    if (parseError.error != QJsonParseError::NoError) return FILE_JSON_ERROR;

    return FILE_OK;
}

int CQsJsonFileLoader::SaveToFile(void)
{
    if (!m_File.open(QIODevice::WriteOnly)) return FILE_OPEN_ERROR;
    m_File.write(m_JdocFileData.toJson());
    m_File.close();

    return FILE_OK;
}

void CQsJsonFileLoader::SetRootObject(QString SubObjectName, QJsonObject SubObject)
{
    if (SubObjectName.isEmpty()) return;

    QJsonObject Object = m_JdocFileData.object();
    Object.insert(SubObjectName, SubObject);

    m_JdocFileData.setObject(Object);
}

QJsonObject CQsJsonFileLoader::SetSubObject(QJsonObject RootObject, QString SubObjectName, QJsonObject SubObject)
{
    if (SubObjectName.isEmpty()) return QJsonObject();

    RootObject.insert(SubObjectName, SubObject);

    return RootObject;
}

QJsonObject CQsJsonFileLoader::SetObjectValue(QJsonObject RootObject, QString Parameter, QVariant vData)
{
    if (Parameter.isEmpty()) return QJsonObject();

    switch ((QMetaType::Type)vData.type())
    {
    case QMetaType::QString:    RootObject.insert(Parameter, vData.toString());    break;
    case QMetaType::Int:        RootObject.insert(Parameter, vData.toInt());       break;
    case QMetaType::Bool:       RootObject.insert(Parameter, vData.toBool());      break;
    case QMetaType::Double:     RootObject.insert(Parameter, vData.toDouble());    break;
    case QMetaType::QJsonArray: RootObject.insert(Parameter, vData.toJsonArray()); break;
    default:                    RootObject.insert(Parameter, QJsonValue::Null);    break;
    }

    return RootObject;
}

void CQsJsonFileLoader::SetParameter(QString Parameter, QVariant vData)
{
    if (Parameter.isEmpty()) return;

    QJsonObject JsonObj;

    JsonObj = m_JdocFileData.object();

    switch ((QMetaType::Type)vData.type())
    {
    case QMetaType::QString:    JsonObj.insert(Parameter, vData.toString());    break;
    case QMetaType::Int:        JsonObj.insert(Parameter, vData.toInt());       break;
    case QMetaType::Bool:       JsonObj.insert(Parameter, vData.toBool());      break;
    case QMetaType::Double:     JsonObj.insert(Parameter, vData.toDouble());    break;
    case QMetaType::QJsonArray: JsonObj.insert(Parameter, vData.toJsonArray()); break;
    default:                    JsonObj.insert(Parameter, QJsonValue::Null);    break;
    }

    m_JdocFileData.setObject(JsonObj);
}

void CQsJsonFileLoader::SetSubParameter(QString Parameter, QString SubParameter, QVariant vData)
{
    if (Parameter.isEmpty() || SubParameter.isEmpty()) return;

    QJsonObject JsonObj, JsonSubObj;

    JsonObj = m_JdocFileData.object();
    JsonSubObj = JsonObj.value(Parameter).toObject();

    switch ((QMetaType::Type)vData.type())
    {
    case QMetaType::QString:    JsonSubObj.insert(SubParameter, vData.toString());    break;
    case QMetaType::Int:        JsonSubObj.insert(SubParameter, vData.toInt());       break;
    case QMetaType::Bool:       JsonSubObj.insert(SubParameter, vData.toBool());      break;
    case QMetaType::Double:     JsonSubObj.insert(SubParameter, vData.toDouble());    break;
    case QMetaType::QJsonArray: JsonSubObj.insert(SubParameter, vData.toJsonArray()); break;
    default:                    JsonSubObj.insert(SubParameter, QJsonValue::Null);    break;
    }

    JsonObj.insert(Parameter, JsonSubObj);

    m_JdocFileData.setObject(JsonObj);
}

void CQsJsonFileLoader::SetArrayParameter(QString Array, int index, QString ArrayParameter, QVariant vData)
{
    if (index < 0 || ArrayParameter.isEmpty() || Array.isEmpty()) return;

    QJsonObject JsonObj, JsonArrayObj;
    QJsonArray JsonArray;

    JsonObj = m_JdocFileData.object();
    JsonArray = JsonObj.value(Array).toArray();
    JsonArrayObj = JsonArray[index].toObject();

    switch ((QMetaType::Type)vData.type())
    {
    case QMetaType::QString:    JsonArrayObj.insert(ArrayParameter, vData.toString());    break;
    case QMetaType::Int:        JsonArrayObj.insert(ArrayParameter, vData.toInt());       break;
    case QMetaType::Bool:       JsonArrayObj.insert(ArrayParameter, vData.toBool());      break;
    case QMetaType::Double:     JsonArrayObj.insert(ArrayParameter, vData.toDouble());    break;
    case QMetaType::QJsonArray: JsonArrayObj.insert(ArrayParameter, vData.toJsonArray()); break;
    default:                    JsonArrayObj.insert(ArrayParameter, QJsonValue::Null);    break;
    }

    if (JsonArray[index].toObject().isEmpty()) JsonArray.insert(index, JsonArrayObj);
    else                                       JsonArray.replace(index, JsonArrayObj);

    JsonObj.insert(Array, JsonArray);

    m_JdocFileData.setObject(JsonObj);
}

void CQsJsonFileLoader::SetSubArrayParameter(QString Parameter, QString SubArray, int index, QString SubArrayParameter, QVariant vData)
{
    if (index < 0 || Parameter.isEmpty() || SubArrayParameter.isEmpty() || SubArray.isEmpty()) return;

    QJsonObject JsonObj, JsonSubObj, JsonSubArrayObj;
    QJsonArray JsonSubArray;

    JsonObj = m_JdocFileData.object();
    JsonSubObj = JsonObj.value(Parameter).toObject();
    JsonSubArray = JsonSubObj.value(SubArray).toArray();
    JsonSubArrayObj = JsonSubArray[index].toObject();

    switch ((QMetaType::Type)vData.type())
    {
    case QMetaType::QString:    JsonSubArrayObj.insert(SubArrayParameter, vData.toString());    break;
    case QMetaType::Int:        JsonSubArrayObj.insert(SubArrayParameter, vData.toInt());       break;
    case QMetaType::Bool:       JsonSubArrayObj.insert(SubArrayParameter, vData.toBool());      break;
    case QMetaType::Double:     JsonSubArrayObj.insert(SubArrayParameter, vData.toDouble());    break;
    case QMetaType::QJsonArray: JsonSubArrayObj.insert(SubArrayParameter, vData.toJsonArray()); break;
    default:                    JsonSubArrayObj.insert(SubArrayParameter, QJsonValue::Null);    break;
    }

    if (JsonSubArray[index].toObject().isEmpty()) JsonSubArray.insert(index, JsonSubArrayObj);
    else                                          JsonSubArray.replace(index, JsonSubArrayObj);

    JsonSubObj.insert(SubArray, JsonSubArray);
    JsonObj.insert(Parameter, JsonSubObj);

    m_JdocFileData.setObject(JsonObj);
}

QJsonValue CQsJsonFileLoader::GetParameter(QString Parameter, QJsonValue DefaultValue)
{
    if (Parameter.isEmpty()) return DefaultValue;

    QJsonObject JsonObj = m_JdocFileData.object();
    if (!JsonObj.contains(Parameter)) return DefaultValue;

    return JsonObj[Parameter];
}

QJsonValue CQsJsonFileLoader::GetSubParameter(QString Parameter, QString SubParameter, QJsonValue DefaultValue)
{
    if (Parameter.isEmpty() || SubParameter.isEmpty()) return DefaultValue;

    QJsonObject JsonObj, JsonSubObj;

    JsonObj = m_JdocFileData.object();
    if (!JsonObj.contains(Parameter)) return DefaultValue;

    JsonSubObj = JsonObj.value(Parameter).toObject();
    if (!JsonSubObj.contains(SubParameter)) return DefaultValue;

    return JsonSubObj[SubParameter];
}

int CQsJsonFileLoader::GetArraySize(QString Array)
{
    if (Array.isEmpty()) return -1;

    QJsonObject JsonObj;
    QJsonArray JsonArray;

    JsonObj = m_JdocFileData.object();
    if (!JsonObj.contains(Array)) return -1;

    JsonArray = JsonObj.value(Array).toArray();
    if (JsonArray.isEmpty()) return 0;

    return JsonArray.size();
}

int CQsJsonFileLoader::GetSubArraySize(QString Parameter, QString SubArray)
{
    if (Parameter.isEmpty() || SubArray.isEmpty()) return -1;

    QJsonObject JsonObj, JsonSubObj;
    QJsonArray JsonSubArray;

    JsonObj = m_JdocFileData.object();
    if (!JsonObj.contains(Parameter)) return -1;

    JsonSubObj = JsonObj.value(Parameter).toObject();
    if (!JsonSubObj.contains(SubArray)) return -1;

    JsonSubArray = JsonSubObj.value(SubArray).toArray();
    if (JsonSubArray.isEmpty()) return 0;

    return JsonSubArray.size();
}

QJsonValue CQsJsonFileLoader::GetArrayParameter(QString Array, int index, QString Parameter, QJsonValue DefaultValue)
{
    if (index < 0 || Parameter.isEmpty() || Array.isEmpty()) return DefaultValue;

    QJsonObject JsonObj, JsonArrayObj;
    QJsonArray JsonArray;

    JsonObj = m_JdocFileData.object();
    if (!JsonObj.contains(Array)) return DefaultValue;

    JsonArray = JsonObj.value(Array).toArray();
    if (JsonArray.isEmpty()) return DefaultValue;

    JsonArrayObj = JsonArray.at(index).toObject();
    if (!JsonArrayObj.contains(Parameter)) return DefaultValue;

    return JsonArrayObj.value(Parameter);
}

QJsonValue CQsJsonFileLoader::GetSubArrayParameter(QString Parameter, QString SubArray, int index, QString SubParameter, QJsonValue DefaultValue)
{
    if (index < 0 || Parameter.isEmpty() || SubArray.isEmpty() || SubParameter.isEmpty()) return DefaultValue;

    QJsonObject JsonObj, JsonSubObj, JsonArrayObj;
    QJsonArray JsonArray;

    JsonObj = m_JdocFileData.object();
    if (!JsonObj.contains(Parameter)) return DefaultValue;

    JsonSubObj = JsonObj.value(Parameter).toObject();
    if (!JsonSubObj.contains(SubArray)) return DefaultValue;

    JsonArray = JsonSubObj.value(SubArray).toArray();
    if (JsonArray.isEmpty()) return DefaultValue;

    JsonArrayObj = JsonArray.at(index).toObject();
    if (!JsonArrayObj.contains(SubParameter)) return DefaultValue;

    return JsonArrayObj.value(SubParameter);
}

void CQsJsonFileLoader::UnicodeJSONencode(QByteArray *pData)
{
    for (int i=0; i < pData->size(); i++)
    {
        switch ((unsigned char)pData->at(i))
        {
        // *****************************************************************
        // ��������� ��� ������� ����� � ����� https://unicode-table.com/ru
        // *****************************************************************
        case 0x80: pData->replace(i, 1, "\\u0402"); break;  // ������������� ��������� ����� ����
        case 0x81: pData->replace(i, 1, "\\u0403"); break;  // ������������� ��������� ����� ��
        case 0x82: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x83: pData->replace(i, 1, "\\u0453"); break;  // ������������� �������� ����� ��
        case 0x84: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x85: pData->replace(i, 1, "\\u2026"); break;  // ����������
        case 0x86: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x87: pData->replace(i, 1, "\\u01C2"); break;  // ��������� ����� ������������ ������
        case 0x88: pData->replace(i, 1, "\\u20AC"); break;  // ���� ����
        case 0x89: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x8A: pData->replace(i, 1, "\\u0409"); break;  // ������������� ��������� ����� ��
        case 0x8B: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x8C: pData->replace(i, 1, "\\u040A"); break;  // ������������� ��������� ����� ���
        case 0x8D: pData->replace(i, 1, "\\u040C"); break;  // ������������� ��������� ����� ��
        case 0x8E: pData->replace(i, 1, "\\u040b"); break;  // ������������� ��������� ����� �����
        case 0x8F: pData->replace(i, 1, "\\u040F"); break;  // ������������� ��������� ����� ���
        case 0x90: pData->replace(i, 1, "\\u0452"); break;  // ������������� �������� ����� ����
        case 0x91: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x92: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x93: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x94: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x95: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x96: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x97: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x98: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x99: pData->replace(i, 1, "\\u2122"); break;  // ���� �������� �����
        case 0x9A: pData->replace(i, 1, "\\u0459"); break;  // ������������� �������� ����� ��
        case 0x9B: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x9C: pData->replace(i, 1, "\\u045A"); break;  // ������������� �������� ����� ���
        case 0x9D: pData->replace(i, 1, "\\u045C"); break;  // ������������� �������� ����� ��
        case 0x9E: pData->replace(i, 1, "\\u045B"); break;  // ������������� �������� ����� �����
        case 0x9F: pData->replace(i, 1, "\\u045F"); break;  // ������������� �������� ����� ���
        case 0xA0: pData->replace(i, 1, "\\u003F"); break;  //
        case 0xA1: pData->replace(i, 1, "\\u040E"); break;  // ������������� ��������� ����� � �������
        case 0xA2: pData->replace(i, 1, "\\u045E"); break;  // ������������� �������� ����� � �������
        case 0xA3: pData->replace(i, 1, "\\u003F"); break;  //
        case 0xA4: pData->replace(i, 1, "\\u00A4"); break;  // ���� ������
        case 0xA5: pData->replace(i, 1, "\\u0490"); break;  // ������������� ��������� ����� � � ��������
        case 0xA6: pData->replace(i, 1, "\\u00A6"); break;  // ���������� ������������ �����
        case 0xA7: pData->replace(i, 1, "\\u00A7"); break;  // $
        case 0xA8: pData->replace(i, 1, "\\u0401"); break;  // �
        case 0xA9: pData->replace(i, 1, "\\u00A9"); break;  // ���� ���������� �����
        case 0xAA: pData->replace(i, 1, "\\u003F"); break;  //
        case 0xAB: pData->replace(i, 1, "\\u00AB"); break;  // ����������� ����� ������� �������
        case 0xAC: pData->replace(i, 1, "\\u00AC"); break;  // ���� ���������
        case 0xAD: pData->replace(i, 1, "\\u003F"); break;  //
        case 0xAE: pData->replace(i, 1, "\\u003F"); break;  //
        case 0xAF: pData->replace(i, 1, "\\u0407"); break;  // ������������� ��������� ����� ��
        case 0xB0: pData->replace(i, 1, "\\u00B0"); break;  // ���� �������
        case 0xB1: pData->replace(i, 1, "\\u00B1"); break;  // ���� ����-�����
        case 0xB2: pData->replace(i, 1, "\\u0406"); break;  // ������������� ��������� ����� � ������������
        case 0xB3: pData->replace(i, 1, "\\u0456"); break;  // ������������� �������� ����� � ������������
        case 0xB4: pData->replace(i, 1, "\\u0491"); break;  // ������������� �������� ����� � � ��������
        case 0xB5: pData->replace(i, 1, "\\u00B5"); break;  // ���� �����
        case 0xB6: pData->replace(i, 1, "\\u00B6"); break;  // ���� ������
        case 0xB7: pData->replace(i, 1, "\\u00B7"); break;  // ����� �� ������
        case 0xB8: pData->replace(i, 1, "\\u0451"); break;  // �
        case 0xB9: pData->replace(i, 1, "\\u2116"); break;  // �
        case 0xBA: pData->replace(i, 1, "\\u003F"); break;  //
        case 0xBB: pData->replace(i, 1, "\\u00BB"); break;  // ����������� ������ ������� �������
        case 0xBC: pData->replace(i, 1, "\\u003F"); break;  //
        case 0xBD: pData->replace(i, 1, "\\u0405"); break;  // ������������� ��������� ����� ���
        case 0xBE: pData->replace(i, 1, "\\u0455"); break;  // ������������� �������� ����� ���
        case 0xBF: pData->replace(i, 1, "\\u0457"); break;  // ������������� �������� ����� ��
        case 0xC0: pData->replace(i, 1, "\\u0410"); break;  // �
        case 0xC1: pData->replace(i, 1, "\\u0411"); break;  // �
        case 0xC2: pData->replace(i, 1, "\\u0412"); break;  // �
        case 0xC3: pData->replace(i, 1, "\\u0413"); break;  // �
        case 0xC4: pData->replace(i, 1, "\\u0414"); break;  // �
        case 0xC5: pData->replace(i, 1, "\\u0415"); break;  // �
        case 0xC6: pData->replace(i, 1, "\\u0416"); break;  // �
        case 0xC7: pData->replace(i, 1, "\\u0417"); break;  // �
        case 0xC8: pData->replace(i, 1, "\\u0418"); break;  // �
        case 0xC9: pData->replace(i, 1, "\\u0419"); break;  // �
        case 0xCA: pData->replace(i, 1, "\\u041A"); break;  // �
        case 0xCB: pData->replace(i, 1, "\\u041B"); break;  // �
        case 0xCC: pData->replace(i, 1, "\\u041C"); break;  // �
        case 0xCD: pData->replace(i, 1, "\\u041D"); break;  // �
        case 0xCE: pData->replace(i, 1, "\\u041E"); break;  // �
        case 0xCF: pData->replace(i, 1, "\\u041F"); break;  // �
        case 0xD0: pData->replace(i, 1, "\\u0420"); break;  // �
        case 0xD1: pData->replace(i, 1, "\\u0421"); break;  // �
        case 0xD2: pData->replace(i, 1, "\\u0422"); break;  // �
        case 0xD3: pData->replace(i, 1, "\\u0423"); break;  // �
        case 0xD4: pData->replace(i, 1, "\\u0424"); break;  // �
        case 0xD5: pData->replace(i, 1, "\\u0425"); break;  // �
        case 0xD6: pData->replace(i, 1, "\\u0426"); break;  // �
        case 0xD7: pData->replace(i, 1, "\\u0427"); break;  // �
        case 0xD8: pData->replace(i, 1, "\\u0428"); break;  // �
        case 0xD9: pData->replace(i, 1, "\\u0429"); break;  // �
        case 0xDA: pData->replace(i, 1, "\\u042A"); break;  // �
        case 0xDB: pData->replace(i, 1, "\\u042B"); break;  // �
        case 0xDC: pData->replace(i, 1, "\\u042C"); break;  // �
        case 0xDD: pData->replace(i, 1, "\\u042D"); break;  // �
        case 0xDE: pData->replace(i, 1, "\\u042E"); break;  // �
        case 0xDF: pData->replace(i, 1, "\\u042F"); break;  // �
        case 0xE0: pData->replace(i, 1, "\\u0430"); break;  // �
        case 0xE1: pData->replace(i, 1, "\\u0431"); break;  // �
        case 0xE2: pData->replace(i, 1, "\\u0432"); break;  // �
        case 0xE3: pData->replace(i, 1, "\\u0433"); break;  // �
        case 0xE4: pData->replace(i, 1, "\\u0434"); break;  // �
        case 0xE5: pData->replace(i, 1, "\\u0435"); break;  // �
        case 0xE6: pData->replace(i, 1, "\\u0436"); break;  // �
        case 0xE7: pData->replace(i, 1, "\\u0437"); break;  // �
        case 0xE8: pData->replace(i, 1, "\\u0438"); break;  // �
        case 0xE9: pData->replace(i, 1, "\\u0439"); break;  // �
        case 0xEA: pData->replace(i, 1, "\\u043A"); break;  // �
        case 0xEB: pData->replace(i, 1, "\\u043B"); break;  // �
        case 0xEC: pData->replace(i, 1, "\\u043C"); break;  // �
        case 0xED: pData->replace(i, 1, "\\u043D"); break;  // �
        case 0xEE: pData->replace(i, 1, "\\u043E"); break;  // �
        case 0xEF: pData->replace(i, 1, "\\u043F"); break;  // �
        case 0xF0: pData->replace(i, 1, "\\u0440"); break;  // �
        case 0xF1: pData->replace(i, 1, "\\u0441"); break;  // �
        case 0xF2: pData->replace(i, 1, "\\u0442"); break;  // �
        case 0xF3: pData->replace(i, 1, "\\u0443"); break;  // �
        case 0xF4: pData->replace(i, 1, "\\u0444"); break;  // �
        case 0xF5: pData->replace(i, 1, "\\u0445"); break;  // �
        case 0xF6: pData->replace(i, 1, "\\u0446"); break;  // �
        case 0xF7: pData->replace(i, 1, "\\u0447"); break;  // �
        case 0xF8: pData->replace(i, 1, "\\u0448"); break;  // �
        case 0xF9: pData->replace(i, 1, "\\u0449"); break;  // �
        case 0xFA: pData->replace(i, 1, "\\u044A"); break;  // �
        case 0xFB: pData->replace(i, 1, "\\u044B"); break;  // �
        case 0xFC: pData->replace(i, 1, "\\u044C"); break;  // �
        case 0xFD: pData->replace(i, 1, "\\u044D"); break;  // �
        case 0xFE: pData->replace(i, 1, "\\u044E"); break;  // �
        case 0xFF: pData->replace(i, 1, "\\u044F"); break;  // �
        }
    }
}
