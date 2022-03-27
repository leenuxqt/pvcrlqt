#ifndef SLAVEINFOFRAME_H
#define SLAVEINFOFRAME_H

#include <QHash>
#include <QFrame>
#include <QModbusDataUnit>

class QLineEdit;


#include "slaveconfdialog.h"

namespace Ui {
class SlaveInfoFrame;
}


class SlaveInfoFrame : public QFrame
{
    Q_OBJECT

public:
    explicit SlaveInfoFrame(QWidget *parent = nullptr);
    ~SlaveInfoFrame();

private:
    Ui::SlaveInfoFrame *ui;

public:
    void setConfig(const SlaveConfig &cfg);

    void parseDataUnit(const QModbusDataUnit &dataUnit);

    SlaveConfig &getConfig()
    {
        return _config;
    }

private:
    void _updateUi();
    void updateEditorValue( const int addr, const float itemValue );
    void updateEditorValue( const int addr, const int itemValue );

private:

    SlaveConfig _config;
    QHash<int, QLineEdit*> m_hashAddrToEditor;

    float (*changeFunc)(const uint16_t *src);
};

#endif // SLAVEINFOFRAME_H
