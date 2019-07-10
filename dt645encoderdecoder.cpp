#include "dt645encoderdecoder.h"

#include "definedpollcodes.h"
#include "ucmetereventcodes.h"
#include "matildalimits.h"


///[!] type-converter
#include "src/base/prettyvalues.h"
#include "src/base/convertatype.h"






//-----------------------------------------------------------------------------

DT645EncoderDecoder::DT645EncoderDecoder(QObject *parent) : QObject(parent)
{

}

//-----------------------------------------------------------------------------

QByteArray DT645EncoderDecoder::getDefPasswd()
{
    return "00000000";//bcd
}

//-----------------------------------------------------------------------------

QByteArray DT645EncoderDecoder::bcdList2normal(const QList<quint8> &list)
{
    return bcd2normal(ConvertAtype::listUint8mid2arrMess(list, 0, -1));
}

//-----------------------------------------------------------------------------

QByteArray DT645EncoderDecoder::bcd2normal(const QByteArray &bcd)
{
    QByteArray out;
    for(int i = 0, imax = bcd.length(); i < imax; i++)
        out.append(QByteArray::number(bcd.mid(i,1).toHex().toUShort(0,16), 16).rightJustified(2,'0'));
    return out;
}


//-----------------------------------------------------------------------------


QByteArray DT645EncoderDecoder::arr2bcd(const QByteArray &arr)
{
    QByteArray out;
    for(int i = 0, imax = arr.length(); i < imax; i++)
        out.append(quint8(arr.at(i)));
    return out;
}

//-----------------------------------------------------------------------------

QByteArray DT645EncoderDecoder::arrRotate(const QByteArray &in)
{
    return arrRotateExt(in, 0x0);

}

//-----------------------------------------------------------------------------

QByteArray DT645EncoderDecoder::arrRotateExt(const quint64 &number, const int &maxHexLen, const qint8 &addbyte)
{
    QByteArray a;
    a.setNum(number, 16);
    a = a.rightJustified(maxHexLen, '0', true);
    return arrRotateExt(QByteArray::fromHex(a), addbyte);
}

//-----------------------------------------------------------------------------

QByteArray DT645EncoderDecoder::arrRotateExt(const QByteArray &in, const qint8 &addbyte)
{
    QByteArray out;
    for(int i = 0, imax = in.length(); i < imax; i++)
        out.prepend( qint8(in.at(i)) + addbyte );
    return out;
}

QByteArray DT645EncoderDecoder::addAbyte(const QByteArray &in, const qint8 &addbyte)
{
    QByteArray out;
    for(int i = 0, imax = in.length(); i < imax; i++)
        out.append( qint8(in.at(i)) + addbyte );
    return out;
}

//-----------------------------------------------------------------------------

QByteArray DT645EncoderDecoder::getBroadcastAddr()
{
    QByteArray ni;
    for(int i = 0; i < DLT645_BROADCASTADDRLEN; i++)
        ni.append(DLT645_BROADCASTADDRPART);
    return ni;
}
//-----------------------------------------------------------------------------

QByteArray DT645EncoderDecoder::getNormalMeterAddress(const QByteArray &ni)
{
    return arrRotate(arr2bcd(ni.rightJustified(DLT645_BROADCASTADDRLENNUMB, '0')));
}

//-----------------------------------------------------------------------------

QString DT645EncoderDecoder::enableDisableTheEnergyKey(const QString &oldvrsn, const QString &energy, const bool &isSupported)
{
//only for A+;A-;R+;R- and others
    if(oldvrsn.contains(energy))
        return oldvrsn;
    const QString energystate = isSupported ? energy.toUpper() : energy.toLower();
    QString vrsn = oldvrsn;
    if(vrsn.contains(energystate, Qt::CaseInsensitive)){
        vrsn.remove(energystate, Qt::CaseInsensitive);
    }
    vrsn.append(energystate);
    return vrsn;
}

//-----------------------------------------------------------------------------

QString DT645EncoderDecoder::setMeterPhaseCount(const QString &oldvrsn, const bool &isSinglePhaseMeter)
{
    if(isSinglePhase(oldvrsn) == isSinglePhaseMeter && !oldvrsn.mid(2,1).startsWith("_"))
        return oldvrsn;

    const int len = oldvrsn.length();
    if(len < 2)
        return oldvrsn;//there is no YY before

    QString vrsn = oldvrsn;
    const QString text = isSinglePhaseMeter ? "1" : "3";
    if(len == 2){
        vrsn.append(text);
    }else{
        vrsn.replace(2,1, text);
    }
    return vrsn;
}

//-----------------------------------------------------------------------------

QStringList DT645EncoderDecoder::getSupportedEnrg(const quint8 &code, const QString &version)
{
    /*
     *  version <YY><phase count><relay><<Energy letter><energy direction>>
     * - YY - protocol year (97 or 07) - always first two characters
     * - phase count - '1' || '3' || '_'  the third character
     * - relay - R - has realy, r - has no relay, '_' - unk
     * - <<Energy letter><energy direction>> - Energy letter {A||a,R||r||U||u||Q||q||P||p||I||i||COS_F||cos_f} - upper - supported, lower - not supported, '+' - to a consumer, '-' - from a consumer
     *
     * YY3RA+A-R+R-
     */
    if(!isPollCodeSupported(code))
        return QStringList();
    QStringList enrgs;

    const QString enrgpartofversion = version.mid(4);


    if(code == POLL_CODE_READ_VOLTAGE){
//        lk = isSinglePhase(version) ? QString("PA,IA,UA,cos_fA,F").split(',') :
//                                                   QString("UA,UB,UC,IA,IB,IC,PA,PB,PC,QA,QB,QC,cos_fA,cos_fB,cos_fC,F").split(',');

        const QStringList lk = QString("U,I,P,Q,cos_f").split(",");
        const QStringList phaselist = QString(isSinglePhase(version) ? "A" : "A;B;C").split(";");


        for(int i = 0, imax = lk.size(), jmax = phaselist.size(); i < imax; i++){
            if(enrgpartofversion.isEmpty() || !enrgpartofversion.contains(lk.at(i).toLower())){//ai - does not support A-
//                if(lk.at(i) == "F"){
//                    enrgs.append(lk.at(i));
//                }else{
                    for(int j = 0; j < jmax; j++)
                        enrgs.append(lk.at(i) + phaselist.at(j));
//                }
            }
        }
    }else{
        const QStringList lk = QString("A+;A-;R+;R-").split(";");
        for(int i = 0, imax = lk.size(); i < imax; i++){
            if(enrgpartofversion.isEmpty() || !enrgpartofversion.contains(lk.at(i).toLower()))//ai - does not support A-
                enrgs.append(lk.at(i));

        }
        if(!enrgs.contains("A+"))
            enrgs.prepend("A+");//I think all electricity meters can mesure A+


    }


     if(code == POLL_CODE_READ_VOLTAGE){
         //everything can be

     }else{//it can't be
         if(!enrgs.contains("A+"))
             enrgs.prepend("A+");//I think all electricity meters can mesure A+
     }

    return enrgs;

}

//-----------------------------------------------------------------------------

bool DT645EncoderDecoder::isPollCodeSupported(const quint16 &pollCode, QVariantHash &hashTmpData)
{
    if(hashTmpData.value("plgChecked").toBool())
        return true;

    const bool isSupp = isPollCodeSupported(pollCode);

    hashTmpData.insert("sprtdVls", isSupp);
    hashTmpData.insert("plgChecked", true);
    return isSupp;
}

//-----------------------------------------------------------------------------

bool DT645EncoderDecoder::isPollCodeSupported(const quint16 &pollCode)
{
    return (pollCode == POLL_CODE_READ_VOLTAGE || pollCode == POLL_CODE_READ_POWER || pollCode == POLL_CODE_READ_TOTAL || pollCode == POLL_CODE_READ_END_MONTH);
}

