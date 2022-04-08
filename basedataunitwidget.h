#ifndef BASEDATAUNITWIDGET_H
#define BASEDATAUNITWIDGET_H

#include <QWidget>
#include <QModbusDataUnit>

class BaseDataUnitWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BaseDataUnitWidget(QWidget *parent = nullptr);

signals:

public:
    virtual void parseDataUnit(const QModbusDataUnit &dataUnit)=0;

};

#endif // BASEDATAUNITWIDGET_H
