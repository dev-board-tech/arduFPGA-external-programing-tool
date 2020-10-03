#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QtSerialPort/QSerialPort>

static bool SHOW_PROGRESS = true;

/*
 * This function convert a Value between MinValue and MaxValue to a fit into a value from 0 to MaxPercentageValue.
 */
int util_int_to_percent(int maxPercentageValue, int minValue, int maxValue, int value) {
    int ReturnedValue = 0;
    if(value > maxValue)
        value = maxValue;
    else if(value < minValue)
        value = minValue;
    if (maxValue < 65536)
        ReturnedValue = ((value - minValue) * 0x10000) / (((maxValue - minValue) * 0x10000) / maxPercentageValue);
    else
        ReturnedValue = (value - minValue) / ((maxValue - minValue) / maxPercentageValue);
    if(ReturnedValue > maxPercentageValue)
        ReturnedValue = maxPercentageValue;
    else if(ReturnedValue < 0)
        ReturnedValue = 0;
    return ReturnedValue;
}

/*
 * This function convert a Value between 0 and MaxPercentageValue to a value between MinValue and MaxValue.
 */
int util_percent_to_int(int minValue, int maxValue, int maxPercentageValue, int value) {
    int result = (signed long long)((value * (maxValue - minValue)) / maxPercentageValue) + minValue;
    if(result > maxValue)
        result = maxValue;
    else if(result < minValue)
        result = minValue;
    return result;
}

static bool uartSendReceiveChar(QSerialPort *serial, char c) {
    bool err = false;
    QByteArray arr;
    char cRx;
    arr.append(c);
    if(serial->write(arr) == -1) {
        err = true;
    } else {
        if(serial->waitForReadyRead(5)) {
            QByteArray responseData = serial->readAll();
            if(responseData.count() == 1) {
                cRx = responseData[0];
                if(c != cRx) {
                    err = true;
                } else {
                    err = false;
                }
            } else {
                err = true;
            }
        } else {
            err = true;
        }
    }
    return err;
}

static bool uartReadBytes(bool postStatus, QString logType, int size, QSerialPort *serial, QString *data) {
    QTextStream stream(stdout);
    bool err = false;
    QString sent;
    if(serial->isOpen()) {
        uartSendReceiveChar(serial, '\r');
        //int count = 0;
        while(1) {
            if(sent.length() > 0) {
                for(int sendCnt = 0; sendCnt < sent.length(); sendCnt++) {
                    stream << "\b";
                }
                stream.flush();
                sent.clear();
            }
            bool newData = false;
            QEventLoop loop;
            QTimer::singleShot(250, &loop, SLOT(quit()));
            loop.exec();
            if(serial->waitForReadyRead(50)) {
                QByteArray responseData = serial->readAll();
                data->append(responseData);
                //count += responseData.length();
                if(postStatus) {
                    if(logType.length() > 0){
                        QStringList list = data->split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
                        int count = list.count() * 16;
                        if(!logType.compare("address")) {
                            sent.append("READ: " + QString::asprintf("%d", count));
                            //stream << "READ: " << QString::asprintf("%d", count) << Qt::endl;
                            std::string conv = sent.toStdString();
                            stream << conv.c_str();
                            stream.flush();
                        } else if(!logType.compare("progress")) {
                            sent.append("READ: <");
                            int cnt = 0;
                            int cursor = util_int_to_percent(32, 0, size, count);
                            //int cursor = util_percent_to_int(0, 32, size, count);
                            for(;cnt < cursor; cnt++) {
                                sent.append("=");
                            }
                            for(;cnt < 32; cnt++) {
                                sent.append("-");
                            }
                            sent.append("> " + QString::asprintf("%d", count) + "Bytes");
                            std::string conv = sent.toStdString();
                            stream << conv.c_str();
                            stream.flush();
                        }
                    }
                }
                newData = true;
            } else {
                if(data->length() == 0) {
                    err = true;
                }
            }
            if(!newData) {
                break;
            }
        }
    }
    stream << "\r\n";
    return err;
}

