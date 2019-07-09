#ifndef DLT645_H
#define DLT645_H

#include <QObject>
#include <QtPlugin>
#include <QPair>
#include <QVariantHash>

//#define PLG_SAMPLE  0 //0 - full plagin , 1 - only itIsYour and itIsYourRead + nessasory some functions
#define PLG_BBB 1

#include "shared/meterplugin.h"


#include "dt645encoderdecoder.h"


class DT645 : public QObject, MeterPlugin
{

    Q_OBJECT


    Q_PLUGIN_METADATA(IID "ua.zbyralko.hello_zb.MeterPlugin" FILE "zbyralko.json")
    Q_INTERFACES(MeterPlugin)

public:
    QString getMeterType() ;

    QString getMeterAddrsAndPsswdRegExp();

    quint16 getPluginVersion() ;

    quint8 getPasswdType() ;

    QString getVersion() ;

    QByteArray getDefPasswd() ;

    QString getSupportedMeterList() ;

    quint8 getMaxTariffNumber(const QString &version) ;

    QStringList getEnrgList4thisMeter(const quint8 &pollCode, const QString &version) ;

    quint8 getMeterTypeTag();

    Mess2meterRezult mess2meter(const Mess2meterArgs &pairAboutMeter);

    QVariantHash decodeMeterData(const DecodeMeterMess &threeHash);

    QVariantHash helloMeter(const QVariantHash &hashMeterConstData);

    QString meterTypeFromMessage(const QByteArray &readArr);

    QVariantHash isItYour(const QByteArray &arr);

    QVariantHash isItYourRead(const QByteArray &arr);

    QByteArray niChanged(const QByteArray &arr);

    QVariantHash meterSn2NI(const QString &meterSn);



    //parametryzatsiya
    Mess2meterRezult messParamPamPam(const Mess2meterArgs &pairAboutMeter);
    QVariantHash decodeParamPamPam(const DecodeMeterMess &threeHash) ;

    QVariantHash how2logout(const QVariantHash &hashConstData, const QVariantHash &hashTmpData) ;

    QVariantHash getDoNotSleep(const quint8 &minutes);

    QVariantHash getGoToSleep(const quint8 &seconds) ;

private:

    DT645EncoderDecoder encoderdecoder;


    QVariantHash fullCurrent(const QVariantList &meterMessVar, const quint8 &errCode, quint16 &step,
                           const QVariantHash &hashConstData, const QVariantHash &hashTmpData, const QList<QByteArray> &lastDump, int &warning_counter, int &error_counter);


    QVariantHash fullVoltage(const QVariantList &meterMessVar, const quint8 &errCode, quint16 &step,
                           const QVariantHash &hashConstData, const QVariantHash &hashTmpData, const QList<QByteArray> &lastDump, int &warning_counter, int &error_counter);

    QVariantHash fullPower(const QVariantList &meterMessVar, const quint8 &errCode, quint16 &step,
                           const QVariantHash &hashConstData, const QVariantHash &hashTmpData, const QList<QByteArray> &lastDump, int &warning_counter, int &error_counter);

    QVariantHash fullMeterJrnl(const QVariantList &meterMessVar, const quint8 &errCode, quint16 &step,
                           const QVariantHash &hashConstData, const QVariantHash &hashTmpData, const QList<QByteArray> &lastDump, int &warning_counter, int &error_counter);

    QVariantHash fullEoD(const QVariantList &meterMessVar, const quint8 &errCode, quint16 &step,
                           const QVariantHash &hashConstData, const QVariantHash &hashTmpData, const QList<QByteArray> &lastDump, int &warning_counter, int &error_counter);

    QVariantHash fullEoM(const QVariantList &meterMessVar, const quint8 &errCode, quint16 &step,
                           const QVariantHash &hashConstData, const QVariantHash &hashTmpData, const QList<QByteArray> &lastDump, int &warning_counter, int &error_counter);

    bool decodeMeterDtSnVrsn(const QVariantList &meterMessVar, const QVariantHash &hashConstData, const QVariantHash &hashTmpData, QVariantHash &hash, quint16 &step, int &warning_counter, int &error_counter, int &goodObis);






    void resetHDLCglobalVariables();


};


#endif // DLT645_H