//-----------------------------------------------------------------------------

bool DT645EncoderDecoder::is1997version(const QString &version)
{
    return (version.isEmpty() || version.startsWith("97"));
}

//-----------------------------------------------------------------------------

bool DT645EncoderDecoder::isSinglePhase(const QString &version)
{
    return (!version.isEmpty() && version.mid(2,1).startsWith("1"));
}

//-----------------------------------------------------------------------------

QByteArray DT645EncoderDecoder::defPassword4meterVersion(const QString &version)
{
    return defPassword4meterVersion(is1997version(version));
}

//-----------------------------------------------------------------------------

QByteArray DT645EncoderDecoder::defPassword4meterVersion(const bool &is1997)
{
    return is1997 ? "1234" : "1234";
}
//-----------------------------------------------------------------------------
QBitArray DT645EncoderDecoder::getBitarrFromAbyteERR(const quint32 &errcode)
{
    /*
     * 0 - method data
     * 1 - data identifying error
     * 2 - wrong password
     * 3 - constant 0????
     * 4 - time zone over
     * 5 - days period
     * 6 - rate number
     * 7 - constant at 0
    */
    const quint8 err = quint8(errcode);
    const QBitArray arr = ConvertAtype::uint8ToBitArray(err);
    return arr;
}
//-----------------------------------------------------------------------------

bool DT645EncoderDecoder::hasNoData(const quint32 &errcode)
{
    return getBitarrFromAbyteERR(errcode).at(1);
}

bool DT645EncoderDecoder::hasWrongPassword(const quint32 &errcode)
{
    return getBitarrFromAbyteERR(errcode).at(2);

}
//-----------------------------------------------------------------------------

QString DT645EncoderDecoder::calcMeterAddr(const QString &meterSn)
{
    //only numbers from 0-999999999998
    bool ok;
    const quint64 addr = meterSn.toULongLong(&ok);
    return ok ?  QString::number(addr) : QString();
}

//-----------------------------------------------------------------------------
void DT645EncoderDecoder::insertDefaultHashMessageValues(QVariantHash &hashMessage)
{
    if(hashMessage.isEmpty())
        return;//there are no messages
    hashMessage.insert("quickCRC", true);
    hashMessage.insert("endSymb", QByteArray(1, quint8(DLT645_END_FRAME)));
    hashMessage.insert("data7EPt", false);
    hashMessage.insert("readLen", DLT645_MIN_FRAME_LEN);
}

//-----------------------------------------------------------------------------

QByteArray DT645EncoderDecoder::createAmessage(const QByteArray &ni, const quint8 &controlcode, const quint16 &command, const QByteArray &payload)
{
    QByteArray message;

    message.append(DLT645_START_FRAME);
    lastAddr = getMeterAddress(ni);
    message.append(lastAddr);
    message.append(DLT645_START_FRAME);
    message.append(controlcode);

    message.append(quint8(2 + payload.length()));//command is 2 bytes len

    message.append(arrRotateExt(command, 4, 0x33));
    if(!payload.isEmpty())
        message.append(addAbyte(arr2bcd(payload), 0x33));//just add, payload should be rotated before
    message.append(calcByteModulo(message));
    message.append(DLT645_END_FRAME);

    for(int i = 0; i < 4; i++)
        message.prepend(DLT645_WAKE_UP_BYTE);



    lastWas1997format = true;
    return message;

}

//-----------------------------------------------------------------------------

QByteArray DT645EncoderDecoder::getMeterAddress(const QByteArray &ni)
{
    return ni.isEmpty() ?
                getBroadcastAddr() :
                getNormalMeterAddress(ni);
}

//-----------------------------------------------------------------------------

QByteArray DT645EncoderDecoder::calcByteModulo(const QByteArray &messagepart)
{
    quint8 byte = 0x0;
    for(int i = 0, imax = messagepart.length(); i < imax; i++)
        byte += quint8(messagepart.at(i));
    return QByteArray(1, byte);
}

//-----------------------------------------------------------------------------

QByteArray DT645EncoderDecoder::meterNiFromConstHash(const QVariantHash &hashConstData)
{
    return (hashConstData.value("hardAddrsn", false).toBool()) ? hashConstData.value("NI").toByteArray() : QByteArray();
}

//-----------------------------------------------------------------------------

QByteArray DT645EncoderDecoder::meterPasswordFromConstHash(const QVariantHash &hashConstData)
{
    QByteArray passwd = hashConstData.value("passwd").toByteArray();
    if(passwd.isEmpty())
        passwd = getDefPasswd();
    return passwd;
}

//-----------------------------------------------------------------------------

QByteArray DT645EncoderDecoder::getReadMessage(const QByteArray &ni, const quint16 &command)
{
    return createAmessage(ni, DLT645_CNTR_MASTER_READ_DATA, command, QByteArray());
}

//-----------------------------------------------------------------------------

QByteArray DT645EncoderDecoder::getReadMessage(const QVariantHash &hashConstData, const quint16 &command)
{
    return getReadMessage(meterNiFromConstHash(hashConstData), command);

}

//-----------------------------------------------------------------------------

QByteArray DT645EncoderDecoder::getWriteMessage(const QByteArray &ni, const quint16 &command, const QByteArray &password, const QByteArray &writedatahex)
{

    return createAmessage(ni, DLT645_CNTR_MASTER_WRITE_DATA, command, QByteArray::fromHex(password) + arrRotate(QByteArray::fromHex(writedatahex)));
}

//-----------------------------------------------------------------------------

QByteArray DT645EncoderDecoder::getWriteMessage(const QVariantHash &hashConstData, const quint16 &command, const QByteArray &writedatahex)
{
    return getWriteMessage(meterNiFromConstHash(hashConstData), command, meterPasswordFromConstHash(hashConstData), writedatahex);
}

//-----------------------------------------------------------------------------

bool DT645EncoderDecoder::checkMessageBytes(const QByteArray &readArr, const QByteArray &lastAddr, const int &indxfrom, const QList<quint8> &bytes, const bool &toSlave, quint8 &errCode, QString &errstr, QByteArray &destination)
{

    errCode = DLT645_HAS_NO_ERRORS;
    destination.clear();
    if(bytes.isEmpty()){
        errstr = QString("no data, bytes.size=0");
        return false;
    }

    if(bytes.first() != DLT645_START_FRAME){
        errstr = QString("!startFrame");
        return false;
    }

    if(bytes.last() != DLT645_END_FRAME){
        errstr = QString("!endFrame");
        return false;
    }

    const int byteslen = bytes.size();
    if(byteslen < DLT645_MIN_FRAME_LEN){
        errstr = QString("bytes.size=%1, minlen=%2").arg(byteslen).arg(DLT645_MIN_FRAME_LEN);
        return false;
    }
    //68 95 32 06 00 00 00 68 81 06 53 C3 33 33 33 33 06 16
    if(bytes.at(7) != DLT645_START_FRAME){
        errstr = QString("!startFrame #2");
        return false;
    }

    if(!lastAddr.isEmpty() && lastAddr != getBroadcastAddr() && readArr.mid(indxfrom+1).startsWith(lastAddr)){
        errstr = QString("address '%1' != '%2'").arg(QString(readArr.mid(indxfrom+1, lastAddr.length()).toHex())).arg(QString(lastAddr.toHex()));
        return false;
    }

    quint8 controlbyte = bytes.at(8);

    const QBitArray bitarr = ConvertAtype::uint8ToBitArray(controlbyte);
    if( toSlave != bitarr.at(7)){
        errstr = QString("!response");
        return false;
    }

    if(bitarr.at(6)){
        errstr = QString("abnormal response");
        errCode = DLT645_HAS_ABNORMAL_ERROR;
//        return false;//ignore
    }

    if(bitarr.at(5)){
        qDebug() << "has subsequent data";

    }
//to remove 3 unuseful bits
    controlbyte = controlbyte << 3;
    controlbyte = controlbyte >> 3;

    if(controlbyte != DLT645_CNTR_MASTER_READ_DATA && controlbyte != DLT645_CNTR_MASTER_WRITE_DATA){
        errstr = QString("bad control byte 0x%1").arg(QString::number(controlbyte, 16));
        return false;
    }

    destination = QByteArray::number(bcd2normal(readArr.mid(indxfrom+1,6)).toULongLong());
    if(destination == getBroadcastAddr())
        destination.clear();


    return true;
}

