#ifndef SLAVEINFOWIDGET_H
#define SLAVEINFOWIDGET_H

#include "basedataunitwidget.h"

#include <QWidget>
#include <QHash>
#include <QModbusDataUnit>



class QLineEdit;


#include "slaveconfdialog.h"

namespace Ui {
class SlaveInfoWidget;
}

class SlaveInfoWidget : public BaseDataUnitWidget
{
    Q_OBJECT

public:
    explicit SlaveInfoWidget(QWidget *parent = nullptr);
    ~SlaveInfoWidget();

private:
    Ui::SlaveInfoWidget *ui;

public:
    void setConfig(const SlaveConfig &cfg);

    void parseDataUnit(const QModbusDataUnit &dataUnit);

    const SlaveConfig &getConfig()
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

#endif // SLAVEINFOWIDGET_H
