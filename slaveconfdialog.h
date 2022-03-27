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
    int replyTime=20;

    QString floatType="dcba";
    QString channelName="/dev/ttyS0";

    //TODO
//    SlaveNodeConfig &operator = (const SlaveNodeConfig &other)
//    {
//        if( this!=&other )
//        {
//            this->unSlaveNo = other.unSlaveNo;
//            this->startAddress = other.startAddress;
//            this->numberOfEntries = other.numberOfEntries;

//            if( other.floatType.size() )
//                this->floatType = other.floatType;

//            if( other.channelName.size() )
//                this->channelName = other.channelName;
//        }
//        return *this;
//    }
};

struct SlaveItemConfig
{
    QString strLabel="Label";
    QString strUnit="Unit";
    int nAddr=0;

    //TODO
//    SlaveItemConfig &operator = (const SlaveItemConfig &other)
//    {
//        if( this!=&other )
//        {
//            if( other.strLabel.size() )
//                this->strLabel = other.strLabel;
//            this->strUnit = other.strUnit;
//            this->nAddr = other.nAddr;
//        }
//        return *this;
//    }
};

struct SlaveConfig
{
    SlaveNodeConfig nodeCfg;

    QString strImg;

    QList<SlaveItemConfig> lstItem;

    SlaveConfig &operator=(const SlaveConfig &other)
    {
        if( this!=&other ) {
            this->nodeCfg = other.nodeCfg;

            this->strImg = other.strImg;

            //how to copy?
            this->lstItem = other.lstItem;
        }
        return *this;
    }
};

struct ChannelConfig {
    ModbusConnection channelType;
    QString channelName;
    int respondTimeout;
    QList<SlaveNodeConfig> slaveList;

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
