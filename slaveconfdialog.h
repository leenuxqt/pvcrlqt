#ifndef SLAVECONFDIALOG_H
#define SLAVECONFDIALOG_H

#include <QDialog>
#include <QSerialPort>

#define SPLIT_CHANNEL_SLAVE ("@")

namespace Ui {
class SlaveConfDialog;
}

/****
struct Settings {
    int parity = QSerialPort::EvenParity;
    int baud = QSerialPort::Baud9600;
    int dataBits = QSerialPort::Data8;
    int stopBits = QSerialPort::OneStop;
    int responseTime = 1000;
    int numberOfRetries = 3;
};
******/

enum ModbusConnection {
    Serial,
    Tcp
};

struct SlaveNodeConfig
{
    char unSlaveNo=0;
    int startAddress=0;
    int numberOfEntries=0;

    unsigned int unAddrP=0;
    unsigned int unAddrQ=0;
    unsigned int unAddrU=0;
    unsigned int unAddrI=0;

    QString floatType="dcba";
    QString channelName="/dev/ttyS0";

    SlaveNodeConfig &operator = (const SlaveNodeConfig &other)
    {
        if( this!=&other )
        {
            this->unSlaveNo = other.unSlaveNo;
            this->startAddress = other.startAddress;
            this->numberOfEntries = other.numberOfEntries;

            this->unAddrP = other.unAddrP;
            this->unAddrQ = other.unAddrQ;
            this->unAddrU = other.unAddrU;
            this->unAddrI = other.unAddrI;

            this->floatType = other.floatType;
            this->channelName = other.channelName;
        }
        return *this;
    }
};

struct SlaveConfig
{
    SlaveNodeConfig nodeCfg;

    QString strImg;

    QString strUnitP="W";
    QString strUnitQ="Var";
    QString strUnitU="V";
    QString strUnitI="A";

    SlaveConfig &operator=(const SlaveConfig &other)
    {
        if( this!=&other ) {
            this->nodeCfg = other.nodeCfg;

            this->strImg = other.strImg;

            this->strUnitP = other.strUnitP;
            this->strUnitQ = other.strUnitQ;
            this->strUnitU = other.strUnitU;
            this->strUnitI = other.strUnitI;
        }
        return *this;
    }
};

struct ChannelConfig {
    ModbusConnection channelType;
    QString channelName;
    int respondTimeout;
    QList<SlaveConfig> slaveList;

    //TODO
    //    operator =()
    //    {

    //    }
};

class SlaveConfDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SlaveConfDialog(QWidget *parent = nullptr);
    ~SlaveConfDialog();

private:
    Ui::SlaveConfDialog *ui;
};

#endif // SLAVECONFDIALOG_H