//-----------------------------------------------------------------------------

quint32 DT645EncoderDecoder::getCommandFromAmessage(const QList<quint8> &bytes, const bool &hasAnError)
{
    QByteArray command;
    command.append(bytes.at(10));
    if(!hasAnError)
        command.append(bytes.at(11));
    const QByteArray out = arrRotateExt(command, -0x33);

    const quint32 result = out.toHex().toULong(0,16);

    if(out.isEmpty() || result < 1){
        qDebug() << "getCommandFromAmessage " << command.toHex() << out.toHex();
    }

    return result;

}

//-----------------------------------------------------------------------------

DT645EncoderDecoder::MessageValidatorResult DT645EncoderDecoder::messageIsValid(const QByteArray &readArr, const QByteArray &lastAddr)
{
    QByteArray dest;
    return messageIsValidExt(readArr, lastAddr, true, dest);
}

//-----------------------------------------------------------------------------

DT645EncoderDecoder::MessageValidatorResult DT645EncoderDecoder::messageIsValidExt(const QByteArray &readArr, const QByteArray &lastAddr, const bool &toSlave, QByteArray &destination)
{
    MessageValidatorResult decoderesult = MessageValidatorResult();//reset all

    const int indxfrom = readArr.indexOf(quint8(DLT645_START_FRAME));
    if(indxfrom < 0){
        decoderesult.errstr = QString("no data, indxFrom=%1").arg(indxfrom);
        return decoderesult;
    }

    QList<quint8> bytes;
    for(int i = indxfrom, imax = readArr.size(); i < imax; i++)
        bytes.append(quint8(readArr.at(i)));

    if(!checkMessageBytes(readArr, lastAddr, indxfrom, bytes, toSlave, decoderesult.errCode, decoderesult.errstr, destination))
        return decoderesult;

    //68 95 32 06 00 00 00 68 81 06 53 C3 33 33 33 33 06 16

    const bool hasAnError = (decoderesult.errCode != DLT645_HAS_NO_ERRORS);
    decoderesult.commandCode = getCommandFromAmessage(bytes, hasAnError);
    decoderesult.isValid = true;//now it is ok

    for(int i = 12, imax = bytes.size() - 2; i < imax; i++)//
        decoderesult.listMeterMess.prepend(bytes.at(i) - 0x33);

    return decoderesult;
}

//-----------------------------------------------------------------------------

Mess2meterRezult DT645EncoderDecoder::mess2meter(const Mess2meterArgs &pairAboutMeter)
{
    const QVariantHash hashConstData = pairAboutMeter.hashConstData;
    QVariantHash hashTmpData = pairAboutMeter.hashTmpData;
    QVariantHash hashMessage;

    const quint8 pollCode = hashConstData.value("pollCode").toUInt();
    if(!isPollCodeSupported(pollCode, hashTmpData))
        return Mess2meterRezult(hashMessage,hashTmpData);


    if(hashTmpData.value("vrsn").toString().isEmpty() && !hashConstData.value("vrsn").toString().isEmpty())
        hashTmpData.insert("vrsn", hashConstData.value("vrsn"));

    bool go2exit = false;
    verboseMode = hashConstData.value("verboseMode").toBool();


    quint16 step = hashTmpData.value("step", (quint16)0).toUInt();

    if(verboseMode) qDebug() << "mess2Meter " << step << pollCode ;


    if(!hashTmpData.value("logined", false).toBool()){
        ;//nothing I haven't got a password
//        go2exit = true;
    }
    if(!go2exit && hashTmpData.value("corrDateTime", false).toBool()){
      ;//nothing I haven't got a password
//        go2exit = true;
    }

    /*
     * only for this plugin
     * hashTmpData key
     * corrDateTime     - QString   - korektsiya daty chasu
     * logined          - bool      - (false) if(true) logined good
     * sprtdVls         - bool      - (false) if(true) not supported values = '-'
     * DLT_DTgood      - bool      - (false) if(true) dateTime not read
     *
     *
*/

    if(!go2exit && step < 10 && pollCode > 0){
        switch(step){
        case 1  : hashMessage.insert("message_0", getReadMessage(hashConstData, DLT645_DATE))       ; break; //read date
        case 2  : hashMessage.insert("message_0", getReadMessage(hashConstData, DLT645_TIME))       ; hashTmpData.insert("DLT_timesend", QDateTime::currentDateTimeUtc()); break; //read time
        default : hashMessage.insert("message_0", getReadMessage(hashConstData, DLT645_METER_SN))   ; step = 0; break;//read S/N
        }
        go2exit = true;
    }



    if(!go2exit){
        switch (pollCode) {
//      case POLL_CODE_READ_METER_LOGBOOK   : hashMessage = preparyMeterJournal(hashConstData, hashTmpData, step); break; is not supported
        case POLL_CODE_READ_VOLTAGE         : hashMessage = preparyVoltage(hashConstData, hashTmpData)          ; break;
        case POLL_CODE_READ_POWER           : hashMessage = preparyPower(hashConstData, hashTmpData, step)      ; break;
        case POLL_CODE_READ_TOTAL           : hashMessage = preparyTotalEnrg(hashConstData, hashTmpData)        ; break;
//      case POLL_CODE_READ_END_DAY         : hashMessage = preparyEoD(hashConstData, hashTmpData, step)        ; break; is not supported
        case POLL_CODE_READ_END_MONTH       : hashMessage = preparyEoM(hashConstData, hashTmpData,  step)       ; break;
        default:{ if(verboseMode) qDebug() << "DLT645 pollCode is not supported 1" << pollCode                  ; break;}
        }
        if(hashMessage.isEmpty())
            step = 0xFFFF;
    }

    hashTmpData.insert("step", step);
    if(hashTmpData.value("step").toInt() >= 0xFFFE)
        hashMessage.clear();
    else
        insertDefaultHashMessageValues(hashMessage);

    return Mess2meterRezult(hashMessage,hashTmpData);


}

//-----------------------------------------------------------------------------

