#ifndef VOLTAGECHARTFORM_H
#define VOLTAGECHARTFORM_H

#include <QWidget>
#include <QtCharts/QChartGlobal>

namespace Ui {
class VoltageChartForm;
}

QT_CHARTS_BEGIN_NAMESPACE
class QChartView;
QT_CHARTS_END_NAMESPACE

class Chart;

class VoltageChartForm : public QWidget
{
    Q_OBJECT

public:
    explicit VoltageChartForm(QWidget *parent = nullptr);
    ~VoltageChartForm();

private:
    Ui::VoltageChartForm *ui;

private:
    Chart *chart;
    QtCharts::QChartView *chartView;

private:
    void _initChartPage();
};

#endif // VOLTAGECHARTFORM_H
