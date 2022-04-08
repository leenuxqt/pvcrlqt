
#include "mymodbusfunc.h"

#include "voltagechartform.h"
#include "ui_voltagechartform.h"

#include "chart.h"

#include <QtCharts/QChartView>
#include <QDebug>
#include <QLineEdit>

QT_CHARTS_USE_NAMESPACE

VoltageChartForm::VoltageChartForm(QWidget *parent) :
    BaseDataUnitWidget(parent), //QWidget(parent),
    ui(new Ui::VoltageChartForm),
    chart(0),
    chartView(0)
{
    ui->setupUi(this);
    _initChartPage();
}

VoltageChartForm::~VoltageChartForm()
{
    delete ui;
}

void VoltageChartForm::_initChartPage()
{
    chart = new Chart;
//    chart->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
//    chart->setTitle("Dynamic spline chart");
//    chart->legend()->hide();
//    chart->setAnimationOptions(QChart::AllAnimations);

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    ui->verticalLayoutVol->addWidget( chartView );
}

void VoltageChartForm::setConfig(const SlaveConfig &cfg)
{
    //TODO operator=
    _config = cfg;

    QString strFloatType = _config.nodeCfg.floatType;
    if( strFloatType.toLower()=="abcd" ) {
        changeFunc = my_modbus_get_float_abcd;
    } else if ( strFloatType.toLower()=="cdab" ){
        changeFunc = my_modbus_get_float_cdab;
    } else if ( strFloatType.toLower()=="dcba" ) {
        changeFunc = my_modbus_get_float_dcba;
    } else if (strFloatType.toLower()=="cdab") {
        changeFunc = my_modbus_get_float_cdab;
    } else {
        changeFunc = nullptr;
    }

    QString objName = _config.nodeCfg.channelName+SPLIT_CHANNEL_SLAVE+ QString::number(_config.nodeCfg.unSlaveNo);
    setObjectName( objName );
    qDebug() << "objName==" << objName;
    _updateChart();

}

void VoltageChartForm::_updateChart()
{
    for( int i=0; i<_config.lstItem.size();i++ )
    {
        const SlaveItemConfig &itemCfg = _config.lstItem.at(i);

        if( chart )
            chart->addSlaveSeries(itemCfg);

        m_hashAddrToValueEditor[itemCfg.nAddr] = new QLineEdit(this);
        m_hashAddrToValueEditor[itemCfg.nAddr]->setSizePolicy( QSizePolicy::Minimum, QSizePolicy::Preferred);
        m_hashAddrToValueEditor[itemCfg.nAddr]->setReadOnly( true );
        QString strLabel = tr("%1(%2)").arg(itemCfg.strLabel).arg(itemCfg.strUnit);
        ui->formLayout->addRow(strLabel,m_hashAddrToValueEditor[itemCfg.nAddr]);
    }

    //TODO start at here temp
    if( chart )
        chart->startTimer();
}

void VoltageChartForm::updateEditorRealValue( const int addr, const float itemValue )
{
    QLineEdit *editor = m_hashAddrToValueEditor[addr];
    if( editor ) {
        editor->setText( QString::number(itemValue, 'f', 2)  );
    }
}

void VoltageChartForm::updateEditorRealValue( const int addr, const int itemValue )
{
    QLineEdit *editor = m_hashAddrToValueEditor[addr];
    if( editor ) {
        editor->setText( QString::number(itemValue)  );
    }
}

void VoltageChartForm::parseDataUnit(const QModbusDataUnit &dataUnit)
{
    int dataSize = dataUnit.valueCount();
    if( dataSize<_config.nodeCfg.numberOfEntries ) {
        qDebug() << "modbus data unit valueCount too short real=" << dataSize << " configcount=" << _config.nodeCfg.numberOfEntries;
        return ;
    }

    if( nullptr==changeFunc ) {
        qDebug() << "please config the chang float value function type";
        return;
    }

    for( int i=0;i<_config.lstItem.size();i++)
    {
        int valueBase = dataUnit.registerType()<=QModbusDataUnit::Coils ? 10:16;
        const SlaveItemConfig &itemcfg = _config.lstItem.at(i);
        uint16 rawValue = dataUnit.value( itemcfg.nAddr ) ; //maybe nAddr is too large??
        if( valueBase==16 ) {
            float itemValue = changeFunc( &rawValue );
            updateEditorRealValue(itemcfg.nAddr, itemValue );

            if( chart )
                chart->updateSeries(itemcfg.nAddr, itemValue );
        } else {

            updateEditorRealValue(itemcfg.nAddr, rawValue );
            if( chart )
                chart->updateSeries(itemcfg.nAddr, rawValue );
        }
    }
}
