#ifndef QSJSONFILELOADER_H
#define QSJSONFILELOADER_H

#include <QObject>
#include <QFile>

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QVariant>
#include <QMetaType>

enum FILE_ERRORS
{
    FILE_OK = 0,
    FILE_OPEN_ERROR = 2,
    FILE_JSON_ERROR = 3
};

class CQsJsonFileLoader
{

private:
    QFile                         m_File;
    QJsonDocument                 m_JdocFileData;

public:
    CQsJsonFileLoader(){;}
    CQsJsonFileLoader(QString FileName);

    void SetFileName(QString FileName){m_File.setFileName(FileName);}

    int LoadFromFile(void);
    int SaveToFile(void);

    /*
     ************************************************************************************************************************************
     * Методы обеспечивают доступ к объектам структуры JSON любой степени вложенности.
     * Пример записи параметра:
     *     Json.SetRootObject("Object1", Json.SetObjectValue(Json.GetSubObject(Json.GetRootObject(), "Object1"), "Object2", ObjectData));
     * Пример чтения параметра:
     *     ObjectData = Json.GetObjectValue(Json.GetSubObject(Json.GetRootObject(), "Object1"), "Object2");
     ************************************************************************************************************************************
     */
    void SetRootObject(QString ObjectName, QJsonObject Object);
    QJsonObject SetSubObject(QJsonObject RootObject, QString ObjectName, QJsonObject Object);
    QJsonObject SetObjectValue(QJsonObject RootObject, QString Parameter, QVariant vData);

    QJsonObject GetRootObject(void) {return m_JdocFileData.object();}
    QJsonObject GetSubObject(QJsonObject RootObject, QString SubObjectName) {return RootObject.value(SubObjectName).toObject();}
    QJsonValue  GetObjectValue(QJsonObject Object, QString Parameter){return Object.value(Parameter);}
    /*
     ************************************************************************************************************************************
     */

    void SetParameter(QString Parameter, QVariant vData);
    void SetSubParameter(QString Parameter, QString SubParameter, QVariant vData);
    void SetArrayParameter(QString Array, int index, QString ArrayParameter, QVariant vData);
    void SetSubArrayParameter(QString Parameter, QString SubArray, int index, QString SubArrayParameter, QVariant vData);

    QJsonValue GetParameter(QString Parameter, QJsonValue DefaultValue);
    QJsonValue GetSubParameter(QString Parameter, QString SubParameter, QJsonValue DefaultValue);
    int GetArraySize(QString Array);
    int GetSubArraySize(QString Parameter, QString SubArray);
    QJsonValue GetArrayParameter(QString Array, int index, QString Parameter, QJsonValue DefaultValue);
    QJsonValue GetSubArrayParameter(QString Parameter, QString SubArray, int index, QString SubParameter, QJsonValue DefaultValue);

    void UnicodeJSONencode(QByteArray *pData);

};

#endif // QSJSONFILELOADER_H
