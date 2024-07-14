#ifndef TRANSPORTINFO_H
#define TRANSPORTINFO_H

#include <QDataStream>
#include <QRectF>
#include <vector>

struct TransportInfo : public QRectF
{
    int32_t id;
    float speed;
    float xCoordRel;
    float yCoordRel;
    float w;
    float l;



    QString repr() const;
    void toQRectF();
    TransportInfo scaleUpDown(float factor) const;
    TransportInfo getDangerZone(float timeSpan = 1.f) const;
    void drMove(float dx, float dy);

};

QDataStream &operator>>(QDataStream &ds, std::vector<TransportInfo> &ti);



#endif // TRANSPORTINFO_H
