
#include "mainwindow.h"
#include "ui_mainwindow.h"

#ifndef WIN32
#include "rtumaster.h"
#endif

#include "cmodbuscontroller.h"

#include "slaveinfowidget.h"
#include "voltagechartform.h"

#include <QSplitter>
#include <QVBoxLayout>
#include <QFile>
#include <QGroupBox>
#include <QTabWidget>

#include <QJsonDocument>
#include <QJsonValue>
#include <QJsonArray>
#include <QJsonObject>

#include <QtDebug>


Q_DECLARE_METATYPE(QModbusDataUnit)


MainWindow::MainWindow(int portNo, QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QList<int> sizes;
    sizes << 320 << 80;
    ui->splitter->setSizes( sizes );

    bool res = loadJsonConfig(portNo);


    int id = qRegisterMetaType<QModbusDataUnit>();

    QHash<QString, QList<SlaveNodeConfig> >::const_iterator i = m_hashChannelToSlaveList.constBegin();
    while (i != m_hashChannelToSlaveList.constEnd()) {

        ChannelConfig channelcfg;
        channelcfg.channelName = i.key();
        channelcfg.channelType = channelcfg.channelName.contains(":") ?
                    ModbusConnection::Tcp : ModbusConnection::Serial;


        channelcfg.slaveList = i.value();

        // calc the max timeout config in slave list of this channel
        auto compareFunc = [](const SlaveNodeConfig &v1, const SlaveNodeConfig &v2)->bool { return v1.replyTime<v2.replyTime ;};
        int timeoutVal = std::max_element(channelcfg.slaveList.begin(),channelcfg.slaveList.end(), compareFunc )->replyTime;

        qDebug() << "timeoutVal==" << timeoutVal;
        channelcfg.respondTimeout = timeoutVal;

        CModbusController *pController = new CModbusController(channelcfg,this);
        connect( pController, &CModbusController::sigReciveDataUnit, this,
                [this](const int serverAddress, const QModbusDataUnit &dataUnit){
                    auto controller = qobject_cast<CModbusController*>(sender());
                    _parseModbusDataUnit( controller ,serverAddress, dataUnit);  } );

        ++i;
    }

    initPage();

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


SlaveConfig _makeSlaveConfig(const QJsonObject &jsobj)
{
    SlaveConfig _cfg;

    /**
                "channel":"127.0.0.1:502",
                "slaveno":24,
                "readlength":12,
                "floattype":"dcba",
    **/

    QString strChannelName = jsobj.value("channel").toString();
    QString strFloatType = jsobj.value("floattype").toString("dcba");
    int unSlaveNo = jsobj.value("slaveno").toInt(0);
    int startAddress = jsobj.value("startAddress").toInt(0);
    int readLength = jsobj.value("readlength").toInt(0);
    int replyTime = jsobj.value("replytime").toInt(20);

    _cfg.nodeCfg.channelName = strChannelName;
    _cfg.nodeCfg.floatType = strFloatType;
    _cfg.nodeCfg.unSlaveNo = unSlaveNo;
    _cfg.nodeCfg.startAddress = startAddress;
    _cfg.nodeCfg.numberOfEntries = readLength;
    _cfg.nodeCfg.replyTime = replyTime;

    _cfg.strImg = jsobj.value("image").toString();

    QJsonValue itemData = jsobj.value("dataitem");
    if( itemData.isArray() )
    {
        QJsonArray items = itemData.toArray();
        for(int i=0;i<items.size();i++)
        {
            QJsonObject itemObj = items.at(i).toObject();
            SlaveItemConfig itemcfg;
            itemcfg.strLabel = itemObj.value("Label").toString("Label");
            itemcfg.strUnit = itemObj.value("Unit").toString("Unit");
            itemcfg.nAddr = itemObj.value("Addr").toInt(0);
            _cfg.lstItem.append( itemcfg );
        }
    }

    return _cfg;
}

bool MainWindow::_parseTfObject(const QJsonObject &tf)
{
    if( tf.isEmpty() )
        return false;

    
    QGroupBox *pTfGroupBox = new QGroupBox(tf.value("desc").toString(), this);
    //qDebug() << tf.value("desc").toString() ;
    ui->horizontalLayout_ActivePower->addWidget( pTfGroupBox );
    QHBoxLayout *horizontalLayout_tf = new QHBoxLayout(pTfGroupBox);

    VoltageChartForm *pValChartForm = new VoltageChartForm(this);
    ui->tabWidget_tf->addTab(pValChartForm, tf.value("desc").toString());

    SlaveConfig tfCfg = _makeSlaveConfig(tf);
    pValChartForm->setConfig( tfCfg );

    m_hasNameToInfoFrame[ pValChartForm->objectName() ] = pValChartForm;
    m_hashChannelToSlaveList[ tfCfg.nodeCfg.channelName ].append(tfCfg.nodeCfg);

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



bool MainWindow::_parseBoxObject(const QJsonObject &box, QBoxLayout *parentLayout)
{
    if( box.isEmpty() || !parentLayout )
        return false;


    QGroupBox *pBoxGroupBox = new QGroupBox(box.value("desc").toString(), this);
    //qDebug() << tf.value("desc").toString() ;
    parentLayout->addWidget( pBoxGroupBox );
    QVBoxLayout *pVerLayout_box = new QVBoxLayout();
    pBoxGroupBox->setLayout( pVerLayout_box );

//    SlaveInfoFrame *pBoxFrame = new SlaveInfoFrame(pBoxGroupBox);
    SlaveInfoWidget *pBoxWidget = new SlaveInfoWidget(pBoxGroupBox);
    SlaveConfig boxCfg = _makeSlaveConfig(box);
    pBoxWidget->setConfig( boxCfg );
    pVerLayout_box->addWidget( pBoxWidget );

    m_hasNameToInfoFrame[ pBoxWidget->objectName() ] = pBoxWidget;

    m_hashChannelToSlaveList[ boxCfg.nodeCfg.channelName ].append(boxCfg.nodeCfg);

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

//    SlaveInfoFrame *pInverterFrame = new SlaveInfoFrame(pInverterGroupBox);
    SlaveInfoWidget *pInverterWidget = new SlaveInfoWidget(pInverterGroupBox);
    SlaveConfig inverterCfg = _makeSlaveConfig(inverter);
    pInverterWidget->setConfig( inverterCfg );
    pVerLayout_Inverter->addWidget( pInverterWidget );

    m_hasNameToInfoFrame[ pInverterWidget->objectName() ] = pInverterWidget;

    m_hashChannelToSlaveList[ inverterCfg.nodeCfg.channelName].append(inverterCfg.nodeCfg);

    return true;
}

void MainWindow::initPage()
{

}

void MainWindow::_parseModbusDataUnit(CModbusController *controller, const int serverAddress, const QModbusDataUnit &dataUnit)
{
    if( controller ) {
        qDebug() << "_parseModbusDataUnit controller name=" << controller->getControllerName() << " address=" << serverAddress <<" data unit count=" << dataUnit.valueCount();
        QString strKeyName = controller->getControllerName() + SPLIT_CHANNEL_SLAVE + QString::number(serverAddress);
        auto pInfoFrame = m_hasNameToInfoFrame.value(strKeyName, nullptr);
        if( pInfoFrame ) {
            pInfoFrame->parseDataUnit( dataUnit );
        } else {
            qDebug() << "can not find the frame by key " << strKeyName;
        }

    } else {
        qDebug() << "controller error";
    }


}
