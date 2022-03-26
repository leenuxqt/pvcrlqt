#ifndef CMODBUS_CHANNEL_H
#define CMODBUS_CHANNEL_H

#include "slaveconfdialog.h"

#include <QObject>
#include <QModbusDataUnit>
#include <QModbusDevice>

class QModbusReply;
class QModbusClient;

class QTimer;



class CModbusChannel : public QObject
{
    Q_OBJECT
public:
    explicit CModbusChannel(const ChannelConfig &channelcfg, QObject *parent = nullptr);
    ~CModbusChannel();
public slots:
    void initAfterThreadStart();

private:
    bool _initModbusConnection(  );
//    QList<QModbusDataUnit> _makeReadRequestList() const;
    void _sendReadRequest();

private slots:
    void _slotChannelErrorOccurred(QModbusDevice::Error error);
    void _slotChannelStateChanged(QModbusDevice::State state);

    void _slotChannelReadReady();

    void _slotRegularTimeOut();
    void _slotIdleTimeOut();

private:
    QList<SlaveConfig>::const_iterator currentSlaveIt ;
    int sendnum;
    int deletenum;
    QModbusReply *m_pLastRequest;
    QModbusClient *m_pModbusCommDevice;

    QList<char> m_lstSlaveNo;
    QTimer *m_pRegularTimer;
    QTimer *m_pIdleTimer;

    ChannelConfig m_ChannelCfg;

signals:
    void sigReceiveMDU(const int serverAddress, const QModbusDataUnit &mdu);
};

#endif // CMODBUS_CHANNEL_H
