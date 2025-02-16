#include "cmodbuschannel.h"

#include <QModbusRtuSerialMaster>
#include <QModbusTcpClient>

#include <QTimer>
#include <QDebug>
#include <QThread>
#include <QCoreApplication>
#include <QDateTime>
#include <QUrl>

CModbusChannel::CModbusChannel(const ChannelConfig &channelcfg,QObject *parent)
    : QObject(parent), m_pLastRequest(nullptr), m_pModbusCommDevice(nullptr)
{
    m_ChannelCfg = channelcfg;
    currentSlaveIt = m_ChannelCfg.slaveList.constBegin();

    sendnum = 0;
    deletenum = 0;

}

CModbusChannel::~CModbusChannel()
{
    if (m_pModbusCommDevice)
        m_pModbusCommDevice->disconnectDevice();
    delete m_pModbusCommDevice;
}


void CModbusChannel::initAfterThreadStart()
{

    m_pRegularTimer = new QTimer();
    connect( m_pRegularTimer, SIGNAL(timeout()),
             this, SLOT(_slotRegularTimeOut()) );
    m_pRegularTimer->setInterval( 200 );

    m_pIdleTimer = new QTimer();
    connect(m_pIdleTimer, SIGNAL(timeout()),
            this, SLOT( _slotIdleTimeOut() ) );
    m_pIdleTimer->setInterval(0);

    auto type = m_ChannelCfg.channelType;
    if( Serial==type )
    {
        m_pModbusCommDevice = new QModbusRtuSerialMaster();
        m_pModbusCommDevice->setConnectionParameter( QModbusDevice::SerialPortNameParameter, m_ChannelCfg.channelName);
        m_pModbusCommDevice->setConnectionParameter( QModbusDevice::SerialParityParameter, QSerialPort::NoParity);
        m_pModbusCommDevice->setConnectionParameter( QModbusDevice::SerialBaudRateParameter, QSerialPort::Baud9600);
        m_pModbusCommDevice->setConnectionParameter( QModbusDevice::SerialDataBitsParameter, QSerialPort::Data8);
        m_pModbusCommDevice->setConnectionParameter( QModbusDevice::SerialStopBitsParameter, QSerialPort::OneStop );
    }
    else if( Tcp==type )
    {
        const QUrl url = QUrl::fromUserInput(m_ChannelCfg.channelName);
        m_pModbusCommDevice = new QModbusTcpClient();
        m_pModbusCommDevice->setConnectionParameter(QModbusDevice::NetworkPortParameter,  url.port());
        m_pModbusCommDevice->setConnectionParameter(QModbusDevice::NetworkAddressParameter, url.host());
    }

    //The timeout is used by the client to determine how long it waits for a response from the server
    //If the response is not received within the required timeout, the TimeoutError is set.
    m_pModbusCommDevice->setTimeout( m_ChannelCfg.respondTimeout );

    connect( m_pModbusCommDevice, SIGNAL(errorOccurred(QModbusDevice::Error)),
            this, SLOT(_slotChannelErrorOccurred(QModbusDevice::Error)));

    connect( m_pModbusCommDevice, SIGNAL(stateChanged(QModbusDevice::State)),
             this, SLOT(_slotChannelStateChanged(QModbusDevice::State)) );

    if( m_pRegularTimer)
        m_pRegularTimer->start();

//        if( m_pIdleTimer )
//            m_pIdleTimer->start();

}

bool CModbusChannel::_initModbusConnection( )
{
    if( m_pModbusCommDevice->state() != QModbusDevice::ConnectedState )
    {
        if( !m_pModbusCommDevice->connectDevice() )
        {
            qDebug() << m_ChannelCfg.channelName + tr(" ====Connect failed: ") + m_pModbusCommDevice->errorString();
            return 0;
        }
    }

    return 1;
}

void CModbusChannel::_slotChannelErrorOccurred(QModbusDevice::Error error)
{

}


void CModbusChannel::_slotChannelStateChanged(QModbusDevice::State state)
{
//    bool connected = (state!=QModbusDevice::UnconnectedState);
}


void CModbusChannel::_sendReadRequest()
{
    if(!m_pModbusCommDevice)
        return ;

    if( !_initModbusConnection() )
        return ;

    if (currentSlaveIt!=m_ChannelCfg.slaveList.constEnd() )
    {
        const SlaveConfig &cfg = *currentSlaveIt;

        QModbusDataUnit anUnit(QModbusDataUnit::HoldingRegisters, cfg.nodeCfg.startAddress, cfg.nodeCfg.numberOfEntries);
        // m_pModbusCommDevice->setNumberOfRetries( 2 );

        //sendReadRequest is 异步??
        if( auto *reply = m_pModbusCommDevice->sendReadRequest( anUnit, cfg.nodeCfg.unSlaveNo ) )
        {
            sendnum++;
            if( !reply->isFinished() )
            {
                connect( reply, SIGNAL( finished() ),
                         this, SLOT( _slotChannelReadReady() ), Qt::DirectConnection );
            }
            else
            {
                qDebug() << "reply yes yes yes finish";
                delete reply;
                ++deletenum;
            }

            //force to send it??
            //https://blog.csdn.net/qq_23516957/article/details/103821423
            //while( !reply->isFinished() )
            //{
                //QCoreApplication::processEvents( QEventLoop::ExcludeUserInputEvents, 20 );
            //}
        }
        else
        {
            qDebug() << tr("Read error ") << m_pModbusCommDevice->errorString();
        }

        //msleep will block all the other timer
        //QThread::msleep(500);
        //std::this_thread::sleep_for( std::chrono::millisends(5) );
    }
}

void CModbusChannel::_slotChannelReadReady()
{
    auto reply = qobject_cast<QModbusReply *>(sender());
    if( !reply )
        return;

    if( reply->error()==QModbusDevice::NoError )
    {
        const QModbusDataUnit unit = reply->result();
        emit sigReceiveMDU(reply->serverAddress(), unit );

//        for(uint i=0;i<unit.valueCount();i++)
//        {
//            int valueBase = unit.registerType()<=QModbusDataUnit::Coils ? 10:16;
//            const QString entry = tr("Arress: %1, Value: %2").arg(unit.startAddress()).arg( QString::number(unit.value(i), valueBase) );
//            qDebug() << "index " << i << "Value " << entry;
//        }
    }
    else if( reply->error()==QModbusDevice::ProtocolError )
    {
        qDebug() << tr("Read response error: %1 (Modbus exception: %x%2")
                    .arg(reply->errorString())
                    .arg( reply->rawResult().exceptionCode(),-1,16 );
    }
    else
    {
        qDebug() << tr("Read response error: %1 (code: 0x%2)")
                    .arg( reply->errorString() )
                    .arg( reply->error(), -1, 16 );
    }

    reply->deleteLater();

    ++deletenum;

//    qDebug() << "_slotChannelReadReady >>>>";
}

void CModbusChannel::_slotRegularTimeOut()
{
//    qDebug() << "=======_slotRegularTimeOut" ;
//    qDebug() << "sendnum==" << sendnum << " deletenum==" << deletenum;
    _sendReadRequest();

    ++currentSlaveIt;
    //loop again
    if( currentSlaveIt==m_ChannelCfg.slaveList.constEnd() )
        currentSlaveIt = m_ChannelCfg.slaveList.constBegin();
}

void CModbusChannel::_slotIdleTimeOut()
{
    qDebug() << "**********_slotIdleTimeOut" << QTime::currentTime();
    _sendReadRequest();
}












