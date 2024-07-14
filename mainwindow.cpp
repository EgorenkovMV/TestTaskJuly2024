#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QLayout>

Painter::Painter(QWidget *parent)
    : QWidget(parent)
{}

Painter::~Painter()
{}

void Painter::setData(std::vector<TransportInfo> currData)
{
    this->currData = currData;
}

void Painter::paintEvent(QPaintEvent *) {
    if (blinkingActive) {
        if (blinkCounter <= 0) {
            blinkCounter = blinkCounterMax;
            if (currColor == blinkUpColor) {
                currColor = blinkDownColor;
            }
            else {
                currColor = blinkUpColor;
            }
        }
        blinkCounter--;
    }

    QPainter p(this);
    p.setPen(Qt::red);
    p.setBrush(currColor);
    for (const TransportInfo &ti : currData) {
        p.drawRect(ti);

        p.setPen(Qt::darkBlue);
        p.setBrush(Qt::lightGray);
    }
}



MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , painter(new Painter(this))
{
    ui->setupUi(this);

    painter->setMinimumSize(300, 900);
    painter->update();

    connect(ui->pbReconnect, &QPushButton::clicked, this, &MainWindow::reconnect);
    connect(ui->cbSafeMode, &QCheckBox::stateChanged, this, &MainWindow::safeModeChecked);
    connect(ui->cbDangerZonesView, &QCheckBox::stateChanged, this, &MainWindow::dangerZonesViewChecked);


}

MainWindow::~MainWindow()
{
    delete ui;
    delete painter;
}

void MainWindow::appendLog(QString log)
{
    ui->pteMsgLog->setPlainText(ui->pteMsgLog->toPlainText() + log + '\n');
}

void MainWindow::drawCars(const std::vector<TransportInfo> &currData)
{
    std::vector<TransportInfo> drawableData;
    for (const TransportInfo &ti : currData) {
        TransportInfo drawable = ti.scaleUpDown(10);
        drawable.translate(150, 450);
        drawableData.push_back(drawable);
    }

    painter->setData(drawableData);
    painter->update();

}

void MainWindow::activateBlinking(bool val)
{
    if (val == painter->blinkingActive) {
        return;
    }
    painter->blinkingActive = val;
    painter->currColor = painter->blinkDownColor;
    painter->blinkCounter = 0;
}


