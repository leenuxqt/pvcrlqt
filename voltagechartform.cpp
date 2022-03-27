#include "voltagechartform.h"
#include "ui_voltagechartform.h"

#include "chart.h"

#include <QtCharts/QChartView>

QT_CHARTS_USE_NAMESPACE

VoltageChartForm::VoltageChartForm(QWidget *parent) :
    QWidget(parent), ui(new Ui::VoltageChartForm),
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
    chart->setTitle("Dynamic spline chart");
    chart->legend()->hide();
    chart->setAnimationOptions(QChart::AllAnimations);

    chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);
    chartView->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    ui->verticalLayoutVol->addWidget( chartView );
}
