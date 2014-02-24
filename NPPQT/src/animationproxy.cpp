#include "animationproxy.h"

AnimationProxy::AnimationProxy(QGraphicsItem *_client, QObject *parent) :
    QObject(parent)
{
    client = _client;
}

QPointF AnimationProxy::pos()
{
    return client->pos();
}

void AnimationProxy::setPos(QPointF _pos)
{
    client->setPos(_pos);
}