static bool uartSendAddrRange(QSerialPort *serial, QString cmd, QString start, QString end) {
    bool err = false;
    QString command = cmd + ":" + start + "-" + end.mid(end.length()-4, 4);
    if(serial->isOpen()) {
        for(char ch : command.toStdString()) {
            if(uartSendReceiveChar(serial, ch)) {
                err = true;
                break;
            }
        }
    } else {
        err = true;
    }
    return err;
}

static int parseHexFileToBinByte(QByteArray dataIn, QStringList *dataOut) {
    if(dataIn.length() != 0) {

    }
    return -1;
}

static int parseHexFileToBinWord(QByteArray dataIn, QStringList *dataOut) {
    if(dataIn.length() != 0) {

    }
    return -1;
}

static int sendFlashWords(QStringList data) {

    return 0;
}

static int sendBytes(QStringList data) {

    return 0;
}

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    QCoreApplication::setApplicationName("arduFPGA-ptool");
    QCoreApplication::setApplicationVersion("0.1");
    app.addLibraryPath("D:/Qt/5.15.0/mingw81_64");


    QCommandLineParser parser;
    parser.setApplicationDescription("Test helper");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("source", QCoreApplication::translate("main", "Source file to copy."));
    parser.addPositionalArgument("destination", QCoreApplication::translate("main", "Destination directory."));

    QCommandLineOption targetLogTypeOption(QStringList() << "s" << "log-type",
        QCoreApplication::translate("main", "Log type."),
        QCoreApplication::translate("main", "logtype"));
    parser.addOption(targetLogTypeOption);

    QCommandLineOption targetDataOption(QStringList() << "D" << "data",
        QCoreApplication::translate("main", "Data to send."),
        QCoreApplication::translate("main", "data"));
    parser.addOption(targetDataOption);

    QCommandLineOption targetDataTypeOption(QStringList() << "t" << "data-type",
        QCoreApplication::translate("main", "Data type."),
        QCoreApplication::translate("main", "datatype"));
    parser.addOption(targetDataTypeOption);

    QCommandLineOption targetWriteOption(QStringList() << "W" << "write",
        QCoreApplication::translate("main", "Set com PORT."));
    parser.addOption(targetWriteOption);

    QCommandLineOption targetReadOption(QStringList() << "R" << "read",
        QCoreApplication::translate("main", "Set com PORT."));
    parser.addOption(targetReadOption);

    QCommandLineOption targetComPortOption(QStringList() << "p" << "port",
        QCoreApplication::translate("main", "Set com PORT."),
        QCoreApplication::translate("main", "comport"));
    parser.addOption(targetComPortOption);

    QCommandLineOption targetBaudRateOption(QStringList() << "b" << "baud-rate",
        QCoreApplication::translate("main", "Set baudrate."),
        QCoreApplication::translate("main", "baudrate"));
    parser.addOption(targetBaudRateOption);

    QCommandLineOption targetStartOption(QStringList() << "S" << "start",
        QCoreApplication::translate("main", "Set start address."),
        QCoreApplication::translate("main", "start"));
    parser.addOption(targetStartOption);

    QCommandLineOption targetEndOption(QStringList() << "E" << "end",
        QCoreApplication::translate("main", "Set end address."),
        QCoreApplication::translate("main", "end"));
    parser.addOption(targetEndOption);

    QCommandLineOption targetFlashFileOption(QStringList() << "f" << "flash-file",
        QCoreApplication::translate("main", "FLASH file path."),
        QCoreApplication::translate("main", "flashfile"));
    parser.addOption(targetFlashFileOption);

    QCommandLineOption targetEepromFileOption(QStringList() << "e" << "eep-file",
        QCoreApplication::translate("main", "EEPROM file path."),
        QCoreApplication::translate("main", "eepromfile"));
    parser.addOption(targetEepromFileOption);

    QCommandLineOption targetRamFileOption(QStringList() << "r" << "ram-file",
        QCoreApplication::translate("main", "RAM file path."),
        QCoreApplication::translate("main", "ramfile"));
    parser.addOption(targetRamFileOption);

    QCommandLineOption targetFlashSizeOption(QStringList() << "x" << "flash-size",
        QCoreApplication::translate("main", "RAM file path."),
        QCoreApplication::translate("main", "ramfile"));
    parser.addOption(targetFlashSizeOption);

    QCommandLineOption targetEepromSizeOption(QStringList() << "y" << "eeprom-size",
        QCoreApplication::translate("main", "RAM file path."),
        QCoreApplication::translate("main", "ramfile"));
    parser.addOption(targetEepromSizeOption);

    QCommandLineOption targetRamSizeOption(QStringList() << "z" << "ram-size",
        QCoreApplication::translate("main", "RAM file path."),
        QCoreApplication::translate("main", "ramfile"));
    parser.addOption(targetRamSizeOption);

    parser.process(app);

    bool logTypeIsSet = parser.isSet(targetLogTypeOption);
    QString logType = parser.value(targetLogTypeOption);

    bool dataIsSet = parser.isSet(targetDataOption);
    QString data = parser.value(targetDataOption);

    bool dataTypeIsSet = parser.isSet(targetDataTypeOption);
    QString dataType = parser.value(targetDataTypeOption);

    bool writeIsSet = parser.isSet(targetWriteOption);

    bool readIsSet = parser.isSet(targetReadOption);

    bool comPortIsSet = parser.isSet(targetComPortOption);
    QString comPort = parser.value(targetComPortOption);

    bool baudRateIsSet = parser.isSet(targetBaudRateOption);
    QString baudRate = parser.value(targetBaudRateOption);

    bool startIsSet = parser.isSet(targetStartOption);
    QString start = parser.value(targetStartOption);

    bool endIsSet = parser.isSet(targetEndOption);
    QString end = parser.value(targetEndOption);

    bool flashFileIsSet = parser.isSet(targetFlashFileOption);
    QString flashFile = parser.value(targetFlashFileOption);

    bool eepromFileIsSet = parser.isSet(targetEepromFileOption);
    QString eepromFile = parser.value(targetEepromFileOption);

    bool ramFileIsSet = parser.isSet(targetRamFileOption);
    QString ramFile = parser.value(targetRamFileOption);

    bool flashSizeIsSet = parser.isSet(targetFlashSizeOption);
    QString flashSize = parser.value(targetFlashSizeOption);

    bool eepromSizeIsSet = parser.isSet(targetEepromSizeOption);
    QString eepromSize = parser.value(targetEepromSizeOption);

    bool ramSizeIsSet = parser.isSet(targetRamSizeOption);
    QString ramSize = parser.value(targetRamSizeOption);

    QTextStream stream(stdout);

    QSerialPort serial;
    if(!dataIsSet && comPortIsSet && baudRateIsSet && ((writeIsSet && !readIsSet) || (!writeIsSet && readIsSet)) && (flashFileIsSet || eepromFileIsSet || ramFileIsSet)) {
        serial.setPortName(comPort);
        serial.setBaudRate(baudRate.toInt());
        if (!serial.open(QIODevice::ReadWrite)) {
            stream << "ERR Cannot open COM." << Qt::endl;
            return -6;
        } else {
            if(readIsSet) {
                if(flashFileIsSet) {
                    QFile file(flashFile);
                    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                            stream << "ERR: Cannot create file." << Qt::endl;
                            serial.close();
                            return -2;
                    } else {
                        if(!uartSendAddrRange(&serial, "RF", "0000", flashSize.length() > 3 ? flashSize : "0000")) {
                            QString data;
                            stream << "FLASH: Reading..." << Qt::endl;
                            bool ok = false;
                            if(!uartReadBytes(SHOW_PROGRESS, logType, flashSize.toInt(&ok, 16), &serial, &data)) {
                                data = data.remove(QRegExp("^((?:\r?\n|\r))+"));
                                data = data.replace("\n\r", "\r");
                                QByteArray ba = data.toLocal8Bit();
                                file.write((const char*)(ba.data()), ba.length());
                            } else {
                                stream << "ERR: arduFPGA does not send any data." << Qt::endl;
                                serial.close();
                                return -4;
                            }
                        } else {
                            stream << "ERR: arduFPGA board does not respond." << Qt::endl;
                            serial.close();
                            return -3;
                        }
                    }
                }
                if(eepromFileIsSet) {
                    QFile file(eepromFile);
                    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                            stream << "ERR: Cannot create file." << Qt::endl;
                            serial.close();
                            return -2;
                    } else {
                        if(!uartSendAddrRange(&serial, "RE", "0000", eepromSize.length() > 3 ? eepromSize : "0000")) {
                            QString data;
                            stream << "EEPROM: Reading..." << Qt::endl;
                            bool ok = false;
                            if(!uartReadBytes(SHOW_PROGRESS, logType, eepromSize.toInt(&ok, 16), &serial, &data)) {
                                data = data.remove(QRegExp("^((?:\r?\n|\r))+"));
                                data = data.replace("\n\r", "\r");
                                QByteArray ba = data.toLocal8Bit();
                                file.write((const char*)(ba.data()), ba.length());
                            } else {
                                stream << "ERR: arduFPGA does not send any data." << Qt::endl;
                                serial.close();
                                return -4;
                            }
                        } else {
                            stream << "ERR: arduFPGA board does not respond." << Qt::endl;
                            serial.close();
                            return -3;
                        }
                    }
                }
                if(ramFileIsSet) {
                    QFile file(ramFile);
                    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                            stream << "ERR: Cannot create file." << Qt::endl;
                            serial.close();
                            return -2;
                    } else {
                        if(!uartSendAddrRange(&serial, "RR", "0000", ramSize.length() > 3 ? ramSize : "0000")) {
                            QString data;
                            stream << "RAM: Reading..." << Qt::endl;
                            bool ok = false;
                            if(!uartReadBytes(SHOW_PROGRESS, logType, ramSize.toInt(&ok, 16), &serial, &data)) {
                                data = data.remove(QRegExp("^((?:\r?\n|\r))+"));
                                data = data.replace("\n\r", "\r");
                                QByteArray ba = data.toLocal8Bit();
                                file.write((const char*)(ba.data()), ba.length());
                            } else {
                                stream << "ERR: arduFPGA does not send any data." << Qt::endl;
                                serial.close();
                                return -4;
                            }
                        } else {
                            stream << "ERR: arduFPGA board does not respond." << Qt::endl;
                            serial.close();
                            return -3;
                        }
                    }
                }
            } else if(writeIsSet) {
                if(flashFileIsSet) {
                    if(flashFileIsSet) {
                        QFileInfo fi(flashFile);
                        QString ext = fi.completeSuffix();
                        if(ext.compare("bin") && ext.compare("hex")) {
                            stream << "ERR: FLASH file extension not supported." << Qt::endl;
                            serial.close();
                            return -5;
                        }
                        QFile file(flashFile);
                        if (!file.open(QIODevice::ReadOnly)) {
                                stream << "ERR: Cannot open file." << Qt::endl;
                                serial.close();
                                return -2;
                        } else {
                            QStringList data;
                            QByteArray binary = file.readAll();
                            if(!ext.compare("bin")) {
                                for (int cnt = 0; cnt < file.size(); cnt+=2) {
                                    data.append(QString::asprintf("%02X%02X", (uint8_t)binary.at(cnt+1), (uint8_t)binary.at(cnt)));
                                }
                            } else {
                                if(!ext.compare("hex")) {
                                    parseHexFileToBinWord(binary, &data);
                                }
                            }
                            if(!uartSendAddrRange(&serial, "WF", "0000", QString::asprintf("%04X", data.count()))) {
                                if(sendFlashWords(data)) {
                                    stream << "ERR: arduFPGA does not send any data." << Qt::endl;
                                    serial.close();
                                    return -4;
                                }
                            } else {
                                stream << "ERR: arduFPGA board does not respond." << Qt::endl;
                                serial.close();
                                return -3;
                            }
                        }
                    }
                }
            } else {
                stream << "ERR: No operation selected." << Qt::endl;
                serial.close();
                return -3;
            }
            stream << "OK: Reading memory." << Qt::endl;
            serial.close();
        }
    } else if(dataTypeIsSet && comPortIsSet && baudRateIsSet && startIsSet && endIsSet) {
        serial.setPortName(comPort);
        serial.setBaudRate(baudRate.toInt());
        if (!serial.open(QIODevice::ReadWrite)) {
            stream << "ERR Cannot open COM." << Qt::endl;
            return -6;
        } else {
            if(readIsSet) {
                if(!dataType.compare("FLASH")) {
                    if(!uartSendAddrRange(&serial, "RF", start, end)) {
                        QString data;
                        bool ok = false;
                        if(!uartReadBytes(false, logType, end.toInt(&ok, 16) - start.toInt(&ok, 16), &serial, &data)) {
                            data = data.remove(QRegExp("^((?:\r?\n|\r))+"));
                            data = data.replace("\n\r", "\r");
                            QStringList list = data.split("\r", QString::SkipEmptyParts);
                            for(QString str : list) {
                                std::string conv = str.toStdString();
                                stream << conv.c_str() << Qt::endl;
                            }
                            stream << "OK: Reading FLASH memory." << Qt::endl;
                        } else {
                            stream << "ERR: arduFPGA does not send any data." << Qt::endl;
                            serial.close();
                            return -4;
                        }
                    } else {
                        stream << "ERR: arduFPGA board does not respond." << Qt::endl;
                        serial.close();
                        return -3;
                    }
                } else if(!dataType.compare("EEPROM")) {
                    if(!uartSendAddrRange(&serial, "RE", start, end)) {
                        QString data;
                        bool ok = false;
                        if(!uartReadBytes(false, logType, end.toInt(&ok, 16) - start.toInt(&ok, 16), &serial, &data)) {
                            data = data.remove(QRegExp("^((?:\r?\n|\r))+"));
                            data = data.replace("\n\r", "\r");
                            QStringList list = data.split("\r", QString::SkipEmptyParts);
                            for(QString str : list) {
                                std::string conv = str.toStdString();
                                stream << conv.c_str() << Qt::endl;
                            }
                            stream << "OK: Reading EEPROM memory." << Qt::endl;
                        } else {
                            stream << "ERR: arduFPGA does not send any data." << Qt::endl;
                            serial.close();
                            return -4;
                        }
                    } else {
                        stream << "ERR: arduFPGA board does not respond." << Qt::endl;
                        serial.close();
                        return -3;
                    }

                } else if(!dataType.compare("RAM")) {
                    if(!uartSendAddrRange(&serial, "RR", start, end)) {
                        QString data;
                        bool ok = false;
                        if(!uartReadBytes(false, logType, end.toInt(&ok, 16) - start.toInt(&ok, 16), &serial, &data)) {
                            data = data.remove(QRegExp("^((?:\r?\n|\r))+"));
                            data = data.replace("\n\r", "\r");
                            QStringList list = data.split("\r", QString::SkipEmptyParts);
                            for(QString str : list) {
                                std::string conv = str.toStdString();
                                stream << conv.c_str() << Qt::endl;
                            }
                            stream << "OK: Reading RAM memory." << Qt::endl;
                        } else {
                            stream << "ERR: arduFPGA does not send any data." << Qt::endl;
                            serial.close();
                            return -4;
                        }
                    } else {
                        stream << "ERR: arduFPGA board does not respond." << Qt::endl;
                        serial.close();
                        return -3;
                    }

                } else {
                    stream << "ERR: Invalit memory type." << Qt::endl;
                    serial.close();
                    return -8;
                }
            } else if(writeIsSet) {

            } else {
                stream << "ERR: No operation selected." << Qt::endl;
                serial.close();
                return -3;
            }
            serial.close();
        }
    } else {
        stream << "ERR Incorect arguments." << Qt::endl;
        return -1;
    }

    return app.exec();
}