QVariantHash DT645EncoderDecoder::decodeMeterData(const DecodeMeterMess &threeHash)
{
    const QVariantHash hashConstData = threeHash.hashConstData;
    const QVariantHash hashRead = threeHash.hashRead;
    QVariantHash hashTmpData = threeHash.hashTmpData;


    if(verboseMode){
        foreach (QString key, hashRead.keys()) {
            qDebug() << "DLT read "  << key << hashRead.value(key).toByteArray().toHex().toUpper();
        }
    }

    hashTmpData.insert("messFail", true);
    quint8 pollCode         = hashConstData.value("pollCode").toUInt();
    quint16 step            = hashTmpData.value("step", (quint16)0).toUInt();
    ErrCounters errwarns   = ErrCounters(qMax(0, hashTmpData.value("warning_counter", 0).toInt()), qMax(0, hashTmpData.value("error_counter", 0).toInt()));


    const MessageValidatorResult decoderesult = messageIsValid(hashRead.value("readArr_0").toByteArray(), lastAddr);
    if(!decoderesult.isValid){
        QString warn;
        QString err = decoderesult.errstr;
        hashTmpData.insert(MeterPluginHelper::errWarnKey(errwarns.error_counter, true), MeterPluginHelper::prettyMess(tr("incomming data is invalid"),
                                                                                                             PrettyValues::prettyHexDump( hashRead.value("readArr_0").toByteArray().toHex(), "", 0)
                                                                                                             , err, warn, true));
        pollCode = 0;
    }else{
        if(hashTmpData.value("vrsn").toString().isEmpty())
            hashTmpData.insert("vrsn",lastWas1997format ? "97__" : "07__");//phase and relay unknown count

    }



//    if(pollCode > 0 && hashTmpData.value("corrDateTime", false).toBool()){
//        if(errCode == ERR_WRITE_DONE){
//            hashTmpData.insert("corrDateTime", false);
//            if(verboseMode) qDebug() << "DLMS date good" ;

//            hashTmpData.insert(MeterPluginHelper::nextMatildaEvntName(hashTmpData), MeterPluginHelper::addEvnt2hash(ZBR_EVENT_DATETIME_CORR_DONE, QDateTime::currentDateTimeUtc(),
//                                                                                                                    tr("Meter new date %1 UTC%2%3")
//                                                                                                                    .arg(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"))
//                                                                                                                    .arg( (QDateTime::currentDateTime().offsetFromUtc() >= 0) ? "+" : "" )
//                                                                                                                    .arg(QString::number(QDateTime::currentDateTime().offsetFromUtc()))) );
//        }else{
//            hashTmpData.insert(MeterPluginHelper::nextMatildaEvntName(hashTmpData), MeterPluginHelper::addEvnt2hash(ZBR_EVENT_DATETIME_NOT_CORR, QDateTime::currentDateTimeUtc(),
//                                                                                                                    tr("Correct date: fail") ));
//            hashTmpData.insert(MeterPluginHelper::errWarnKey(error_counter, true), MeterPluginHelper::prettyMess(tr("Correct date: fail"), "", lastErrS.lastErrorStr, lastErrS.lastWarning, true));

//            if(verboseMode) qDebug() << "can't correct date time " << errCode;
//        }
////        hashTmpData.remove("DLMS_dt_sn_vrsn_ready");
//        pollCode = 0;
//    }

    if(step < 10 && pollCode > 0){
        bool result = false;
        bool getDtAgain = false;
        bool stopPoll = false;

        switch(step){
        case 1  : result = decodeMeterDate(decoderesult, hashTmpData); break; //read date
        case 2  : result = decodeMeterTime(decoderesult, hashConstData, hashTmpData, errwarns, getDtAgain, stopPoll ); break;//read time
        default : result = decodeMeterSN(decoderesult, hashTmpData); step = 0; break;//read S/N
        }
        if(result){


            hashTmpData.insert("messFail", false);
            if(stopPoll){
                step = 0xFFFF;
            }else {
                if(getDtAgain){
                    step = 1;
                }else{
                    step++;
                    if(step > 2)
                        step = 10;//go to the poll section
                }
            }

        }

        pollCode = 0;
    }



    QVariantHash decodehash;
    //const QList<quint8> &listMeterMess, const quint32 &commandCode, const quint8 &errCode, const QVariantHash &hashConstData, const QVariantHash &hashTmpData, quint16 &step, ErrCounters &warnerr
    switch(pollCode){
//  case POLL_CODE_READ_METER_LOGBOOK   : decodehash = fullMeterJrnl(decoderesult, hashConstData, hashTmpData, step, errwarns); break;
    case POLL_CODE_READ_VOLTAGE         : decodehash = fullVoltage(  decoderesult, hashTmpData, step, errwarns); break;
    case POLL_CODE_READ_POWER           : decodehash = fullPower(    decoderesult, hashConstData, hashTmpData, step, errwarns); break;
    case POLL_CODE_READ_TOTAL           : decodehash = fullTotalEnrg(decoderesult, hashConstData, hashTmpData, errwarns); break;
//  case POLL_CODE_READ_END_DAY         : decodehash = fullEoD(      decoderesult, hashConstData, hashTmpData, step, errwarns); break;
    case POLL_CODE_READ_END_MONTH       : decodehash = fullEoM(      decoderesult, hashConstData, hashTmpData, step, errwarns); break;
    }


    return decodeEnd(decodehash, errwarns, step, hashTmpData);
}

//-----------------------------------------------------------------------------

Mess2meterRezult DT645EncoderDecoder::messParamPamPam(const Mess2meterArgs &pairAboutMeter)
{
    const QVariantHash hashConstData = pairAboutMeter.hashConstData;
    QVariantHash hashTmpData = pairAboutMeter.hashTmpData;
    QVariantHash hashMessage;


    const quint8 pollCode = hashConstData.value("pollCode").toUInt();

    quint16 step = hashTmpData.value("step", (quint16)0).toUInt();

    if(verboseMode) qDebug() << "mess2Meter " << step << pollCode ;

    switch(pollCode){

    case POLL_CODE_READ_DATE_TIME_DST   :{
        if(step < 1)
            step = 1;
        switch(step){
        case 1  : hashMessage.insert("message_0", getReadMessage(hashConstData, DLT645_DATE))       ; break; //read date
        case 2  : hashMessage.insert("message_0", getReadMessage(hashConstData, DLT645_TIME))       ; hashTmpData.insert("DLT_timesend", QDateTime::currentDateTimeUtc()); break; //read time
        default : hashMessage.insert("message_0", getReadMessage(hashConstData, DLT645_METER_SN))   ; step = 0; break;//read S/N
        }
        break;}//       52

    case POLL_CODE_WRITE_DATE_TIME      :{

        const QDateTime currdt = QDateTime::currentDateTime().addSecs(3);
        switch(step){
        case 1  : hashMessage.insert("message_0", getWriteMessage(hashConstData, DLT645_TIME, currdt.toString("hhmmss").toLocal8Bit()) ); break; //write time
        case 2  : hashMessage.insert("message_0", getReadMessage(hashConstData, DLT645_DATE))       ; break; //read date
        case 3  : hashMessage.insert("message_0", getReadMessage(hashConstData, DLT645_TIME))       ; hashTmpData.insert("DLT_timesend", QDateTime::currentDateTimeUtc()); break; //read time
        default : hashMessage.insert("message_0", getWriteMessage(hashConstData, DLT645_DATE, QString(currdt.toString("yyMMdd") + "0" + QString::number(currdt.date().dayOfWeek())).toLocal8Bit()))   ; step = 0; break;//write date
        }
        break;}//           53

    default:{
        hashTmpData.insert("notsup", true);
//        hashTmpData.insert("notsupasdone", true);
        step = 0xFFFF;
        break;}
    }


    hashTmpData.insert("step", step);
    if(hashTmpData.value("step").toInt() >= 0xFFFE)
        hashMessage.clear();
    else
        insertDefaultHashMessageValues(hashMessage);

    return Mess2meterRezult(hashMessage,hashTmpData);
}

//-----------------------------------------------------------------------------

