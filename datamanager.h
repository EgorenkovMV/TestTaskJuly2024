#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include <QObject>
#include <QTcpSocket>
#include <QString>
#include <QRectF>
#include "mainwindow.h"
#include "transportinfo.h"


enum class State
{
    Idle,
    Maneuvering,
    // Retreating is assumed instant
    Succeeded,
    Crashed,    // not our fault since retreating is instant
};


class DataManager : public QObject
{
    Q_OBJECT
public:
    explicit DataManager(QObject *parent = nullptr);
    ~DataManager();

private slots:
    void readData();
    void setDangerZonesView(bool val);
    void setSafeMode(bool val);

public slots:
    void reconnect();

signals:

private:
    MainWindow *mw;
    QTcpSocket *soc;
    State currState;
    TransportInfo selfInfo;
    bool selfInfoNotInited = true;
    bool safeModeEnabled = false;
    bool dangerZonesViewEnabled = false;


    void processData(std::vector<TransportInfo> currData);
    void startManeuvering();
    void retreat();


};

#endif // DATAMANAGER_H
