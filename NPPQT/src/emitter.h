#ifndef EMITTER_H
#define EMITTER_H

#include <QObject>
#include <QPointF>
#include <QList>
#include <QGraphicsItem>
#include <QPropertyAnimation>

class NPPAnimation: public QObject
{
    Q_OBJECT
public:
    NPPAnimation();

    QPropertyAnimation *anim;
    NPPAnimation *next;

    virtual ~NPPAnimation();

    virtual void start();
};

class BoltAnimation: public NPPAnimation, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)
public:
    int current_angle;

    BoltAnimation(QPointF from, QPointF to);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF boundingRect() const;

    virtual ~BoltAnimation();

    void start();
};

class BallParticle
{
public:
    qreal angle;
    qreal currentLength;
    int type;
};

class BallAnimation: public NPPAnimation, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
    Q_PROPERTY(qreal length READ getLength WRITE setLength)
public:
    QList<BallParticle *> particles;
    QPointF position;
    qreal length;
    qreal previousLength;
    qreal size;

    BallAnimation(QPointF where, int newRadius);
    qreal getLength();
    void setLength(qreal newLength);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF boundingRect() const;

    virtual ~BallAnimation();
};

class ArcAnimation: public NPPAnimation, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
    Q_PROPERTY(qreal length READ getLength WRITE setLength)
public:
    QList<BallParticle *> particles;
    QPointF position;
    qreal length;
    qreal previousLength;
    int degrees;
    QRectF brect;
    qreal centerAngle;
    qreal maxLength;

    ArcAnimation(QPointF from, QPointF to, int newDegrees);
    qreal getLength();
    void setLength(qreal newLength);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF boundingRect() const;

    virtual ~ArcAnimation();
};

#endif // EMITTER_H
