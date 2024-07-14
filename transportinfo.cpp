#include "transportinfo.h"
#include <QDebug>
#include <QString>


QString TransportInfo::repr() const
{
    QString repr {"TransportInfo"};

    repr.append("\n  id:        " + QString::number(id));
    repr.append("\n  speed:     " + QString::number(speed));
    repr.append("\n  xCoordRel: " + QString::number(xCoordRel));
    repr.append("\n  yCoordRel: " + QString::number(yCoordRel));
    repr.append("\n  w:         " + QString::number(w));
    repr.append("\n  l:         " + QString::number(l));

    return repr;
}

void TransportInfo::toQRectF()
{
    setTopLeft(QPointF {xCoordRel - (w / 2.), yCoordRel - (l / 2.)});
    setHeight(l);
    setWidth(w);
}

TransportInfo TransportInfo::scaleUpDown(float factor) const
{
    TransportInfo copy {*this};

    if (factor < 0) {
        return copy;
    }

    copy.moveCenter({copy.center().x() * factor, copy.center().y() * factor});

    qreal initHeight = copy.height();
    qreal initWidth = copy.width();

    copy.setTop(copy.top() - (factor - 1) / 2 * initHeight);
    copy.setBottom(copy.bottom() + (factor - 1) / 2 * initHeight);
    copy.setLeft(copy.left() - (factor - 1) / 2 * initWidth);
    copy.setRight(copy.right() + (factor - 1) / 2 * initWidth);

    return copy;
}

TransportInfo TransportInfo::getDangerZone(float timeSpan) const
{
    TransportInfo copy {*this};

    copy.setTop(copy.top() - (speed * timeSpan));

    return copy;
}

void TransportInfo::drMove(float dx, float dy)
{
    xCoordRel += dx;
    yCoordRel += dy;
    translate(dx, dy);
}





QDataStream &operator>>(QDataStream &ds, std::vector<TransportInfo> &ti)
{
    ds.setFloatingPointPrecision(QDataStream::SinglePrecision);
    ds.setByteOrder(QDataStream::BigEndian);

    uint16_t syncBytes;
    uchar dataPacketsCount;
    ds >> syncBytes >> dataPacketsCount;

    if (syncBytes != 0x045c) {
        qWarning() << "sync bytes wrong:" << QString::number(syncBytes, 16);
    }

    char *speed = new char [4];
    char *xCoordRel = new char [4];
    char *yCoordRel = new char [4];
    char *w = new char [4];
    char *l = new char [4];

    for (int i = 0; i < dataPacketsCount; i++) {
        TransportInfo item;

        ds >> item.id;
        ds.readRawData(speed, 4);
        ds.readRawData(xCoordRel, 4);
        ds.readRawData(yCoordRel, 4);
        ds.readRawData(w, 4);
        ds.readRawData(l, 4);

        std::memcpy(&item.speed, speed, sizeof item.speed);
        std::memcpy(&item.xCoordRel, xCoordRel, sizeof item.xCoordRel);
        std::memcpy(&item.yCoordRel, yCoordRel, sizeof item.yCoordRel);
        std::memcpy(&item.w, w, sizeof item.w);
        std::memcpy(&item.l, l, sizeof item.l);

        item.toQRectF();

        ti.push_back(item);
    }

    delete [] speed;
    delete [] xCoordRel;
    delete [] yCoordRel;
    delete [] w;
    delete [] l;

    return ds;
}

