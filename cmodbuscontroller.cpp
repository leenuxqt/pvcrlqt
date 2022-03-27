#include "cmodbuscontroller.h"

#include "cmodbuschannel.h"

CModbusController::CModbusController(const ChannelConfig &channelcfg, QObject *parent) : QObject(parent)
{
    controllerName = channelcfg.channelName;
    m_pChannel = new CModbusChannel(channelcfg);
    m_pChannel->moveToThread( &channelThread );

    connect( &channelThread, &QThread::started, m_pChannel, &CModbusChannel::initAfterThreadStart );
    connect( &channelThread, &QThread::finished, m_pChannel, &QObject::deleteLater );

    connect( m_pChannel, &CModbusChannel::sigReceiveMDU, this, &CModbusController::sigReciveDataUnit, Qt::QueuedConnection);//test Qt::QueuedConnection

//    connect(this, &CModbusController::operate, channel, &CModbusChannel::doWork);
//    connect(worker, &Worker::resultReady, this, &Controller::handleResults);
    channelThread.start();
}

CModbusController::~CModbusController()
{
    channelThread.quit();
    channelThread.wait();
}


