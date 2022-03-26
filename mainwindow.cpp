
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "chart.h"

#ifndef WIN32
#include "rtumaster.h"
#endif

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

    int id = qRegisterMetaType<QModbusDataUnit>();

    QHash<QString, QList<SlaveNodeConfig> >::const_iterator i = m_hashChannelToSlaveList.constBegin();
    while (i != m_hashChannelToSlaveList.constEnd()) {

        ChannelConfig channelcfg;
        channelcfg.channelName = i.key();
        channelcfg.channelType = channelcfg.channelName.contains(":") ?
                    ModbusConnection::Tcp : ModbusConnection::Serial;
        channelcfg.respondTimeout = 80;//TODO

        channelcfg.slaveList = i.value();

        CModbusController *pController = new CModbusController(channelcfg,this);
        connect( pController, &CModbusController::sigReciveDataUnit, this,
                [this](const int serverAddress, const QModbusDataUnit &dataUnit){
                    auto controller = qobject_cast<CModbusController*>(sender());
                    _parseModbusDataUnit( controller ,serverAddress, dataUnit);  } );

        ++i;
    }

}

MainWindow::~MainWindow()
{
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

void MainWindow::appendToChannel(const QJsonObject &jsonSlaveObj)
{
    /**
            "channel":"/dev/ttyS1",
            "slaveno":11,
            "floattype":"dcba",
            "AddrP":1,"UnitP":"W",
            "AddrQ":2,"UnitQ":"Var",
            "AddrU":3,"UnitU":"V",
            "AddrI":4,"UnitI":"A",
    **/

    QString strChannelName = jsonSlaveObj.value("channel").toString();
    QString strFloatType = jsonSlaveObj.value("floattype").toString("dcba");
    int unSlaveNo = jsonSlaveObj.value("slaveno").toInt(0);
    int startAddress = jsonSlaveObj.value("startAddress").toInt(0);
    int readLength = jsonSlaveObj.value("readlength").toInt(0);
    int addrP = jsonSlaveObj.value("AddrP").toInt(0);
    int addrQ = jsonSlaveObj.value("AddrQ").toInt(0);
    int addrU = jsonSlaveObj.value("AddrU").toInt(0);
    int addrI = jsonSlaveObj.value("AddrI").toInt(0);

    if( unSlaveNo<1 || strChannelName.isEmpty() || readLength<1 )
        return ;

    SlaveNodeConfig slaveCfg;

    slaveCfg.channelName = strChannelName;
    slaveCfg.floatType = strFloatType;
    slaveCfg.unSlaveNo = unSlaveNo;
    slaveCfg.startAddress = startAddress;
    slaveCfg.numberOfEntries = readLength;

    slaveCfg.unAddrP = addrP;
    slaveCfg.unAddrQ = addrQ;
    slaveCfg.unAddrU = addrU;
    slaveCfg.unAddrI = addrI;

    m_hashChannelToSlaveList[strChannelName].append(slaveCfg);
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

    appendToChannel(box);

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

    appendToChannel(inverter);

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
