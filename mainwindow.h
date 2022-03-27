#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "slaveinfoframe.h"

#include <QMainWindow>

#include <QModbusDataUnit>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class CModbusController;
class QBoxLayout;
class SlaveInfoFrame;


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(int portNo, QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private:

    QHash<QString, CModbusController*> m_hashController;
    QHash<QString, SlaveInfoFrame*> m_hasNameToInfoFrame;
    QHash<QString, QList<SlaveNodeConfig> > m_hashChannelToSlaveList;

private:
    void initPage();
    bool loadJsonConfig(const int portNo );

    bool _parseTfObject(const QJsonObject &tf);
    bool _parseBoxObject(const QJsonObject &box, QBoxLayout *parentLayout);
    bool _parseInverterObject(const QJsonObject &inverter, QBoxLayout *parentLayout);

public slots:
    void _parseModbusDataUnit(CModbusController *controller, const int serverAddress, const QModbusDataUnit &dataUnit);
};



#endif // MAINWINDOW_H
