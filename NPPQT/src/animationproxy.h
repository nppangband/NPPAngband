#ifndef ANIMATIONPROXY_H
#define ANIMATIONPROXY_H

#include <QObject>
#include <QGraphicsItem>

class AnimationProxy : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
    QGraphicsItem *client;
    explicit AnimationProxy(QGraphicsItem *_client, QObject *parent = 0);
signals:

public slots:
    QPointF pos();
    void setPos(QPointF _pos);
};

#endif // ANIMATIONPROXY_H
