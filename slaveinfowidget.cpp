
#include "mymodbusfunc.h"
#include "slaveinfowidget.h"
#include "ui_slaveinfowidget.h"

#include <QDebug>
#include <QLineEdit>
#include <QLabel>


SlaveInfoWidget::SlaveInfoWidget(QWidget *parent) :
    BaseDataUnitWidget(parent), //QWidget(parent),
    ui(new Ui::SlaveInfoWidget)
{
    ui->setupUi(this);
    changeFunc = nullptr;
}

SlaveInfoWidget::~SlaveInfoWidget()
{
    delete ui;
}


void SlaveInfoWidget::setConfig(const SlaveConfig &cfg)
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
    _updateUi();

}


void SlaveInfoWidget::_updateUi()
{
    ui->label_Image->setPixmap( QPixmap( QString(":/%1").arg(_config.strImg) ));

    for( int i=0; i<_config.lstItem.size();i++ )
    {
        const SlaveItemConfig &itemCfg = _config.lstItem.at(i);

        m_hashAddrToEditor[itemCfg.nAddr] = new QLineEdit(this);
        m_hashAddrToEditor[itemCfg.nAddr]->setReadOnly( true );
        ui->formLayout->addRow( tr("%1(%2)").arg(itemCfg.strLabel).arg(itemCfg.strUnit), m_hashAddrToEditor[itemCfg.nAddr] );
    }
}

void SlaveInfoWidget::updateEditorValue( const int addr, const float itemValue )
{
    QLineEdit *editor = m_hashAddrToEditor[addr];
    if( editor ) {
        editor->setText( QString::number(itemValue, 'f', 2)  );
    }
}

void SlaveInfoWidget::updateEditorValue( const int addr, const int itemValue )
{
    QLineEdit *editor = m_hashAddrToEditor[addr];
    if( editor ) {
        editor->setText( QString::number(itemValue)  );
    }
}

void SlaveInfoWidget::parseDataUnit(const QModbusDataUnit &dataUnit)
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
            updateEditorValue( itemcfg.nAddr, itemValue );
        } else {
            updateEditorValue( itemcfg.nAddr, rawValue );
        }
    }
}
