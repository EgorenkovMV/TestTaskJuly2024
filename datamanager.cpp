#include "datamanager.h"
#include <QString>
#include <QByteArray>
#include <QDebug>
#include <QPointF>
#include <QSizeF>
#include <vector>
#include <algorithm>
#include "CRC.h"


namespace Constants {
const QString hostName {"127.0.0.1"};
const quint16 port = 36215;

const int maneuverDuration = 3000;      // msec
const int clockDuration = 50;           // msec
// assuming here that received packages are well synchronized
// assuming receiving packages as a clock

const float maneuverDistance = 3.f;     // meters
// assuming here size of the road
const float dangerZoneTimeSpan = 1.f;   // sec
}


DataManager::DataManager(QObject *parent)
    : QObject{parent}
    , mw {new MainWindow}
    , soc {new QTcpSocket(this)}
{
    mw->show();


    soc->connectToHost(Constants::hostName, Constants::port);

    connect(soc, &QTcpSocket::readyRead, this, &DataManager::readData);
    connect(soc, &QTcpSocket::connected, [this](){
        mw->appendLog("Connection established");
    });
    connect(soc, &QTcpSocket::disconnected, [this](){
        mw->appendLog("Connection lost");
    });

    connect(mw, &MainWindow::reconnect, this, &DataManager::reconnect);
    connect(mw, &MainWindow::safeModeChecked, [this](bool value){
        safeModeEnabled = value;
    });
    connect(mw, &MainWindow::dangerZonesViewChecked, [this](bool value){
        dangerZonesViewEnabled = value;
    });


    currState = State::Idle;
}

DataManager::~DataManager()
{
    delete soc;
    delete mw;
}

void DataManager::readData()
{
    QByteArray data = soc->readAll();

    std::vector<TransportInfo> ti;
    uint32_t crcReceived;
    QDataStream ds {data};

    ds >> ti >> crcReceived;

    data.chop(6);

    uint32_t crcCalculated = CRC::Calculate(data.mid(2).data(), data.mid(2).size(), CRC::CRC_32());

    if (crcCalculated != crcReceived) {
        qWarning() << "crc32 doesn't match";
        mw->appendLog("crc32 doesn't match");

    }

    processData(ti);
}

void DataManager::setDangerZonesView(bool val)
{
    dangerZonesViewEnabled = val;
}

void DataManager::setSafeMode(bool val)
{
    safeModeEnabled = val;
}

void DataManager::reconnect()
{
    soc->connectToHost(Constants::hostName, Constants::port);
}

void DataManager::processData(std::vector<TransportInfo> currData)
{
    if (currState == State::Crashed or currState == State::Succeeded or not currData.size()) {
        return;
    }

    std::vector<TransportInfo>::iterator it = std::find_if(currData.begin(), currData.end(), [](TransportInfo ti){
            return ti.id == 0;
        });

    if (selfInfoNotInited) {
        selfInfoNotInited = false;
        selfInfo = *it;
    }

    selfInfo.speed = it->speed;
    // assuming here that self coordinates are always zero
    selfInfo.l = it->l;
    selfInfo.w = it->w;
    currData.erase(it);

    bool crash = false;
    for (const TransportInfo &ti : currData) {
        if ((selfInfo & ti).height()) {
            crash = true;
            break;
        }
    }

    if (crash) {
        currState = State::Crashed;
        mw->appendLog("CRASHED!");
    }


    bool roadFree = true;
    float microStep = Constants::maneuverDistance *
            static_cast<float>(Constants::clockDuration) / static_cast<float>(Constants::maneuverDuration);

    TransportInfo selfDangerZone;
    selfDangerZone = selfInfo.getDangerZone(Constants::dangerZoneTimeSpan);
    TransportInfo selfDangerZoneCopy {selfDangerZone};

    if (safeModeEnabled) {
        // assuming that our vehicle have to keep safe distance on either lines of traffic
        selfDangerZone.drMove(-microStep, 0);
    }
    else {
        // assuming that our vehicle have to keep safe distance only on left line of traffic
        selfDangerZone.drMove(-Constants::maneuverDistance, 0);
    }

    std::vector<TransportInfo> dangerZones;
    for (const TransportInfo &ti : currData) {
        dangerZones.push_back(ti.getDangerZone(Constants::dangerZoneTimeSpan));
    }

    for (const TransportInfo &tiDangerZone : dangerZones) {
        if ((selfDangerZone & tiDangerZone).height()) {
            roadFree = false;
            break;
        }
    }

    if (roadFree) {
        selfInfo.drMove(-microStep, 0);
        startManeuvering();
        if (selfInfo.xCoordRel < -Constants::maneuverDistance) {
            currState = State::Succeeded;
            mw->appendLog("SUCCESS!");
        }
    }
    else {
        retreat();
    }

    if (dangerZonesViewEnabled) {
        dangerZones.insert(dangerZones.begin(), selfDangerZoneCopy);
        mw->drawCars(dangerZones);
    }
    else {
        currData.insert(currData.begin(), selfInfo);
        mw->drawCars(currData);
    }
}

void DataManager::startManeuvering()
{
    if (currState != State::Maneuvering) {
        currState = State::Maneuvering;
        mw->activateBlinking();
    }
}

void DataManager::retreat()
{
    if (currState == State::Maneuvering) {
        selfInfo.setX(-selfInfo.w / 2);
        selfInfo.xCoordRel = 0;
        currState = State::Idle;
        mw->activateBlinking(false);
    }
}