QVariantHash DT645EncoderDecoder::decodeParamPamPam(const DecodeMeterMess &threeHash)
{
    const QVariantHash hashConstData = threeHash.hashConstData;
    const QVariantHash hashRead = threeHash.hashRead;
    QVariantHash hashTmpData = threeHash.hashTmpData;


    if(verboseMode){
        foreach (QString key, hashRead.keys()) {
            qDebug() << "DLT read "  << key << hashRead.value(key).toByteArray().toHex().toUpper();
        }
    }

    hashTmpData.insert("messFail", true);
    quint8 pollCode         = hashConstData.value("pollCode").toUInt();
    quint16 step            = hashTmpData.value("step", (quint16)0).toUInt();
    ErrCounters errwarns   = ErrCounters(qMax(0, hashTmpData.value("warning_counter", 0).toInt()), qMax(0, hashTmpData.value("error_counter", 0).toInt()));

    const MessageValidatorResult decoderesult = messageIsValid(hashRead.value("readArr_0").toByteArray(), lastAddr);
    if(!decoderesult.isValid){
        QString warn;
        QString err = decoderesult.errstr;
        hashTmpData.insert(MeterPluginHelper::errWarnKey(errwarns.error_counter, true), MeterPluginHelper::prettyMess(tr("incomming data is invalid"),
                                                                                                             PrettyValues::prettyHexDump( hashRead.value("readArr_0").toByteArray().toHex(), "", 0)
                                                                                                             , err, warn, true));
        pollCode = 0;
    }else{
        if(hashTmpData.value("vrsn").toString().isEmpty())
            hashTmpData.insert("vrsn",lastWas1997format ? "97__" : "07__");//phase and relay unknown count

    }


    QVariantHash decodehash;


    switch(pollCode){

    case POLL_CODE_READ_DATE_TIME_DST   :{
        bool result = false;
        bool getDtAgain = false;
        bool stopPoll = false;//it is unused here

        if(step < 0)
            step = 1;
        switch(step){
        case 1  : result = decodeMeterDate(decoderesult, hashTmpData); break; //read date
        case 2  : result = decodeMeterTime(decoderesult, hashConstData, hashTmpData, errwarns, getDtAgain, stopPoll ); break;//read time
        default : result = decodeMeterSN(decoderesult, hashTmpData); step = 0; break;//read S/N
        }
        if(result){

            hashTmpData.insert("messFail", false);
            hashTmpData.insert("DST_Profile", false);//The protocol is from China, there is no DST there

            if(getDtAgain){
                step = 1;
            }else{
                step++;
                if(step > 2)
                    step = 0xFFFF;//go to the exit
            }


        }

        break;}//       52

    case POLL_CODE_WRITE_DATE_TIME      :{

        bool result = false;
        bool getDtAgain = false;
        bool stopPoll = false;//it is unused here

        bool isPasswordBad = false;

        switch(step){
        case 1  :  result = isWriteGood(decoderesult, hashTmpData, errwarns, isPasswordBad);  break;//read S/N
        case 2  : result = decodeMeterDate(decoderesult, hashTmpData); break; //read date
        case 3  : result = decodeMeterTime(decoderesult, hashConstData, hashTmpData, errwarns, getDtAgain, stopPoll ); break;//read time
        default : result = isWriteGood(decoderesult, hashTmpData, errwarns, isPasswordBad); step = 0; break;//read S/N
        }

        if(isPasswordBad){
            hashTmpData.insert("messFail", false);
            step = 0xFFFF;
            break;
        }

        if(result){

            hashTmpData.insert("messFail", false);
            hashTmpData.insert("DST_Profile", false);//The protocol is from China, there is no DST there

            if(getDtAgain){
                step = 2;
            }else{
                step++;
                if(step > 3)
                    step = 0xFFFF;//go to the exit
            }


        }


        break;}//           53

    default:{
        hashTmpData.insert("notsup", true);
//        hashTmpData.insert("notsupasdone", true);
        hashTmpData.insert("step", 0xFFFF);
        break;}
    }
    return decodeEnd(decodehash, errwarns, step, hashTmpData);

}

//-----------------------------------------------------------------------------

QVariantHash DT645EncoderDecoder::decodeEnd(const QVariantHash &decodehash, const ErrCounters &errwarns, quint16 &step, QVariantHash &hashTmpData)
{

    if(!decodehash.isEmpty()){
        MeterPluginHelper::copyHash2hash(decodehash, hashTmpData);

        if(!hashTmpData.value("messFail").toBool() && hashTmpData.value("currEnrg", 0).toInt() > 3){
            hashTmpData.insert("currEnrg", 5);
            step = 0xFFFF;
        }
    }

    if(verboseMode && errwarns.error_counter > 0){
        const ErrCounters errwarnsmirror   = ErrCounters(qMax(0, hashTmpData.value("warning_counter", 0).toInt()), qMax(0, hashTmpData.value("error_counter", 0).toInt()));
        if(errwarns.error_counter > errwarnsmirror.error_counter){
            for(int i = 0; i < errwarns.error_counter && verboseMode; i++)
                qDebug() << "DLT645 error " << hashTmpData.value(QString("Error_%1").arg(i));
        }


    }
    hashTmpData.insert("step", step);
    hashTmpData.insert("error_counter", errwarns.error_counter);
    hashTmpData.insert("warning_counter", errwarns.warning_counter);

    return hashTmpData;
}

//-----------------------------------------------------------------------------

bool DT645EncoderDecoder::isWriteGood(const DT645EncoderDecoder::MessageValidatorResult &decoderesult, QVariantHash &hashTmpData, ErrCounters &errwarns, bool &isPasswordBad)
{
    isPasswordBad = false;
     if( decoderesult.errCode != DLT645_HAS_NO_ERRORS){


         //const QString &mess, const QString &hexDump, const bool &isErr, ErrsStrct &errwarn)
         hashTmpData.insert(MeterPluginHelper::errWarnKey(errwarns.error_counter, true), MeterPluginHelper::prettyMess(tr("Write operation was failed"), "", true, lasterrwarn));

         if(hasWrongPassword(decoderesult.commandCode)){
             isPasswordBad = true;
             hashTmpData.insert(MeterPluginHelper::errWarnKey(errwarns.error_counter, true), MeterPluginHelper::prettyMess(tr("Password is wrong"), "", true, lasterrwarn));
         }
         return false;
     }
     return true;
}

//-----------------------------------------------------------------------------

bool DT645EncoderDecoder::decodeMeterSN(const DT645EncoderDecoder::MessageValidatorResult &decoderesult, QVariantHash &hashTmpData)
{
    if(decoderesult.commandCode == DLT645_METER_SN){
        const QString sn = bcdList2normal(decoderesult.listMeterMess);
        if(!sn.isEmpty()){
            hashTmpData.insert("SN", sn);
            return true;
        }
        if(verboseMode)
            qDebug() << "DLT sn.isEmpty " << sn << decoderesult.listMeterMess << decoderesult.commandCode;

    }

    return false;
}

//-----------------------------------------------------------------------------

bool DT645EncoderDecoder::decodeMeterDate(const DT645EncoderDecoder::MessageValidatorResult &decoderesult, QVariantHash &hashTmpData)
{
    if(decoderesult.commandCode == DLT645_DATE){

        const QString dtstr = bcdList2normal(decoderesult.listMeterMess);
        hashTmpData.insert("DLT_datesaved", QDateTime::currentDateTimeUtc().addSecs(-1));

        if(!dtstr.isEmpty()){//yyMMddww
            const QDate date = QDate::fromString(QDate::currentDate().toString("yyyy").left(2) + dtstr.left(6), "yyyyMMdd");
            if(date.isValid()){

                hashTmpData.insert("DLT_datePart", date);
                return true;
            }
        }
    }

    return false;
}

//-----------------------------------------------------------------------------

