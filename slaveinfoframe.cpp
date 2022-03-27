
#include "slaveinfoframe.h"
#include "ui_slaveinfoframe.h"

#include <QDebug>
#include <QLineEdit>
#include <QLabel>

typedef unsigned short int uint16;
typedef unsigned long int uint32;



// 短整型大小端互换

#define BigLittleSwap16(A)  ((((uint16)(A) & 0xff00) >> 8) | \
                            (((uint16)(A) & 0x00ff) << 8))

 // 长整型大小端互换
#define BigLittleSwap32(A)  ((((uint32)(A) & 0xff000000) >> 24) | \
                            (((uint32)(A) & 0x00ff0000) >> 8) | \
                            (((uint32)(A) & 0x0000ff00) << 8) | \
                            (((uint32)(A) & 0x000000ff) << 24))



 // 本机大端返回1，小端返回0
int checkCPUendian()
{
       union{
              unsigned long int i;
              unsigned char s[4];
       }c;

       c.i = 0x12345678;
       return (0x12 == c.s[0]);
}



// 模拟htonl函数，本机字节序转网络字节序

unsigned long int t_htonl(unsigned long int h)

{
       // 若本机为大端，与网络字节序同，直接返回
       // 若本机为小端，转换成大端再返回
       return checkCPUendian() ? h : BigLittleSwap32(h);
}

// 模拟ntohl函数，网络字节序转本机字节序
unsigned long int t_ntohl(unsigned long int n)
{
       // 若本机为大端，与网络字节序同，直接返回
       // 若本机为小端，网络数据转换成小端再返回
       return checkCPUendian() ? n : BigLittleSwap32(n);
}


// 模拟htons函数，本机字节序转网络字节序
unsigned short int t_htons(unsigned short int h)
{
       // 若本机为大端，与网络字节序同，直接返回
       // 若本机为小端，转换成大端再返回
       return checkCPUendian() ? h : BigLittleSwap16(h);
}


// 模拟ntohs函数，网络字节序转本机字节序
unsigned short int t_ntohs(unsigned short int n)
{
       // 若本机为大端，与网络字节序同，直接返回
       // 若本机为小端，网络数据转换成小端再返回
       return checkCPUendian() ? n : BigLittleSwap16(n);
}

/* Get a float from 4 bytes (Modbus) without any conversion (ABCD) */
float my_modbus_get_float_abcd(const uint16_t *src)
{
    float f;
    uint32_t i;

    i = t_ntohl(((uint32_t)src[0] << 16) + src[1]);
    memcpy(&f, &i, sizeof(float));

    return f;
}

/* Get a float from 4 bytes (Modbus) with swapped words (CDAB) */
float my_modbus_get_float_cdab(const uint16_t *src)
{
    float f;
    uint32_t i;

    i = t_ntohl((((uint32_t)src[1]) << 16) + src[0]);
    memcpy(&f, &i, sizeof(float));

    return f;
}

/* Get a float from 4 bytes (Modbus) in inversed format (DCBA) */
float my_modbus_get_float_dcba(const uint16_t *src)
{
    float f;
    uint32_t i;

//    i = t_ntohl(bswap_32((((uint32_t)src[0]) << 16) + src[1]));
    i = t_ntohl(BigLittleSwap32((((uint32_t)src[0]) << 16) + src[1]));
    memcpy(&f, &i, sizeof(float));

    return f;
}

/* Get a float from 4 bytes (Modbus) with swapped bytes (BADC) */
float my_modbus_get_float_badc(const uint16_t *src)
{
    float f;
    uint32_t i;

//    i = t_ntohl((uint32_t)(bswap_16(src[0]) << 16) + bswap_16(src[1]));
    i = t_ntohl((uint32_t)(BigLittleSwap16(src[0]) << 16) + BigLittleSwap16(src[1]));
    memcpy(&f, &i, sizeof(float));

    return f;
}


SlaveInfoFrame::SlaveInfoFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::SlaveInfoFrame)
{
    ui->setupUi(this);
    changeFunc = nullptr;
}

SlaveInfoFrame::~SlaveInfoFrame()
{
    delete ui;
}

void SlaveInfoFrame::setConfig(const SlaveConfig &cfg)
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


void SlaveInfoFrame::_updateUi()
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

void SlaveInfoFrame::updateEditorValue( const int addr, const float itemValue )
{
    QLineEdit *editor = m_hashAddrToEditor[addr];
    if( editor ) {
        editor->setText( QString::number(itemValue, 'f', 2)  );
    }
}

void SlaveInfoFrame::updateEditorValue( const int addr, const int itemValue )
{
    QLineEdit *editor = m_hashAddrToEditor[addr];
    if( editor ) {
        editor->setText( QString::number(itemValue)  );
    }
}

void SlaveInfoFrame::parseDataUnit(const QModbusDataUnit &dataUnit)
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
