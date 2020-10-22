#include <QtCore>
#include <QCoreApplication>
#include <QCommandLineParser>
#include <QTextStream>
#include <QFile>
#include <QFileInfo>
#include <QTimer>
#include <QString>
#include <QByteArray>
#include <QEventLoop>
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QThread>

#define BAUD_RATE_LIST  {   "600000",\
                            "576000",\
                            "500000",\
                            "480000",\
                            "460800",\
                            "400000",\
                            "375000",\
                            "320000",\
                            "300000",\
                            "250000",\
                            "240000",\
                            "230400",\
                            "200000",\
                            "192000",\
                            "187500",\
                            "160000",\
                            "150000",\
                            "125000",\
                            "120000",\
                            "115200",\
                            "100000",\
                            "96000",\
                            "80000",\
                            "75000",\
                            "64000",\
                            "62500",\
                            "60000",\
                            "57600",\
                            "50000",\
                            "48000",\
                            "40000",\
                            "38400",\
                            "37500",\
                            "32000",\
                            "30000",\
                            "25000",\
                            "24000",\
                            "20000",\
                            "19200",\
                            "16000",\
                            "15000",\
                            "12800",\
                            "12500",\
                            "12000",\
                            "10000",\
                            "9600",\
                            "8000",\
                            "7500",\
                            "6400",\
                            "6000",\
                            "5000",\
                            "4800",\
                            "4000",\
                            "3200",\
                            "3000",\
                            "2500",\
                            "2400",\
                            "2000",\
                            "1800",\
                            "1600",\
                            "1500",\
                            "1200",\
                            "1000",\
                            "800",\
                            "600",\
                            "500",\
                            "400",\
                            "300",\
                            "200",\
                            "100"}

#define PROGRESS_VISIBLE                    true
#define PROGRESS_LENGTH                     32

#define ERR_OK                               0
#define ERR_NO_OPERATION_SELECTED           -1
#define ERR_INCORECT_ARGUMENT               -2
#define ERR_NOT_SEND_ANY_DATA               -4
#define ERR_BOARD_NOT_RESPONDING            -5
#define ERR_FILE_EXTENSION_NOT_SUPPORTED    -6
#define ERR_CAN_NOT_OPPEN_COM_PORT          -7
#define ERR_INVALID_MEMORY_TYPE             -8
#define ERR_INVALID_HEX_FILE                -9
#define ERR_INVALID_CHECK_SUM               -10
#define ERR_NOT_INPLEMENTED                 -11
#define ERR_NOTHING_TO_DO                   -12
#define ERR_WRONG_RESPONSE                  -13
#define ERR_UNRECOGNIZED_FILE               -14
#define ERR_CAN_NOT_OPEN_FLASH_FILE         -15
#define ERR_CAN_NOT_OPEN_EEPROM_FILE        -16
#define ERR_CAN_NOT_OPEN_RAM_FILE           -17
#define ERR_CAN_NOT_CREATE_FLASH_FILE       -18
#define ERR_CAN_NOT_CREATE_EEPROM_FILE      -19
#define ERR_CAN_NOT_CREATE_RAM_FILE         -20

#define MESSAGE_0   "INFO: Operation done successfully."
#define MESSAGE_1   "ERROR: No operation selected."
#define MESSAGE_2   "ERROR: Incorect argument."
#define MESSAGE_4   "ERROR: arduFPGA does not send any data."
#define MESSAGE_5   "ERROR: arduFPGA board not responding."
#define MESSAGE_6   "ERROR: File extension not supported."
#define MESSAGE_7   "ERROR: Cannot open COM."
#define MESSAGE_8   "ERROR: Invalit memory type."
#define MESSAGE_9   "ERROR: Invalid hex file."
#define MESSAGE_10  "ERROR: Invalid check sum."
#define MESSAGE_11  "ERROR: Function not inplemented."
#define MESSAGE_12  "ERROR: Nothing to do."
#define MESSAGE_13  "ERROR: arduFPGA send wrong response."
#define MESSAGE_14  "ERROR: Unrecognized file format."
#define MESSAGE_15  "ERROR: Can not open FLASH file."
#define MESSAGE_16  "ERROR: Can not open EEPROM file."
#define MESSAGE_17  "ERROR: Can not open RAM file."
#define MESSAGE_18  "ERROR: Cannot create FLASH file."
#define MESSAGE_19  "ERROR: Cannot create EEPROM file."
#define MESSAGE_20  "ERROR: Cannot create RAM file."