bool DT645EncoderDecoder::decodeMeterTime(const DT645EncoderDecoder::MessageValidatorResult &decoderesult, const QVariantHash &hashConstData, QVariantHash &hashTmpData, ErrCounters &warnerr,
                                          bool &getDateAgain, bool &stopPoll)
{
    getDateAgain = false;
    if(decoderesult.commandCode == DLT645_TIME){

        const QDateTime currdt = QDateTime::currentDateTime();
//        const QDateTime senddtutc = hashTmpData.value("DLT_timesend", currdt.toUTC()).toDateTime();
        const QDateTime datesaved = hashTmpData.value("DLT_datesaved", currdt.toUTC()).toDateTime();

        const QString timestr = bcdList2normal(decoderesult.listMeterMess);

        if(!timestr.isEmpty()){//hhmmss
            const QTime time = QTime::fromString(timestr, "hhmmss");

            QDateTime meterDateTime = QDateTime(hashTmpData.value("DLT_datePart", currdt.date()).toDate(), time, Qt::UTC);

            if(datesaved.toLocalTime().date().day() != currdt.date().day()){
                getDateAgain = true;//date changed, so it is necessary to read meter date again
                return true;
            }

            meterDateTime.setOffsetFromUtc(currdt.offsetFromUtc());

            ErrsStrct lastErrS;
            QString err, warn, mtdEvnt;
            const bool allowExchange = MeterPluginHelper::getCorrDateTime(hashTmpData, hashConstData, meterDateTime, err, warn, mtdEvnt);

            if(!err.isEmpty())
                hashTmpData.insert(  MeterPluginHelper::errWarnKey(warnerr.error_counter, true),
                                     MeterPluginHelper::prettyMess(err,  PrettyValues::prettyHexDump(decoderesult.listMeterMess, decoderesult.commandCode), true, lastErrS));

            if(!warn.isEmpty())
                hashTmpData.insert(  MeterPluginHelper::errWarnKey(warnerr.warning_counter, false),
                                     MeterPluginHelper::prettyMess(warn,  PrettyValues::prettyHexDump(decoderesult.listMeterMess, decoderesult.commandCode), false, lastErrS));

            if(!mtdEvnt.isEmpty())
                hashTmpData.insert(  MeterPluginHelper::nextMatildaEvntName(hashTmpData),
                                     MeterPluginHelper::addEvnt2hash(ZBR_EVENT_DATETIME_NEED2CORR, QDateTime::currentDateTimeUtc(),
                                                                                                                          mtdEvnt) );

            if(!allowExchange)
                stopPoll = true;

            return true;
        }

    }

    return false;
}

//-----------------------------------------------------------------------------

quint8 DT645EncoderDecoder::getValidDltStep(const quint8 &startFrom, const QString &version, QStringList &unsupportedKeys)
{
    quint8 dltStep = startFrom;
    if(dltStep > 0){

        const QStringList l = getSupportedEnrg(POLL_CODE_READ_VOLTAGE, version);
        for(int i = startFrom; i < 16; i++){
            const VolateMessageParam enrgsett = getCommandAndEnrgLetter4dltStep(dltStep);
            if(enrgsett.enrgkey.isEmpty())
                return 15;//break all ;

            if(l.contains(enrgsett.enrgkey)){
                return dltStep;
            }
            unsupportedKeys.append(enrgsett.enrgkey);
            dltStep++;
        }
    }
    return startFrom;
}

//-----------------------------------------------------------------------------

QVariantHash DT645EncoderDecoder::preparyVoltageResultHash(QVariantHash &resulthash, quint8 &dltStep, quint16 &step)
{
    QStringList unsupp;
    dltStep = getValidDltStep(dltStep+1, resulthash.value("vrsn").toString(), unsupp);
    if(dltStep > 14)
        step = 0xFFFF;

    for(int i = 0, imax = unsupp.size(); i < imax; i++)
        resulthash.insert(unsupp.at(i), "!");
    resulthash.insert("F", "!");
    resulthash.insert("messFail", false);//go to the next energy
    resulthash.insert("DLT_step", dltStep);


    return resulthash;
}

//-----------------------------------------------------------------------------

DT645EncoderDecoder::VolateMessageParam DT645EncoderDecoder::getCommandAndEnrgLetter4dltStep(const quint8 &dltStep)
{
    quint16 messagecommand = 0;
    QString enrgletter;
    qreal multiplication = 1.0;
    switch(dltStep){
    case  0: messagecommand = DLT645_VOLTAGE_A       ; enrgletter = "UA"    ; multiplication = 1.0  ; break;//         0xB611
    case  1: messagecommand = DLT645_VOLTAGE_B       ; enrgletter = "UB"    ; multiplication = 1.0  ; break;//         0xB612
    case  2: messagecommand = DLT645_VOLTAGE_C       ; enrgletter = "UC"    ; multiplication = 1.0  ; break;//         0xB613

    case  3: messagecommand = DLT645_CURRENT_A       ; enrgletter = "IA"    ; multiplication = 0.01 ; break;//        0xB621
    case  4: messagecommand = DLT645_CURRENT_B       ; enrgletter = "IB"    ; multiplication = 0.01 ; break;//        0xB622
    case  5: messagecommand = DLT645_CURRENT_C       ; enrgletter = "IC"    ; multiplication = 0.01 ; break;//        0xB623

    case  6: messagecommand = DLT645_POWER_ACTIVE_A  ; enrgletter = "PA"    ; multiplication = 0.0001 ; break;//        0xB631
    case  7: messagecommand = DLT645_POWER_ACTIVE_B  ; enrgletter = "PB"    ; multiplication = 0.0001 ; break;//        0xB632
    case  8: messagecommand = DLT645_POWER_ACTIVE_C  ; enrgletter = "PC"    ; multiplication = 0.0001 ; break;//        0xB633

        //reactive power has my own multiplication, because the one from the document is wrong
    case  9: messagecommand = DLT645_POWER_REACTIVE_A; enrgletter = "QA"    ; multiplication = 0.0001 ; break;//       0xB641
    case 10: messagecommand = DLT645_POWER_REACTIVE_B; enrgletter = "QB"    ; multiplication = 0.0001 ; break;//       0xB642
    case 11: messagecommand = DLT645_POWER_REACTIVE_C; enrgletter = "QC"    ; multiplication = 0.0001 ; break;//       0xB643

    case 12: messagecommand = DLT645_POWER_FACTOR_A  ; enrgletter = "cos_fA"; multiplication = 0.001; break;//       0xB651
    case 13: messagecommand = DLT645_POWER_FACTOR_B  ; enrgletter = "cos_fB"; multiplication = 0.001; break;//       0xB652
    case 14: messagecommand = DLT645_POWER_FACTOR_C  ; enrgletter = "cos_fC"; multiplication = 0.001; break;//       0xB653
//    default: messagecommand = DLT645_VOLTAGE_A; enrgletter = "UA"; break;//
    }
    VolateMessageParam param;
    param.command = messagecommand;
    param.enrgkey = enrgletter;
    param.multiplication = multiplication;
    return param;
}


//-----------------------------------------------------------------------------

QVariantHash DT645EncoderDecoder::preparyVoltage(const QVariantHash &hashConstData, QVariantHash &hashTmpData)
{
    QVariantHash hashMessage;

    QStringList unsupp;
    const quint8 dltStep = getValidDltStep( hashTmpData.value("DLT_step", 0).toUInt(), hashTmpData.value("vrsn").toString(), unsupp);
    const quint16 messagecommand = getCommandAndEnrgLetter4dltStep(dltStep).command;

     if(messagecommand > 0){
         hashMessage.insert("message_0", getReadMessage(hashConstData,messagecommand));
     }

    return hashMessage;
}

//-----------------------------------------------------------------------------

