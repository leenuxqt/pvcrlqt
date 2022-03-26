
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "chart.h"

#ifndef WIN32
#include "rtumaster.h"
#endif

#include "slaveinfoframe.h"

#include "cmodbuscontroller.h"

#include <QtCharts/QChartView>
#include <QSplitter>
#include <QVBoxLayout>
#include <QFile>
#include <QGroupBox>

#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>

#include <QtDebug>


Q_DECLARE_METATYPE(QModbusDataUnit)

QT_CHARTS_USE_NAMESPACE

MainWindow::MainWindow(int portNo, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , chart(0)
    , chartView(0)
{
    ui->setupUi(this);

    QList<int> sizes;
    sizes << 320 << 80;
    ui->splitter->setSizes( sizes );

    bool res = loadJsonConfig(portNo);

    initPage();

//    if( 0==initRtuMaster(portNo) )
//    {
//        chart->startTimer();
//    }

    ChannelConfig serialChannelcfg;
    serialChannelcfg.channelType = ModbusConnection::Serial;
    serialChannelcfg.respondTimeout = 80;
    serialChannelcfg.channelName="COM1";

    QList<SlaveConfig> serialSlaveList;

    SlaveConfig dev3Cfg;
    dev3Cfg.nodeCfg.unSlaveNo = 3;
    dev3Cfg.nodeCfg.startAddress = 0;
    dev3Cfg.nodeCfg.numberOfEntries = 13;
    dev3Cfg.nodeCfg.unAddrP = 6;
    dev3Cfg.nodeCfg.unAddrQ = 8;
    dev3Cfg.nodeCfg.unAddrU = 10;

    serialSlaveList.append( dev3Cfg );

    SlaveConfig dev4Cfg;
    dev4Cfg.nodeCfg.unSlaveNo = 4;
    dev4Cfg.nodeCfg.startAddress = 0;
    dev4Cfg.nodeCfg.numberOfEntries = 14;
    dev4Cfg.nodeCfg.unAddrP = 6;
    dev4Cfg.nodeCfg.unAddrQ = 8;
    dev4Cfg.nodeCfg.unAddrU = 10;

    serialSlaveList.append( dev4Cfg );
    serialChannelcfg.slaveList = serialSlaveList;

//    auto fun3 = [](const QModbusDataUnit &mdu) { qDebug() <<mdu.valueCount(); }; //lamda表达式

    int id = qRegisterMetaType<QModbusDataUnit>();


    CModbusController *pController = new CModbusController(serialChannelcfg,this);
    connect( pController, &CModbusController::sigReciveDataUnit, this,
            [this](const int serverAddress, const QModbusDataUnit &dataUnit){
                auto controller = qobject_cast<CModbusController*>(sender());
                _parseModbusDataUnit( controller ,serverAddress, dataUnit);  } );
//    m_hashController[serialChannelcfg.channelName] = pController;

    //=======================
    ChannelConfig tcpChannelcfg;
    tcpChannelcfg.channelType = ModbusConnection::Tcp;
    tcpChannelcfg.respondTimeout = 80;
    tcpChannelcfg.channelName="127.0.0.1:502";

    QList<SlaveConfig> tcpSlaveList;

    SlaveConfig dev5Cfg;
    dev5Cfg.nodeCfg.unSlaveNo = 5;
    dev5Cfg.nodeCfg.startAddress = 0;
    dev5Cfg.nodeCfg.numberOfEntries = 25;
    dev5Cfg.nodeCfg.unAddrP = 6;
    dev5Cfg.nodeCfg.unAddrQ = 8;
    dev5Cfg.nodeCfg.unAddrU = 10;

    tcpSlaveList.append( dev5Cfg );

    SlaveConfig dev6Cfg;
    dev6Cfg.nodeCfg.unSlaveNo = 6;
    dev6Cfg.nodeCfg.startAddress = 0;
    dev6Cfg.nodeCfg.numberOfEntries = 16;
    dev6Cfg.nodeCfg.unAddrP = 6;
    dev6Cfg.nodeCfg.unAddrQ = 8;
    dev6Cfg.nodeCfg.unAddrU = 10;

    tcpSlaveList.append( dev6Cfg );

    tcpChannelcfg.slaveList = tcpSlaveList;

    pController = new CModbusController(tcpChannelcfg,this);
    connect( pController, &CModbusController::sigReciveDataUnit, this,
            [this](const int serverAddress, const QModbusDataUnit &dataUnit) {
                auto controller = qobject_cast<CModbusController*>(sender());
                _parseModbusDataUnit(controller, serverAddress, dataUnit);  } );

}

MainWindow::~MainWindow()
{
#ifdef __linux
    releaseRtuMaster();
#endif

    delete ui;
}

bool MainWindow::loadJsonConfig(const int portNo )
{
    QString fileName = QString("/configport_%1.json").arg(portNo);
    QString  appPath = QApplication::applicationDirPath();

    qDebug() << appPath+fileName;

    QFile jsonFile( appPath + fileName );
    if( jsonFile.open(QIODevice::ReadOnly) )
    {
        QByteArray content = jsonFile.readAll();

        QJsonDocument jsonDoc(QJsonDocument::fromJson(content));

        if( jsonDoc.isObject() )
        {
            QJsonValue activePowerInfo = jsonDoc.object().value("activatepower");
            if( activePowerInfo.isArray() )
            {
                QJsonArray tfList = activePowerInfo.toArray();

                for(int i=0;i<tfList.size();i++)
                {
                    QJsonValue tfInfo = tfList.at(i);
                    if( tfInfo.isObject() )
                    {
                        _parseTfObject(tfInfo.toObject());
                    }
                }
            }
        }


        jsonFile.close();
        return true;
    }
    else
        qDebug() << "oopen failed";

    return false;
}


bool MainWindow::_parseTfObject(const QJsonObject &tf)
{
    if( tf.isEmpty() )
        return false;

    
    QGroupBox *pTfGroupBox = new QGroupBox(tf.value("desc").toString(), this);
    //qDebug() << tf.value("desc").toString() ;
    ui->horizontalLayout_ActivePower->addWidget( pTfGroupBox );
    QHBoxLayout *horizontalLayout_tf = new QHBoxLayout(pTfGroupBox);

    QJsonValue boxData = tf.value("children");
    if( boxData.isArray() )
    {
        QJsonArray boxList = boxData.toArray();
        for(int i=0;i<boxList.size();i++)
        {
            QJsonValue boxInfo = boxList.at(i);
            if( boxInfo.isObject() )
                _parseBoxObject(boxInfo.toObject(), horizontalLayout_tf);
        }
    }

    return true;
    
}


SlaveConfig _makeSlaveConfig(const QJsonObject &jsobj)
{
    SlaveConfig _cfg;

    _cfg.nodeCfg.channelName = jsobj.value("channel").toString();
    _cfg.nodeCfg.unSlaveNo = jsobj.value("slaveno").toInt();
    _cfg.nodeCfg.unAddrP = jsobj.value("AddrP").toInt();
    _cfg.nodeCfg.unAddrQ = jsobj.value("AddrQ").toInt();
    _cfg.nodeCfg.unAddrU = jsobj.value("AddrU").toInt();
    _cfg.nodeCfg.unAddrI = jsobj.value("AddrI").toInt();

    _cfg.strUnitP = jsobj.value("UnitP").toString();
    _cfg.strUnitQ= jsobj.value("UnitQ").toString();
    _cfg.strUnitU = jsobj.value("UnitU").toString();
    _cfg.strUnitI = jsobj.value("UnitI").toString();

    _cfg.strImg = jsobj.value("image").toString();
    _cfg.nodeCfg.floatType = jsobj.value("floattype").toString();

    return _cfg;
}

bool MainWindow::_parseBoxObject(const QJsonObject &box, QBoxLayout *parentLayout)
{
    if( box.isEmpty() || !parentLayout )
        return false;


    QGroupBox *pBoxGroupBox = new QGroupBox(box.value("desc").toString(), this);
    //qDebug() << tf.value("desc").toString() ;
    parentLayout->addWidget( pBoxGroupBox );
    QVBoxLayout *pVerLayout_box = new QVBoxLayout();
    pBoxGroupBox->setLayout( pVerLayout_box );

    SlaveInfoFrame *pBoxFrame = new SlaveInfoFrame(pBoxGroupBox);
    pBoxFrame->setConfig( _makeSlaveConfig(box) );    
    pVerLayout_box->addWidget( pBoxFrame );

    m_hasNameToSlaveInfo[ pBoxFrame->objectName() ] = pBoxFrame;

    QJsonValue inverterData = box.value("children");
    if( inverterData.isArray() )
    {
        QJsonArray inverterList = inverterData.toArray();
        for(int i=0;i<inverterList.size();i++)
        {
            QJsonValue inverterInfo = inverterList.at(i);
            if( inverterInfo.isObject() )
                _parseInverterObject(inverterInfo.toObject(), pVerLayout_box);
        }
    }

    return true;
}

bool MainWindow::_parseInverterObject(const QJsonObject &inverter, QBoxLayout *parentLayout)
{
    if( inverter.isEmpty() || !parentLayout )
        return false;


    QGroupBox *pInverterGroupBox = new QGroupBox(inverter.value("desc").toString(), this);
    parentLayout->addWidget( pInverterGroupBox );

    QVBoxLayout *pVerLayout_Inverter = new QVBoxLayout(pInverterGroupBox);

    SlaveInfoFrame *pInverterFrame = new SlaveInfoFrame(pInverterGroupBox);
    pInverterFrame->setConfig( _makeSlaveConfig(inverter) );
    pVerLayout_Inverter->addWidget( pInverterFrame );

    m_hasNameToSlaveInfo[ pInverterFrame->objectName() ] = pInverterFrame;

    return true;
}

void MainWindow::initPage()
{
    chart = new Chart;
//    chart->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    chart->setTitle("Dynamic spline chart");
    chart->legend()->hide();
    chart->setAnimationOptions(QChart::AllAnimations);

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    ui->verticalLayoutVol->addWidget( chartView );
}

void MainWindow::_parseModbusDataUnit(CModbusController *controller, const int serverAddress, const QModbusDataUnit &dataUnit)
{
    if( controller ) {
        qDebug() << "controller name=" << controller->getControllerName() << " address=" << serverAddress <<" data unit count=" << dataUnit.valueCount();
        QString strKeyName = controller->getControllerName() + SPLIT_CHANNEL_SLAVE + QString::number(serverAddress);
        SlaveInfoFrame *pInfoFrame = m_hasNameToSlaveInfo.value(strKeyName, nullptr);
        if( pInfoFrame ) {
            pInfoFrame->parseDataUnit( dataUnit );
        } else {
            qDebug() << "can not find the frame by key " << strKeyName;
        }

    } else {
        qDebug() << "controller error";
    }


}
