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
        // Материалы для таблицы взяты с сайта https://unicode-table.com/ru
        // *****************************************************************
        case 0x80: pData->replace(i, 1, "\\u0402"); break;  // Кириллическая заглавная буква дьже
        case 0x81: pData->replace(i, 1, "\\u0403"); break;  // Кириллическая заглавная буква гэ
        case 0x82: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x83: pData->replace(i, 1, "\\u0453"); break;  // Кириллическая строчная буква гэ
        case 0x84: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x85: pData->replace(i, 1, "\\u2026"); break;  // Многоточие
        case 0x86: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x87: pData->replace(i, 1, "\\u01C2"); break;  // Латинская буква альвеолярный щелчок
        case 0x88: pData->replace(i, 1, "\\u20AC"); break;  // Знак Евро
        case 0x89: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x8A: pData->replace(i, 1, "\\u0409"); break;  // Кириллическая заглавная буква ле
        case 0x8B: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x8C: pData->replace(i, 1, "\\u040A"); break;  // Кириллическая заглавная буква нье
        case 0x8D: pData->replace(i, 1, "\\u040C"); break;  // Кириллическая заглавная буква кэ
        case 0x8E: pData->replace(i, 1, "\\u040b"); break;  // Кириллическая заглавная буква гервь
        case 0x8F: pData->replace(i, 1, "\\u040F"); break;  // Кириллическая заглавная буква дже
        case 0x90: pData->replace(i, 1, "\\u0452"); break;  // Кириллическая строчная буква дьже
        case 0x91: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x92: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x93: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x94: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x95: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x96: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x97: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x98: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x99: pData->replace(i, 1, "\\u2122"); break;  // Знак торговой марки
        case 0x9A: pData->replace(i, 1, "\\u0459"); break;  // Кириллическая строчная буква ле
        case 0x9B: pData->replace(i, 1, "\\u003F"); break;  //
        case 0x9C: pData->replace(i, 1, "\\u045A"); break;  // Кириллическая строчная буква нье
        case 0x9D: pData->replace(i, 1, "\\u045C"); break;  // Кириллическая строчная буква ке
        case 0x9E: pData->replace(i, 1, "\\u045B"); break;  // Кириллическая строчная буква гервь
        case 0x9F: pData->replace(i, 1, "\\u045F"); break;  // Кириллическая строчная буква дже
        case 0xA0: pData->replace(i, 1, "\\u003F"); break;  //
        case 0xA1: pData->replace(i, 1, "\\u040E"); break;  // Кириллическая заглавная буква у краткое
        case 0xA2: pData->replace(i, 1, "\\u045E"); break;  // Кириллическая строчная буква у краткое
        case 0xA3: pData->replace(i, 1, "\\u003F"); break;  //
        case 0xA4: pData->replace(i, 1, "\\u00A4"); break;  // Знак валюты
        case 0xA5: pData->replace(i, 1, "\\u0490"); break;  // Кириллическая заглавная буква г с подъёмом
        case 0xA6: pData->replace(i, 1, "\\u00A6"); break;  // Изломанная вертикальная черта
        case 0xA7: pData->replace(i, 1, "\\u00A7"); break;  // $
        case 0xA8: pData->replace(i, 1, "\\u0401"); break;  // Ё
        case 0xA9: pData->replace(i, 1, "\\u00A9"); break;  // Знак авторского права
        case 0xAA: pData->replace(i, 1, "\\u003F"); break;  //
        case 0xAB: pData->replace(i, 1, "\\u00AB"); break;  // Открывающая левая кавычка «ёлочка»
        case 0xAC: pData->replace(i, 1, "\\u00AC"); break;  // Знак отрицания
        case 0xAD: pData->replace(i, 1, "\\u003F"); break;  //
        case 0xAE: pData->replace(i, 1, "\\u003F"); break;  //
        case 0xAF: pData->replace(i, 1, "\\u0407"); break;  // Кириллическая заглавная буква йи
        case 0xB0: pData->replace(i, 1, "\\u00B0"); break;  // Знак градуса
        case 0xB1: pData->replace(i, 1, "\\u00B1"); break;  // Знак плюс-минус
        case 0xB2: pData->replace(i, 1, "\\u0406"); break;  // Кириллическая заглавная буква и десятеричное
        case 0xB3: pData->replace(i, 1, "\\u0456"); break;  // Кириллическая строчная буква и десятеричное
        case 0xB4: pData->replace(i, 1, "\\u0491"); break;  // Кириллическая строчная буква г с подъёмом
        case 0xB5: pData->replace(i, 1, "\\u00B5"); break;  // Знак микро
        case 0xB6: pData->replace(i, 1, "\\u00B6"); break;  // Знак абзаца
        case 0xB7: pData->replace(i, 1, "\\u00B7"); break;  // Точка по центру
        case 0xB8: pData->replace(i, 1, "\\u0451"); break;  // ё
        case 0xB9: pData->replace(i, 1, "\\u2116"); break;  // №
        case 0xBA: pData->replace(i, 1, "\\u003F"); break;  //
        case 0xBB: pData->replace(i, 1, "\\u00BB"); break;  // Закрывающая правая кавычка «ёлочка»
        case 0xBC: pData->replace(i, 1, "\\u003F"); break;  //
        case 0xBD: pData->replace(i, 1, "\\u0405"); break;  // Кириллическая заглавная буква дзе
        case 0xBE: pData->replace(i, 1, "\\u0455"); break;  // Кириллическая строчная буква дзе
        case 0xBF: pData->replace(i, 1, "\\u0457"); break;  // Кириллическая строчная буква йи
        case 0xC0: pData->replace(i, 1, "\\u0410"); break;  // А
        case 0xC1: pData->replace(i, 1, "\\u0411"); break;  // Б
        case 0xC2: pData->replace(i, 1, "\\u0412"); break;  // В
        case 0xC3: pData->replace(i, 1, "\\u0413"); break;  // Г
        case 0xC4: pData->replace(i, 1, "\\u0414"); break;  // Д
        case 0xC5: pData->replace(i, 1, "\\u0415"); break;  // Е
        case 0xC6: pData->replace(i, 1, "\\u0416"); break;  // Ж
        case 0xC7: pData->replace(i, 1, "\\u0417"); break;  // З
        case 0xC8: pData->replace(i, 1, "\\u0418"); break;  // И
        case 0xC9: pData->replace(i, 1, "\\u0419"); break;  // Й
        case 0xCA: pData->replace(i, 1, "\\u041A"); break;  // К
        case 0xCB: pData->replace(i, 1, "\\u041B"); break;  // Л
        case 0xCC: pData->replace(i, 1, "\\u041C"); break;  // М
        case 0xCD: pData->replace(i, 1, "\\u041D"); break;  // Н
        case 0xCE: pData->replace(i, 1, "\\u041E"); break;  // О
        case 0xCF: pData->replace(i, 1, "\\u041F"); break;  // П
        case 0xD0: pData->replace(i, 1, "\\u0420"); break;  // Р
        case 0xD1: pData->replace(i, 1, "\\u0421"); break;  // С
        case 0xD2: pData->replace(i, 1, "\\u0422"); break;  // Т
        case 0xD3: pData->replace(i, 1, "\\u0423"); break;  // У
        case 0xD4: pData->replace(i, 1, "\\u0424"); break;  // Ф
        case 0xD5: pData->replace(i, 1, "\\u0425"); break;  // Х
        case 0xD6: pData->replace(i, 1, "\\u0426"); break;  // Ц
        case 0xD7: pData->replace(i, 1, "\\u0427"); break;  // Ч
        case 0xD8: pData->replace(i, 1, "\\u0428"); break;  // Ш
        case 0xD9: pData->replace(i, 1, "\\u0429"); break;  // Щ
        case 0xDA: pData->replace(i, 1, "\\u042A"); break;  // Ъ
        case 0xDB: pData->replace(i, 1, "\\u042B"); break;  // Ы
        case 0xDC: pData->replace(i, 1, "\\u042C"); break;  // Ь
        case 0xDD: pData->replace(i, 1, "\\u042D"); break;  // Э
        case 0xDE: pData->replace(i, 1, "\\u042E"); break;  // Ю
        case 0xDF: pData->replace(i, 1, "\\u042F"); break;  // Я
        case 0xE0: pData->replace(i, 1, "\\u0430"); break;  // а
        case 0xE1: pData->replace(i, 1, "\\u0431"); break;  // б
        case 0xE2: pData->replace(i, 1, "\\u0432"); break;  // в
        case 0xE3: pData->replace(i, 1, "\\u0433"); break;  // г
        case 0xE4: pData->replace(i, 1, "\\u0434"); break;  // д
        case 0xE5: pData->replace(i, 1, "\\u0435"); break;  // е
        case 0xE6: pData->replace(i, 1, "\\u0436"); break;  // ж
        case 0xE7: pData->replace(i, 1, "\\u0437"); break;  // з
        case 0xE8: pData->replace(i, 1, "\\u0438"); break;  // и
        case 0xE9: pData->replace(i, 1, "\\u0439"); break;  // й
        case 0xEA: pData->replace(i, 1, "\\u043A"); break;  // к
        case 0xEB: pData->replace(i, 1, "\\u043B"); break;  // л
        case 0xEC: pData->replace(i, 1, "\\u043C"); break;  // м
        case 0xED: pData->replace(i, 1, "\\u043D"); break;  // н
        case 0xEE: pData->replace(i, 1, "\\u043E"); break;  // о
        case 0xEF: pData->replace(i, 1, "\\u043F"); break;  // п
        case 0xF0: pData->replace(i, 1, "\\u0440"); break;  // р
        case 0xF1: pData->replace(i, 1, "\\u0441"); break;  // с
        case 0xF2: pData->replace(i, 1, "\\u0442"); break;  // т
        case 0xF3: pData->replace(i, 1, "\\u0443"); break;  // у
        case 0xF4: pData->replace(i, 1, "\\u0444"); break;  // ф
        case 0xF5: pData->replace(i, 1, "\\u0445"); break;  // х
        case 0xF6: pData->replace(i, 1, "\\u0446"); break;  // ц
        case 0xF7: pData->replace(i, 1, "\\u0447"); break;  // ч
        case 0xF8: pData->replace(i, 1, "\\u0448"); break;  // ш
        case 0xF9: pData->replace(i, 1, "\\u0449"); break;  // щ
        case 0xFA: pData->replace(i, 1, "\\u044A"); break;  // ъ
        case 0xFB: pData->replace(i, 1, "\\u044B"); break;  // ы
        case 0xFC: pData->replace(i, 1, "\\u044C"); break;  // ь
        case 0xFD: pData->replace(i, 1, "\\u044D"); break;  // э
        case 0xFE: pData->replace(i, 1, "\\u044E"); break;  // ю
        case 0xFF: pData->replace(i, 1, "\\u044F"); break;  // я
        }
    }
}
