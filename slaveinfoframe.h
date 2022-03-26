#ifndef SLAVEINFOFRAME_H
#define SLAVEINFOFRAME_H

#include <QFrame>
#include <QModbusDataUnit>

#include "slaveconfdialog.h"

namespace Ui {
class SlaveInfoFrame;
}

template <typename T=float>
class SlaveData
{
public:
    T valueP;
    T valueQ;
    T valueU;
    T valueI;
};


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
    void setData(const SlaveData<float> &_d);
    void parseDataUnit(const QModbusDataUnit &dataUnit);

    SlaveConfig &getConfig()
    {
        return _config;
    }

private:
    void _updateUi();

private:
    SlaveData<float> _data;
    SlaveConfig _config;

    float (*changeFunc)(const uint16_t *src);
};

#endif // SLAVEINFOFRAME_H
