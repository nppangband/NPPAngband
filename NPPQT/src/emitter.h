#ifndef EMITTER_H
#define EMITTER_H

#include <QObject>
#include <QPointF>
#include <QList>
#include <QGraphicsItem>
#include <QTimer>

class Particle
{
public:
    Particle();

    bool active;
    QPointF position;
    QPointF velocity;
};

class Emitter : public QObject
{
    Q_OBJECT
public:
    int step;
    int max_steps;
    int pause;
    QPointF position;
    QPointF velocity;
    QList<Particle *> particles;
    Emitter *next;
    QTimer timer;

    explicit Emitter(QObject *parent = 0);
    void start();
    virtual void do_step() {}

    virtual ~Emitter();
signals:
    void finished();

public slots:
    void timeout();
};

class BallEmitter: public Emitter, public QGraphicsItem
{
    Q_OBJECT
    Q_INTERFACES(QGraphicsItem)
public:
    qreal radius;

    BallEmitter(int y, int x, int _radius);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
    QRectF boundingRect() const;

    void do_step();
};

#endif // EMITTER_H