QVariantHash DT645EncoderDecoder::fullVoltage(const MessageValidatorResult &decoderesult, const QVariantHash &hashTmpData, quint16 &step, ErrCounters &warnerr)
{
    QVariantHash resulthash;

    quint8 dltStep = hashTmpData.value("DLT_step", 0).toUInt();

    const VolateMessageParam voltaparam = getCommandAndEnrgLetter4dltStep(dltStep);
    QString currEnrgLetter = voltaparam.enrgkey;
    currEnrgLetter.chop(1);

    const QString vrsn = hashTmpData.value("vrsn").toString();
    if(decoderesult.listMeterMess.isEmpty() || decoderesult.errCode != DLT645_HAS_NO_ERRORS){
        if(hasNoData(decoderesult.commandCode)){ //add something 2 vrsn
//            insertNotSupValues2hash("", energyletter, trff, resulthash);
            const QStringList phases = QString("A;B;C").split(";");
            const int startFrom = dltStep%3;
            for(int i = startFrom, imax = phases.size(); i < imax; i++)
                resulthash.insert(QString("%1%2").arg(currEnrgLetter).arg(phases.at(i)), "!");



            if(startFrom == 0)//energy is not supp
                resulthash.insert("vrsn", enableDisableTheEnergyKey(vrsn, currEnrgLetter, false));
            else if(!isSinglePhase(vrsn))
                resulthash.insert("vrsn", setMeterPhaseCount(vrsn, true));//mark as a single phase meter


            return preparyVoltageResultHash(resulthash, dltStep, step);


        }
        return resulthash;

    }

    if((dltStep%3) == 0)//can't determine phase count
        resulthash.insert("vrsn", enableDisableTheEnergyKey(vrsn, currEnrgLetter, true));
    else
        resulthash.insert("vrsn", setMeterPhaseCount(vrsn, false));//mark as a 3 phase meter


    const QString valstr = bcdList2normal(decoderesult.listMeterMess);
    bool ok;
    const qreal valreal = valstr.toDouble(&ok) * voltaparam.multiplication;

    if(!ok){
        return resulthash;
    }
    const QString valStr = PrettyValues::prettyNumber(valreal, 4, 4);
    resulthash.insert(voltaparam.enrgkey, valStr);

   return preparyVoltageResultHash(resulthash, dltStep, step);
}

//-----------------------------------------------------------------------------

QVariantHash DT645EncoderDecoder::preparyPower(const QVariantHash &hashConstData, QVariantHash &hashTmpData, quint16 &step)
{
    QVariantHash hashMessage;
    return hashMessage;

}

//-----------------------------------------------------------------------------

QVariantHash DT645EncoderDecoder::fullPower(const MessageValidatorResult &decoderesult, const QVariantHash &hashConstData, const QVariantHash &hashTmpData, quint16 &step, ErrCounters &warnerr)
{
    QVariantHash resulthash;
    resulthash.insert("messFail", true);
    return resulthash;

}

//-----------------------------------------------------------------------------

void DT645EncoderDecoder::preparyTariffResultHash(const QString &prefics, const QList<quint8> &listMeterMess, const QVariantHash &hashTmpData, const int &trff, const QString &energyletter, QVariantHash &resulthash)
{

    const QString fullprefics = prefics.isEmpty() ? QString() : QString("%1_").arg(prefics);

    //from the maximum tariff to the lowerest
    QStringList tariffdata;
    for(int i = 0, imax = listMeterMess.size(), t = 0; i < imax ; i += 4, t++){
        const QString kwhstr = bcdList2normal(listMeterMess.mid(i,4));
        bool ok;
        const qreal kwhreal = kwhstr.toDouble(&ok) * 0.01;

        if(!ok)
            return ;

        const QString valStr = PrettyValues::prettyNumber(kwhreal, 2, 2);

        tariffdata.prepend(valStr);
    }

    for(int i = 0, imax = tariffdata.size(); i < imax && i <= trff; i++)
        resulthash.insert(QString("%1T%2_%3").arg(fullprefics).arg(i).arg(energyletter), tariffdata.at(i));

    resulthash.insert("messFail", false);//go to the next energy
    resulthash.insert("currEnrg", hashTmpData.value("DLT_currEnrg"));

}

//-----------------------------------------------------------------------------

QVariantHash DT645EncoderDecoder::preparyTotalEnrg(const QVariantHash &hashConstData, QVariantHash &hashTmpData)
{
    QVariantHash hashMessage;
    const QString version = hashTmpData.value("vrsn").toString();

    int currEnrg = qMax(0, hashTmpData.value("currEnrg", 0).toInt());
    const QStringList listPlgEnrg = getSupportedEnrg(POLL_CODE_READ_TOTAL, version);

    const QStringList energies = QString("A+;A-;R+;R-").split(";");
    const QStringList listEnrg = hashConstData.value("listEnrg").toStringList();

    quint32 messagecommand = 0;
    QString enrgletter;
    for(int imax = energies.size(); currEnrg < imax ; currEnrg++){
        const QString energy = energies.at(currEnrg);
        if(!listPlgEnrg.contains(energy) || !listEnrg.contains(energy))
            continue;

        if(messagecommand < 1){
            switch(currEnrg){
            case 0: messagecommand = DLT645_TOTAL_A_POSITIVE; break;
            case 1: messagecommand = DLT645_TOTAL_A_REVERSE ; break;
            case 2: messagecommand = DLT645_TOTAL_R_POSITIVE; break;
            case 3: messagecommand = DLT645_TOTAL_R_REVERSE ; break;
            }
            enrgletter = energy;
        }else{
            break;//to get the next index
        }
    }

    if(messagecommand > 0){
        hashTmpData.insert("DLT_currEnrgLetter", enrgletter);
        hashTmpData.insert("DLT_currEnrg", currEnrg);

        hashMessage.insert("message_0", getReadMessage(hashConstData,messagecommand));
    }else{
        hashTmpData.insert("currEnrg", currEnrg);
    }


    return hashMessage;

}

//-----------------------------------------------------------------------------

QVariantHash DT645EncoderDecoder::fullTotalEnrg(const MessageValidatorResult &decoderesult, const QVariantHash &hashConstData, const QVariantHash &hashTmpData, ErrCounters &warnerr)
{
  QVariantHash resulthash;

  const int trff = hashConstData.value("trff", DEF_TARIFF_NUMB).toInt();
  const QString energyletter = hashTmpData.value("DLT_currEnrgLetter").toString();
  if(decoderesult.listMeterMess.isEmpty() || decoderesult.errCode != DLT645_HAS_NO_ERRORS){
      if(hasNoData(decoderesult.commandCode)){ //add something 2 vrsn
          insertNotSupValues2hash("", energyletter, trff, resulthash);

          resulthash.insert("vrsn", enableDisableTheEnergyKey(hashTmpData.value("vrsn").toString(), hashTmpData.value("DLT_currEnrgLetter").toString(), false));

          resulthash.insert("messFail", false);//go to the next energy
          resulthash.insert("currEnrg", hashTmpData.value("DLT_currEnrg"));
          return resulthash;
      }
  }
  resulthash.insert("vrsn", enableDisableTheEnergyKey(hashTmpData.value("vrsn").toString(), hashTmpData.value("DLT_currEnrgLetter").toString(), true));
  preparyTariffResultHash("", decoderesult.listMeterMess, hashTmpData, trff, energyletter, resulthash);
  return resulthash;

}

//-----------------------------------------------------------------------------

