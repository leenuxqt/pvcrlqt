#ifndef VOLTAGECHARTFORM_H
#define VOLTAGECHARTFORM_H


#include "basedataunitwidget.h"

#include "slaveconfdialog.h"

#include <QWidget>
#include <QtCharts/QChartGlobal>

namespace Ui {
class VoltageChartForm;
}

QT_CHARTS_BEGIN_NAMESPACE
class QChartView;
QT_CHARTS_END_NAMESPACE

class Chart;
class QLineEdit;

class VoltageChartForm : public BaseDataUnitWidget // QWidget
{
    Q_OBJECT

public:
    explicit VoltageChartForm(QWidget *parent = nullptr);
    ~VoltageChartForm();

private:
    Ui::VoltageChartForm *ui;


public:
    void setConfig(const SlaveConfig &cfg);

    void parseDataUnit(const QModbusDataUnit &dataUnit);

    const SlaveConfig &getConfig()
    {
        return _config;
    }

private:
    Chart *chart;
    QtCharts::QChartView *chartView;

    SlaveConfig _config;
    QHash<int, QLineEdit*> m_hashAddrToValueEditor;

    float (*changeFunc)(const uint16_t *src);

private:
    void _initChartPage();
    void _updateChart();

    void updateEditorRealValue( const int addr, const float itemValue );

    void updateEditorRealValue( const int addr, const int itemValue );
};

#endif // VOLTAGECHARTFORM_H