/*
 * This function convert a Value between MinValue and MaxValue to a fit into a value from 0 to MaxPercentageValue.
 */
int util_int_to_percent(int maxPercentageValue, int minValue, int maxValue, int value) {
    int ReturnedValue = 0;
    if(value > maxValue) {
        value = maxValue;
    }
    else if(value < minValue) {
        value = minValue;
    }
    ReturnedValue = (value - minValue) / ((maxValue - minValue) / maxPercentageValue);
    if(ReturnedValue > maxPercentageValue)
        ReturnedValue = maxPercentageValue;
    else if(ReturnedValue < 0)
        ReturnedValue = 0;
    return ReturnedValue;
}

static bool uartSendReceiveChar(QSerialPort *serial, char c) {
    bool err = false;
    QByteArray arr;
    char cRx;
    arr.append(c);
    if(serial->write(arr) == ERR_INCORECT_ARGUMENT) {
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

void displayProgress(QString *sent, QString logType, int count, int size) {
    QTextStream stream(stdout);
    if(sent->length() > 0) {
        for(int sendCnt = 0; sendCnt < sent->length(); sendCnt++) {
            stream << "\b";
        }
        stream.flush();
        sent->clear();
    }
    if(!logType.compare("address")) {
        sent->append("READ: " + QString::asprintf("%d", count));
        //stream << "READ: " << QString::asprintf("%d", count) << Qt::endl;
        std::string conv = sent->toStdString();
        stream << conv.c_str();
        stream.flush();
    } else if(!logType.compare("progress")) {
        sent->append("READ: <");
        int cnt = 0;
        int cursor = util_int_to_percent(PROGRESS_LENGTH, 0, size, count);
        //int cursor = util_percent_to_int(0, PROGRESS_LENGTH, size, count);
        for(;cnt < cursor; cnt++) {
            sent->append("=");
        }
        for(;cnt < PROGRESS_LENGTH; cnt++) {
            sent->append("-");
        }
        sent->append("> " + QString::asprintf("%d", count) + "Bytes");
        std::string conv = sent->toStdString();
        stream << conv.c_str();
        stream.flush();
    }
}

static bool uartReadBytes(bool postStatus, QString logType, int size, QSerialPort *serial, QString *data) {
    QTextStream stream(stdout);
    bool err = false;
    QString sent;
    QStringList list;
    int count = 0;
    if(serial->isOpen()) {
        uartSendReceiveChar(serial, '\r');
        while(1) {
            bool newData = false;
            QEventLoop loop;
            QTimer::singleShot(150, &loop, SLOT(quit()));
            loop.exec();
            if(serial->waitForReadyRead(50)) {
                QByteArray responseData = serial->readAll();
                data->append(responseData);
                if(postStatus) {
                    if(logType.length() > 0){
                        list = data->split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
                        count = list.count() * 16;
                        displayProgress(&sent, logType, count,size);
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
    QByteArray responseData = serial->readAll();
    data->append(responseData);
    list = data->split(QRegExp("[\r\n]"),QString::SkipEmptyParts);
    count = list.count() * 16;
    displayProgress(&sent, logType, count,size);
    stream << "\r\n";
    return err;
}

static bool uartSendAddrRange(QSerialPort *serial, QString cmd, QString start, QString end) {
    bool err = false;
    QString command = cmd + ":" + start + "-" + end.mid(end.length()-4, 4) + "\r";
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

static int parseHexFileToBinByte(QByteArray dataIn, QStringList *romOut) {
    QTextStream stream(stdout);
    if(dataIn.length() != 0) {
        QString tmp = dataIn;
        QStringList dataLines = tmp.split("\r\n");
        uint32_t baseAddress = 0;
        uint32_t extendedAddress = 0;
        for(QString line : dataLines) {
            if(line.at(0) == ':') {
                bool ok = false;
                uint32_t lineLen = line.mid(1, 2).toUInt(&ok, 16) * 2;
                if (lineLen == (uint32_t)(line.length() - 11))
                {
                    int cnt = 1;
                    uint8_t chk = 0;
                    for (; cnt < line.length() - 2; cnt += 2)
                    {
                        chk -= (uint8_t)(line.mid(cnt, 2).toUInt(&ok, 16));
                    }
                    uint8_t chkCalc = line.mid((int)lineLen + 9, 2).toUInt(&ok, 16);
                    if (chkCalc != chk)
                    {
                        stream << MESSAGE_10 << Qt::endl;
                        return ERR_INVALID_CHECK_SUM;
                    }
                    QString lineType = line.mid(7, 2);
                    if (!lineType.compare("00")) // Data line//Contains data and 16-bit address. The format described above.
                    {
                        uint32_t lineAddrLowInt = (extendedAddress - baseAddress) + line.mid(3, 4).toUInt(&ok, 16);
                         QString lineData = line.mid(9, lineLen);
                         if ((uint32_t)romOut->count() < lineAddrLowInt + (lineLen / 2))
                         {
                             uint32_t lineNr = romOut->count();
                             for (; lineNr < lineAddrLowInt + (lineLen / 2); lineNr = romOut->count())
                             {
                                 romOut->append("FF");
                             }
                             lineNr = 0;
                             for (; lineNr < lineLen / 2; lineNr++)
                             {
                                 romOut->replace((int)lineAddrLowInt + lineNr, lineData.mid(lineNr * 2, 2));
                             }
                         }
                    }
                    else if (!lineType.compare("01")) //End of file//A file termination record. No data. Has to be the last line of the file, only one per file permitted. Usually ':00000001FF'. Originally the End Of File record could contain a start address for the program being loaded, e.g. :00AB2F0125 would make a jump to address AB2F. This was convenient when programs were loaded from punched paper tape.
                    {
                        stream << MESSAGE_0 << Qt::endl;
                        return ERR_OK;
                    }
                    else if (!lineType.compare("02")) //Extended segment address//Segment-base address. Used when 16 bits are not enough, identical to 80x86 real mode addressing. The address specified by the 02 record is multiplied by 16 (shifted 4 bits left) and added to the subsequent 00 record addresses. This allows addressing of up to a megabyte of address space. The address field of this record has to be 0000, the byte count is 02 (the segment is 16-bit). The least significant hex digit of the segment address is always 0.
                    {
                        extendedAddress = line.mid(9, 4).toULong(&ok, 16) << 4;
                    }
                    else if (!lineType.compare("03")) //Start segment address//For 80x86 processors, it specifies the initial content of the CS:IP registers. The address field is 0000, the byte count is 04, the first two bytes are the CS value, the latter two are the IP value.
                    {
                        stream << MESSAGE_11 << Qt::endl;
                        return ERR_NOT_INPLEMENTED;
                    }
                    else if (!lineType.compare("04")) //Extended line address//Allowing for fully 32 bit addressing. The address field is 0000, the byte count is 02. The two data bytes represent the upper 16 bits of the 32 bit address, when combined with the address of the 00 type record.
                    {
                        extendedAddress = line.mid(9, 4).toULong(&ok, 16) << 16;
                    }
                    else if (!lineType.compare("05")) //Start line address//The address field is 0000, the byte count is 04. The 4 data bytes represent the 32-bit value loaded into the EIP register of the 80386 and higher CPU.
                    {
                        stream << MESSAGE_11 << Qt::endl;
                        return ERR_NOT_INPLEMENTED;
                    }
                }
            }
        }
    }
    return ERR_INCORECT_ARGUMENT;
}

static int parseHexFileToBinWord(QByteArray dataIn, QStringList *dataOut) {
    QTextStream stream(stdout);
    if(dataIn.length() != 0) {
        QStringList romByte;
        int err = ERR_OK;
        if(!(err = parseHexFileToBinByte(dataIn, &romByte))) {
            for(uint32_t cnt = 0; cnt < (uint32_t)romByte.count(); cnt += 2) {
                QString c0 = "FF", c1 = "FF";
                c0 = romByte.at(cnt);
                if((cnt + 1) < (uint32_t)romByte.count()) {
                    c1 = romByte.at(cnt + 1);
                }
                dataOut->append(c1 + c0);
            }
        } else {
            return err;
        }
    } else {
        stream << MESSAGE_12 << Qt::endl;
        return ERR_NOTHING_TO_DO;
    }
    stream << MESSAGE_0 << Qt::endl;
    return ERR_OK;
}

static int sendData(bool postStatus, QSerialPort *serial, QStringList data, QString logType) {
    int cnt = 0;
    int cntProgress = 0;
    QString sent;
    if(serial->isOpen()) {
        serial->clear();
        for(QString line : data) {
            serial->write(line.toUtf8());
            serial->waitForBytesWritten(30000);
            {
                cntProgress++;
                QByteArray responseData = serial->readAll();
                if(cntProgress >= (data.count() * 2) / PROGRESS_LENGTH && postStatus) {
                    displayProgress(&sent, logType, cnt * 2,data.count() * 2);
                    cntProgress = 0;
                }
                cnt++;

            }
        }
    }
    if(postStatus) {
        displayProgress(&sent, logType, cnt * 2,data.count() * 2);
        QTextStream stream(stdout);
        stream << "\r\n";
    }
    return ERR_OK;
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

    QCommandLineOption targetCommandOption(QStringList() << "C" << "command",
        QCoreApplication::translate("main", "Get COM list."),
        QCoreApplication::translate("main", "command"));
    parser.addOption(targetCommandOption);

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

    bool commandIsSet = parser.isSet(targetCommandOption);
    QString command = parser.value(targetCommandOption);

    //bool logTypeIsSet = parser.isSet(targetLogTypeOption);
    QString logType = parser.value(targetLogTypeOption);

    bool dataIsSet = parser.isSet(targetDataOption);
    QString data = parser.value(targetDataOption);

    bool dataTypeIsSet = parser.isSet(targetDataTypeOption);
    QString dataType = parser.value(targetDataTypeOption);

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

    if(commandIsSet && !dataIsSet && comPortIsSet && baudRateIsSet && ((flashFileIsSet && flashSizeIsSet) || (eepromFileIsSet && eepromSizeIsSet) || (ramFileIsSet && ramSizeIsSet) || ((flashFileIsSet || eepromFileIsSet || ramFileIsSet) && (!command.compare("write") || !command.compare("writer"))))) {
        serial.setPortName(comPort);
        serial.setBaudRate(baudRate.toInt());
        if (!serial.open(QIODevice::ReadWrite)) {
            stream << MESSAGE_7 << Qt::endl;
            return ERR_CAN_NOT_OPPEN_COM_PORT;
        } else {
            if(!command.compare("read")) {
                if(flashFileIsSet) {
                    QFile file(flashFile);
                    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                            stream << MESSAGE_18 << Qt::endl;
                            serial.close();
                            return ERR_CAN_NOT_CREATE_FLASH_FILE;
                    } else {
                        bool ok = false;
                        int size = flashSize.toInt(&ok, 16);
                        if(!uartSendAddrRange(&serial, "RF", "0000", size < 65536 ? QString::asprintf("%04x", size) : "0000")) {
                            QString data;
                            stream << "FLASH: Reading..." << Qt::endl;
                            if(!uartReadBytes(PROGRESS_VISIBLE, logType, flashSize.toInt(&ok, 16), &serial, &data)) {
                                data = data.remove(QRegExp("^((?:\r?\n|\r))+"));
                                data = data.replace("\n\r", "\r");
                                QByteArray ba = data.toLocal8Bit();
                                file.write((const char*)(ba.data()), ba.length());
                            } else {
                                stream << MESSAGE_4 << Qt::endl;
                                serial.close();
                                return ERR_NOT_SEND_ANY_DATA;
                            }
                        } else {
                            stream << MESSAGE_5 << Qt::endl;
                            serial.close();
                            return ERR_BOARD_NOT_RESPONDING;
                        }
                    }
                }
                if(eepromFileIsSet) {
                    QFile file(eepromFile);
                    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                            stream << MESSAGE_19 << Qt::endl;
                            serial.close();
                            return ERR_CAN_NOT_CREATE_EEPROM_FILE;
                    } else {
                        bool ok = false;
                        int size = eepromSize.toInt(&ok, 16);
                        if(!uartSendAddrRange(&serial, "RE", "0000", size < 65536 ? QString::asprintf("%04x", size) : "0000")) {
                            QString data;
                            stream << "EEPROM: Reading..." << Qt::endl;
                            if(!uartReadBytes(PROGRESS_VISIBLE, logType, eepromSize.toInt(&ok, 16), &serial, &data)) {
                                data = data.remove(QRegExp("^((?:\r?\n|\r))+"));
                                data = data.replace("\n\r", "\r");
                                QByteArray ba = data.toLocal8Bit();
                                file.write((const char*)(ba.data()), ba.length());
                            } else {
                                stream << MESSAGE_4 << Qt::endl;
                                serial.close();
                                return ERR_NOT_SEND_ANY_DATA;
                            }
                        } else {
                            stream << MESSAGE_5 << Qt::endl;
                            serial.close();
                            return ERR_BOARD_NOT_RESPONDING;
                        }
                    }
                }
                if(ramFileIsSet) {
                    QFile file(ramFile);
                    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                            stream << MESSAGE_20 << Qt::endl;
                            serial.close();
                            return ERR_CAN_NOT_CREATE_RAM_FILE;
                    } else {
                        bool ok = false;
                        int size = ramSize.toInt(&ok, 16);
                        if(!uartSendAddrRange(&serial, "RR", "0000", size < 65536 ? QString::asprintf("%04x", size) : "0000")) {
                            QString data;
                            stream << "RAM: Reading..." << Qt::endl;
                            if(!uartReadBytes(PROGRESS_VISIBLE, logType, ramSize.toInt(&ok, 16), &serial, &data)) {
                                data = data.remove(QRegExp("^((?:\r?\n|\r))+"));
                                data = data.replace("\n\r", "\r");
                                QByteArray ba = data.toLocal8Bit();
                                file.write((const char*)(ba.data()), ba.length());
                            } else {
                                stream << MESSAGE_4 << Qt::endl;
                                serial.close();
                                return ERR_NOT_SEND_ANY_DATA;
                            }
                        } else {
                            stream << MESSAGE_5 << Qt::endl;
                            serial.close();
                            return ERR_BOARD_NOT_RESPONDING;
                        }
                    }
                }
                return ERR_OK;
            } else if(!command.compare("write") || !command.compare("writer")) {
                if(flashFileIsSet) {
                    QFileInfo fi(flashFile);
                    QString ext = fi.suffix();
                    if(ext.compare("bin") && ext.compare("hex")) {
                        stream << MESSAGE_6 << Qt::endl;
                        serial.close();
                        return ERR_FILE_EXTENSION_NOT_SUPPORTED;
                    }
                    QFile file(flashFile);
                    if (!file.open(QIODevice::ReadOnly)) {
                            stream << MESSAGE_15 << Qt::endl;
                            serial.close();
                            return ERR_CAN_NOT_OPEN_FLASH_FILE;
                    } else {
                        QStringList data;
                        QByteArray binary = file.readAll();
                        if(!ext.compare("bin")) {
                            /*
                             * From bin to necessary format is simple.
                             */
                            for (int cnt = 0; cnt < file.size(); cnt+=2) {
                                data.append(QString::asprintf("%02X%02X", (uint8_t)binary.at(cnt+1), (uint8_t)binary.at(cnt)));
                            }
                        } else if(!ext.compare("hex")) {
                            if(parseHexFileToBinWord(binary, &data)) {
                                stream << MESSAGE_9 << Qt::endl;
                                serial.close();
                                return ERR_INVALID_HEX_FILE;
                            }
                        } else {
                            stream << MESSAGE_14 << Qt::endl;
                            serial.close();
                            return ERR_UNRECOGNIZED_FILE;
                        }
                        if(!command.compare("writer") && !eepromFileIsSet && !ramFileIsSet) {
                            data.append("L");
                        }
                        if(!uartSendAddrRange(&serial, "WF", "0000", QString::asprintf("%04X", data.count()))) {
                            if(sendData(PROGRESS_VISIBLE, &serial, data, logType)) {
                                stream << MESSAGE_4 << Qt::endl;
                                serial.close();
                                return ERR_NOT_SEND_ANY_DATA;
                            }
                        } else {
                            stream << MESSAGE_5 << Qt::endl;
                            serial.close();
                            return ERR_BOARD_NOT_RESPONDING;
                        }
                    }
                }
                if(eepromFileIsSet) {
                    QFileInfo fi(eepromFile);
                    QString ext = fi.suffix();
                    if(ext.compare("bin") && ext.compare("eep")) {
                        stream << MESSAGE_6 << Qt::endl;
                        serial.close();
                        return ERR_FILE_EXTENSION_NOT_SUPPORTED;
                    }
                    QFile file(eepromFile);
                    if (!file.open(QIODevice::ReadOnly)) {
                            stream << MESSAGE_16 << Qt::endl;
                            serial.close();
                            return ERR_CAN_NOT_OPEN_EEPROM_FILE;
                    } else {
                        QStringList data;
                        QByteArray binary = file.readAll();
                        if(!ext.compare("bin")) {
                            /*
                             * From bin to necessary format is simple.
                             */
                            for (int cnt = 0; cnt < file.size(); cnt+=2) {
                                data.append(QString::asprintf("%02X%02X", (uint8_t)binary.at(cnt+1), (uint8_t)binary.at(cnt)));
                            }
                        } else if(!ext.compare("eep")) {
                            if(parseHexFileToBinByte(binary, &data)) {
                                stream << MESSAGE_9 << Qt::endl;
                                serial.close();
                                return ERR_INVALID_HEX_FILE;
                            }
                        } else {
                            stream << MESSAGE_14 << Qt::endl;
                            serial.close();
                            return ERR_UNRECOGNIZED_FILE;
                        }
                        if(!command.compare("writer") && !ramFileIsSet) {
                            data.append("L");
                        }
                        if(!uartSendAddrRange(&serial, "WE", "0000", QString::asprintf("%04X", data.count()))) {
                            if(sendData(PROGRESS_VISIBLE, &serial, data, logType)) {
                                stream << MESSAGE_4 << Qt::endl;
                                serial.close();
                                return ERR_NOT_SEND_ANY_DATA;
                            }
                        } else {
                            stream << MESSAGE_5 << Qt::endl;
                            serial.close();
                            return ERR_BOARD_NOT_RESPONDING;
                        }
                    }
                }
                if(ramFileIsSet) {
                    QFileInfo fi(ramFile);
                    QString ext = fi.suffix();
                    if(ext.compare("bin") && ext.compare("ram")) {
                        stream << MESSAGE_6 << Qt::endl;
                        serial.close();
                        return ERR_FILE_EXTENSION_NOT_SUPPORTED;
                    }
                    QFile file(ramFile);
                    if (!file.open(QIODevice::ReadOnly)) {
                            stream << MESSAGE_17 << Qt::endl;
                            serial.close();
                            return ERR_CAN_NOT_OPEN_RAM_FILE;
                    } else {
                        QStringList data;
                        QByteArray binary = file.readAll();
                        if(!ext.compare("bin")) {
                            /*
                             * From bin to necessary format is simple.
                             */
                            for (int cnt = 0; cnt < file.size(); cnt+=2) {
                                data.append(QString::asprintf("%02X%02X", (uint8_t)binary.at(cnt+1), (uint8_t)binary.at(cnt)));
                            }
                        } else if(!ext.compare("ram")) {
                            if(parseHexFileToBinByte(binary, &data)) {
                                stream << MESSAGE_9 << Qt::endl;
                                serial.close();
                                return ERR_INVALID_HEX_FILE;
                            }
                        } else {
                            stream << MESSAGE_14 << Qt::endl;
                            serial.close();
                            return ERR_UNRECOGNIZED_FILE;
                        }
                        if(!command.compare("writer")) {
                            data.append("L");
                        }
                        if(!uartSendAddrRange(&serial, "WR", "0000", QString::asprintf("%04X", data.count()))) {
                            if(sendData(PROGRESS_VISIBLE, &serial, data, logType)) {
                                stream << MESSAGE_4 << Qt::endl;
                                serial.close();
                                return ERR_NOT_SEND_ANY_DATA;
                            }
                        } else {
                            stream << MESSAGE_5 << Qt::endl;
                            serial.close();
                            return ERR_BOARD_NOT_RESPONDING;
                        }
                    }
                }
                return ERR_OK;
            } else {
                stream << MESSAGE_1 << Qt::endl;
                serial.close();
                return ERR_NO_OPERATION_SELECTED;
            }
            stream << "OK: Reading memory." << Qt::endl;
            serial.close();
        }
    } else if(commandIsSet && dataTypeIsSet && comPortIsSet && baudRateIsSet && startIsSet && endIsSet) {
        serial.setPortName(comPort);
        serial.setBaudRate(baudRate.toInt());
        if (!serial.open(QIODevice::ReadWrite)) {
            stream << MESSAGE_7 << Qt::endl;
            return ERR_CAN_NOT_OPPEN_COM_PORT;
        } else {
            if(!command.compare("read")) {
                if(!dataType.compare("FLASH")) {
                    if(!uartSendAddrRange(&serial, "RF", start, end)) {
                        QString data;
                        bool ok = false;
                        if(!uartReadBytes(true, logType, end.toInt(&ok, 16) - start.toInt(&ok, 16), &serial, &data)) {
                            data = data.remove(QRegExp("^((?:\r?\n|\r))+"));
                            data = data.replace("\n\r", "\r");
                            QStringList list = data.split("\r", QString::SkipEmptyParts);
                            for(QString str : list) {
                                std::string conv = str.toStdString();
                                stream << conv.c_str() << Qt::endl;
                            }
                            stream << "OK: Reading FLASH memory." << Qt::endl;
                        } else {
                            stream << MESSAGE_4 << Qt::endl;
                            serial.close();
                            return ERR_NOT_SEND_ANY_DATA;
                        }
                    } else {
                        stream << MESSAGE_5 << Qt::endl;
                        serial.close();
                        return ERR_BOARD_NOT_RESPONDING;
                    }
                } else if(!dataType.compare("EEPROM")) {
                    if(!uartSendAddrRange(&serial, "RE", start, end)) {
                        QString data;
                        bool ok = false;
                        if(!uartReadBytes(true, logType, end.toInt(&ok, 16) - start.toInt(&ok, 16), &serial, &data)) {
                            data = data.remove(QRegExp("^((?:\r?\n|\r))+"));
                            data = data.replace("\n\r", "\r");
                            QStringList list = data.split("\r", QString::SkipEmptyParts);
                            for(QString str : list) {
                                std::string conv = str.toStdString();
                                stream << conv.c_str() << Qt::endl;
                            }
                            stream << "OK: Reading EEPROM memory." << Qt::endl;
                        } else {
                            stream << MESSAGE_4 << Qt::endl;
                            serial.close();
                            return ERR_NOT_SEND_ANY_DATA;
                        }
                    } else {
                        stream << MESSAGE_5 << Qt::endl;
                        serial.close();
                        return ERR_BOARD_NOT_RESPONDING;
                    }

                } else if(!dataType.compare("RAM")) {
                    if(!uartSendAddrRange(&serial, "RR", start, end)) {
                        QString data;
                        bool ok = false;
                        if(!uartReadBytes(true, logType, end.toInt(&ok, 16) - start.toInt(&ok, 16), &serial, &data)) {
                            data = data.remove(QRegExp("^((?:\r?\n|\r))+"));
                            data = data.replace("\n\r", "\r");
                            QStringList list = data.split("\r", QString::SkipEmptyParts);
                            for(QString str : list) {
                                std::string conv = str.toStdString();
                                stream << conv.c_str() << Qt::endl;
                            }
                            stream << "OK: Reading RAM memory." << Qt::endl;
                        } else {
                            stream << MESSAGE_4 << Qt::endl;
                            serial.close();
                            return ERR_NOT_SEND_ANY_DATA;
                        }
                    } else {
                        stream << MESSAGE_5 << Qt::endl;
                        serial.close();
                        return ERR_BOARD_NOT_RESPONDING;
                    }

                } else {
                    stream << MESSAGE_8 << Qt::endl;
                    serial.close();
                    return ERR_INVALID_MEMORY_TYPE;
                }
            } else if(!command.compare("write")) {

            } else {
                stream << MESSAGE_1 << Qt::endl;
                serial.close();
                return ERR_NO_OPERATION_SELECTED;
            }
            serial.close();
            return 0;
        }
    } else if(commandIsSet && !command.compare("getcomlist")) {
        QList<QSerialPortInfo> list = QSerialPortInfo::availablePorts();
        for(QSerialPortInfo portInfo: list) {
#ifdef Q_OS_LINUX
            stream << "/dev/" <<portInfo.portName() << Qt::endl;
#elif defined(Q_OS_WIN32)
            stream << portInfo.portName() << endl;
#endif
        }
        return 0;
    } else if(commandIsSet && comPortIsSet && !command.compare("getbaudrate")) {
        QStringList list = BAUD_RATE_LIST;
        for(QString baud: list) {
            stream << baud << Qt::endl;
        }
        return 0;
    } else {
        stream << MESSAGE_2 << Qt::endl;
        return ERR_INCORECT_ARGUMENT;
    }

    return app.exec();
}