bool DT645EncoderDecoder::closeUnsuppEoMintervals(const QVariantHash &hashConstData, QVariantHash &hashTmpData)
{

    hashTmpData.insert("pollDate_0", hashConstData.value("pollDate_0").toDateTime());
    int monthIndx = hashTmpData.value("monthIndx",0).toInt();
    const QDateTime dtPoll = hashConstData.value("pollDate").toDateTime();
    QDateTime dtPollIndx = hashConstData.value(QString("pollDate_%1").arg(monthIndx), dtPoll).toDateTime();

//    const QStringList listEnrg = hashConstData.value("listEnrg").toStringList();
//    const QStringList getSuppEnrg = getSupportedEnrg(POLL_CODE_READ_END_MONTH, hashTmpData.value("vrsn").toString());

    const QString supported_yyyyMM = QDate::currentDate().addMonths(-1).toString("yyyyMM");

    bool hasSuppInterval = false;
    QStringList warnMess;

    for( ; monthIndx < MAX_GLYBYNA; ){

        if(warnMess.isEmpty())
            warnMess.append(tr("EOM: ignore date:"));
        warnMess.append(QString(" %1,").arg(dtPollIndx.toString("yyyy-MM")));

        monthIndx++;
        if(!hashConstData.contains(QString("pollDate_%1").arg(QString::number( monthIndx)))){
            monthIndx = MAX_GLYBYNA;
            break;
        }
        dtPollIndx = hashConstData.value(QString("pollDate_%1").arg(QString::number( monthIndx)), dtPoll).toDateTime();

        if(dtPollIndx.toString("yyyyMM") == supported_yyyyMM){
            hashTmpData.insert("monthIndx", monthIndx);
            hasSuppInterval = true;
            break;//it is supported interval
        }

    }

    if(!warnMess.isEmpty()){
        int warning_counter = hashTmpData.value("warning_counter").toInt();
        hashTmpData.insert(  MeterPluginHelper::errWarnKey(warning_counter, false), warnMess.join("\n"));
        hashTmpData.insert("warning_counter", warning_counter);
    }
    return hasSuppInterval;

}

//-----------------------------------------------------------------------------

QVariantHash DT645EncoderDecoder::preparyEoM(const QVariantHash &hashConstData, QVariantHash &hashTmpData, quint16 &step)
{
    QVariantHash hashMessage;

    bool ok;
    qint32 monthAgo = MeterPluginHelper::calculateMonthAgo(hashConstData.value("pollDate").toDateTime(), ok);//  //QByteArray::number(hashTmpData.value("NIK_daysAgo", 0).toUInt(), 16);

    if(!ok || monthAgo > 1){
        step = 0xFFFF;
        if(verboseMode) qDebug() << "date is invalid " << hashConstData.value("pollDate") <<hashConstData.value("pollDate").toDateTime() << "end_month " ;
        return hashMessage;
    }
    if(!hashTmpData.value("DLT_enriesAreClosed").toBool()){
        //close not supported intervals
        closeUnsuppEoMintervals(hashConstData, hashTmpData);
        hashTmpData.insert("DLT_enriesAreClosed", true);

    }
    const int monthIndx = hashTmpData.value("monthIndx",0).toInt();
    const QDateTime lastMonthDt = hashConstData.value(QString("pollDate_%1").arg(monthIndx)).toDateTime();
    const QString lastMonthDtStr = QString("%1_%2").arg(lastMonthDt.toString("yyyy_MM")).arg(lastMonthDt.date().daysInMonth());


    const QString version = hashTmpData.value("vrsn").toString();
    const int trff = hashConstData.value("trff", DEF_TARIFF_NUMB).toInt();

    int currEnrg = qMax(0, hashTmpData.value("currEnrg", 0).toInt());
    const QStringList listPlgEnrg = getSupportedEnrg(POLL_CODE_READ_END_MONTH, version);

    const QStringList energies = QString("A+;A-;R+;R-").split(";");
    const QStringList listEnrg = hashConstData.value("listEnrg").toStringList();

    quint32 messagecommand = 0;
    QString enrgletter;




    for(int imax = energies.size(); currEnrg < imax ; currEnrg++){
        const QString energy = energies.at(currEnrg);
        if(!listPlgEnrg.contains(energy) || !listEnrg.contains(energy))
            continue;

        if(messagecommand < 1){

            bool hasEmpty = false;
            for(int t = 0; t <= trff && !hasEmpty; t++){
                const QString valStr = hashTmpData.value(QString("%1_23_59_T%2_%3").arg(lastMonthDtStr).arg(t).arg(energy)).toString();
                hasEmpty = (valStr.isEmpty() || valStr == "?" ||
                            (valStr == "!" && hashConstData.value("vrsn").toString().isEmpty()) || (valStr == "-" && lastMonthDt.daysTo(QDateTime::currentDateTime()) >= 0 && lastMonthDt.daysTo(QDateTime::currentDateTime()) < 2 ));

            }

            if(!hasEmpty)
                continue;

            switch(currEnrg){
            case 0: messagecommand = DLT645_EOF_MONTH_A_POSITIVE; break;
            case 1: messagecommand = DLT645_EOF_MONTH_A_REVERSE ; break;
            case 2: messagecommand = DLT645_EOF_MONTH_R_POSITIVE; break;
            case 3: messagecommand = DLT645_EOF_MONTH_R_REVERSE ; break;
            }
            enrgletter = energy;
        }else{
            break;//to get the next index
        }
    }

    if(messagecommand > 0){
        hashTmpData.insert("DLT_currEnrgLetter", enrgletter);
        hashTmpData.insert("DLT_currEnrg", currEnrg);

        hashMessage.insert("message_0", getReadMessage(hashConstData,messagecommand));
    }else{
        hashTmpData.insert("currEnrg", currEnrg);
    }



 return hashMessage;
}

//-----------------------------------------------------------------------------

QVariantHash DT645EncoderDecoder::fullEoM(const MessageValidatorResult &decoderesult, const QVariantHash &hashConstData, const QVariantHash &hashTmpData, quint16 &step, ErrCounters &warnerr)
{
    QVariantHash resulthash;

    const int monthIndx = hashTmpData.value("monthIndx",0).toInt();
    const QDateTime lastMonthDt = hashConstData.value(QString("pollDate_%1").arg(monthIndx)).toDateTime();
    const QString lastMonthDtStr = QString("%1_%2_23_59").arg(lastMonthDt.toString("yyyy_MM")).arg(lastMonthDt.date().daysInMonth());

    QStringList listDate = hashTmpData.value("listDate").toStringList();
    if(listDate.isEmpty()){
        listDate.append(lastMonthDtStr);
        resulthash.insert("listDate", listDate);
    }


    const int trff = hashConstData.value("trff", DEF_TARIFF_NUMB).toInt();
    const QString energyletter = hashTmpData.value("DLT_currEnrgLetter").toString();
    if(decoderesult.listMeterMess.isEmpty() || decoderesult.errCode != DLT645_HAS_NO_ERRORS){
        if(hasNoData(decoderesult.commandCode)){ //add something 2 vrsn
            insertNotSupValues2hash(lastMonthDtStr, energyletter, trff, resulthash);

            resulthash.insert("vrsn", enableDisableTheEnergyKey(hashTmpData.value("vrsn").toString(), hashTmpData.value("DLT_currEnrgLetter").toString(), false));

            resulthash.insert("messFail", false);//go to the next energy
            resulthash.insert("currEnrg", hashTmpData.value("DLT_currEnrg"));
            return resulthash;
        }
    }
    resulthash.insert("vrsn", enableDisableTheEnergyKey(hashTmpData.value("vrsn").toString(), hashTmpData.value("DLT_currEnrgLetter").toString(), true));
    preparyTariffResultHash(lastMonthDtStr, decoderesult.listMeterMess, hashTmpData, trff, energyletter, resulthash);
    return resulthash;
}

//-----------------------------------------------------------------------------

void DT645EncoderDecoder::insertNotSupValues2hash(const QString &prefics, const QString &enrg, const int &trffs, QVariantHash &hash)
{
    insertStatuses2hash(prefics, enrg, trffs, "!", hash);
}

//-----------------------------------------------------------------------------

void DT645EncoderDecoder::insertStatuses2hash(const QString &prefics, const QString &enrg, const int &trffs, const QString &stts, QVariantHash &hash)
{
    if(enrg.isEmpty() || stts.isEmpty())
        return;//it is impossible
    const QString fullprefics = prefics.isEmpty() ? QString() : QString("%1_").arg(prefics);
    for(int i = 0; i <= trffs; i++)
        hash.insert(QString("%1T%2_%3").arg(fullprefics).arg(i).arg(enrg), stts);
}

//-----------------------------------------------------------------------------
