/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef CHART_H
#define CHART_H

#include "mymodbusfunc.h"

#include "slaveconfdialog.h"

#include <QtCharts/QChart>
#include <QtCore/QTimer>
#include <QtCore/QDateTime>

QT_CHARTS_BEGIN_NAMESPACE
class QSplineSeries;
class QValueAxis;
class QDateTimeAxis;
QT_CHARTS_END_NAMESPACE

QT_CHARTS_USE_NAMESPACE

//![1]
class Chart: public QChart
{
    Q_OBJECT
public:
    Chart(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    virtual ~Chart();

public slots:
    void handleTimeout();

    void startTimer();


public:
    void addSlaveSeries(const SlaveItemConfig &itemcfg);

    void updateSeries(const int slaveAddr, const float fVal);
    void updateSeries(const int slaveAddr, const uint16 rawValue);

private:
    QHash<int, QSplineSeries *> m_hashAddrToSeries;
    QHash<int, double> m_hashAddrToRealValue;
    QHash<int, double> m_hashAddrToLastValue;
private:
    QTimer m_timer;
//    QSplineSeries *m_seriesA;
//    QSplineSeries *m_seriesB;
//    QSplineSeries *m_seriesC;

//    qreal m_ya;
//    qreal m_yb;
//    qreal m_yc;

//    qreal m_last_ya;
//    qreal m_last_yb;
//    qreal m_last_yc;

//    QValueAxis *m_axisX;
//    qreal m_x;

    QStringList m_titles;

    QDateTimeAxis *m_axisX;
    QValueAxis *m_axisY;

    qreal m_step;

    QDateTime m_x;
    QDateTime m_last_x;


    int futureSecond;
};
//![1]

#endif /* CHART_H */
