/*
 * Bohdan Zikranets, bohdan@kts-intek.com.ua KTS-INTEK, 2019
 */

#include "dlt645.h"
#include "dt645encoderdecoder.h"


#include <QtCore>
#include <QDebug>
#include <QTime>

///[!] meter-plugin-shared
#include "shared/meterpluginhelper.h"

///[!] type-converter
#include "src/base/prettyvalues.h"
#include "src/base/convertatype.h"

#include "ucmetereventcodes.h"
#include "moji_defy.h"
#include "matildalimits.h"

#include "myucmmeterstypes.h"
#include "definedpollcodes.h"

#define PLG_4_MATILDA    1
#define PLG_4_PC         1 //Parametrization


//-----------------------------------------------------------------------------


QString DT645::getMeterType(){ return QString("DLT645"); }

QString DT645::getMeterAddrsAndPsswdRegExp(){ return QString("%1%2").arg("^(0|[1-9][0-8]{10}[0-9]|[1-9]{11}[0-8])$").arg("^([0-9]{4})$");}

quint16 DT645::getPluginVersion(){ return PLG_VER_RELEASE; }

quint8 DT645::getPasswdType(){ return UCM_PSWRD_DIGIT; }

QString DT645::getVersion(){ return QString("DLT645 v0.0.1 %1").arg(QString(BUILDDATE)); }

QByteArray DT645::getDefPasswd()
{
    return QByteArray("1234");
}

QString DT645::getSupportedMeterList(){ return QString("DLT645-97,DDSF22,DDSD22");}

quint8 DT645::getMaxTariffNumber(const QString &version){  Q_UNUSED(version); return 4; }

QStringList DT645::getEnrgList4thisMeter(const quint8 &pollCode, const QString &version){ return DT645EncoderDecoder::getSupportedEnrg(pollCode, version); }

quint8 DT645::getMeterTypeTag(){ return UC_METER_ELECTRICITY; }

//-----------------------------------------------------------------------------

Mess2meterRezult DT645::mess2meter(const Mess2meterArgs &pairAboutMeter)
{
    return encoderdecoder.mess2meter(pairAboutMeter);
}

//-----------------------------------------------------------------------------

QVariantHash DT645::decodeMeterData(const DecodeMeterMess &threeHash)
{
    return encoderdecoder.decodeMeterData(threeHash);
}

//-----------------------------------------------------------------------------

QVariantHash DT645::helloMeter(const QVariantHash &hashMeterConstData)
{
    QVariantHash hash;
    /*
    * bool data7EPt = hashMessage.value("data7EPt", false).toBool();
    * QByteArray endSymb = hashMessage.value("endSymb", QByteArray("\r\n")).toByteArray();
    * QByteArray currNI = hashMeterConstData.value("NI").toByteArray();
    * hashMessage.value("message")
*/
    hash.insert("quickCRC", true);
    hash.insert("endSymb", QByteArray(1, quint8(DLT645_END_FRAME)));
    hash.insert("message", encoderdecoder.getReadMessage(hashMeterConstData, DLT645_METER_SN));
    //7E 52 FF FF FE FF DC 46 7E
    return hash;
}

//-----------------------------------------------------------------------------

QString DT645::meterTypeFromMessage(const QByteArray &readArr)
{
    const DT645EncoderDecoder::MessageValidatorResult result = encoderdecoder.messageIsValid(readArr, QByteArray());
    if(result.isValid){
        return getMeterType();

    }
     return "";
}

//-----------------------------------------------------------------------------

QVariantHash DT645::isItYour(const QByteArray &arr)
{
    QByteArray dest;
    const DT645EncoderDecoder::MessageValidatorResult result = encoderdecoder.messageIsValidExt(arr, QByteArray(), false, dest);
    if(result.isValid){
        QVariantHash hash;
        hash.insert("nodeID", dest + "\r\n");
        encoderdecoder.insertDefaultHashMessageValues(hash);
        return hash;

    }
    return QVariantHash();
}

//-----------------------------------------------------------------------------

QVariantHash DT645::isItYourRead(const QByteArray &arr)
{
    const DT645EncoderDecoder::MessageValidatorResult result = encoderdecoder.messageIsValid(arr, QByteArray());
    if(result.isValid){
        QVariantHash hash;
        hash.insert("Tak", true);
        return hash;
    }
    return QVariantHash();
}

//-----------------------------------------------------------------------------

QByteArray DT645::niChanged(const QByteArray &arr)
{
    Q_UNUSED(arr);
    return QByteArray();
}

//-----------------------------------------------------------------------------

QVariantHash DT645::meterSn2NI(const QString &meterSn)
{
    /*
     * hard - без варіантів (жорстко), від старого до нового ф-ту
     * altr - альтернатива для стандартного варіанту
     * <keys> : <QStringList>
     */
    QVariantHash h;
    const QString sn = DT645EncoderDecoder::calcMeterAddr(meterSn);
    if(!sn.isEmpty())
        h.insert("hard", sn.split(";"));
    return h;
}

//-----------------------------------------------------------------------------

Mess2meterRezult DT645::messParamPamPam(const Mess2meterArgs &pairAboutMeter)
{
    return encoderdecoder.messParamPamPam(pairAboutMeter);
}

//-----------------------------------------------------------------------------

QVariantHash DT645::decodeParamPamPam(const DecodeMeterMess &threeHash)
{
    return encoderdecoder.decodeParamPamPam(threeHash);
}

//-----------------------------------------------------------------------------

QVariantHash DT645::how2logout(const QVariantHash &hashConstData, const QVariantHash &hashTmpData)
{
    Q_UNUSED(hashConstData);
    Q_UNUSED(hashTmpData);
    return QVariantHash();
}

//-----------------------------------------------------------------------------

QVariantHash DT645::getDoNotSleep(const quint8 &minutes)
{
    Q_UNUSED(minutes);
    return QVariantHash();
}

//-----------------------------------------------------------------------------

QVariantHash DT645::getGoToSleep(const quint8 &seconds)
{
    Q_UNUSED(seconds);
    return QVariantHash();
}

//-----------------------------------------------------------------------------

