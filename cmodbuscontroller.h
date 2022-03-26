#ifndef CMODBUSCONTROLLER_H
#define CMODBUSCONTROLLER_H

#include <QObject>
#include <QThread>
#include <QModbusDevice>

#include "cmodbuschannel.h"

class CModbusController : public QObject
{
    Q_OBJECT
    QThread channelThread;
public:
    explicit CModbusController(const ChannelConfig &channelcfg, QObject *parent = nullptr);

    ~CModbusController();

    QString getControllerName()
    {
        return controllerName;
    }

signals:
    void operate(const QString &);
    void sigReciveDataUnit(const int serverAddress, const QModbusDataUnit &adu);

private:
    CModbusChannel *m_pChannel;
    QString controllerName;

};

#endif // CMODBUSCONTROLLER_H
