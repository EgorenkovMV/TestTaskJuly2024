#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPainter>
#include <vector>
#include "transportinfo.h"



class Painter : public QWidget {
    void paintEvent(QPaintEvent *);
    std::vector<TransportInfo> currData;

public:
    Painter(QWidget *parent = nullptr);
    ~Painter();
    void setData(std::vector<TransportInfo> currData);

    bool blinkingActive = false;
    QColor blinkUpColor = Qt::gray;
    QColor blinkDownColor = Qt::lightGray;
    QColor currColor = Qt::lightGray;
    int blinkCounter = 0;
    int blinkCounterMax = 10;

};


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void appendLog(QString data);
    void drawCars(const std::vector<TransportInfo> &currData);
    void activateBlinking(bool val = true);

signals:
    void reconnect();
    void safeModeChecked(bool);
    void dangerZonesViewChecked(bool);

private:
    Ui::MainWindow *ui;
    Painter *painter;
};
#endif // MAINWINDOW_H
