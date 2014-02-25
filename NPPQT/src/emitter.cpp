#include "emitter.h"
#include "npp.h"
#include <QPainter>
#include <QtCore/qmath.h>

static qreal magnitude(QPointF vec)
{
    return qSqrt(vec.x() * vec.x() + vec.y() * vec.y());
}

Particle::Particle()
{
    active = true;
}

Emitter::Emitter(QObject *parent) :
    QObject(parent)
{
    step = 0;
    max_steps = 100;
    pause = 50;
    next = 0;

    connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

void Emitter::start()
{
    timer.start(pause);
}

BallEmitter::BallEmitter(int y, int x, int newRadius)
{
    QSize size_temp = ui_grid_size();
    int size = size_temp.width();
    size *= (newRadius * 2 + 1);
    radius = size / 2.0;
    setPos((x - newRadius) * size_temp.width(), (y - newRadius) * size_temp.height());
    setZValue(300);
    position = QPointF(radius, radius);
    velocity = QPointF(0, 4);
}

void Emitter::timeout()
{
    do_step();
}

Emitter::~Emitter()
{
    for (int i = 0; i < particles.size(); i++) {
        delete particles.at(i);
    }
    particles.clear();
}

void BallEmitter::do_step()
{
    if (step == 0) {
        Particle *p = new Particle;
        p->position = position;
        particles.append(p);
    }

    for (int i = 0; i < particles.size(); i++) {
        Particle *p = particles.at(i);
        if (!p->active) continue;
        p->position += velocity;
        if (magnitude(p->position - position) > radius) {
            p->active = false;
            emit finished();
            return;
        }
    }

    update();

    if (++step < max_steps) {
        start();
        if (step > 5) velocity = QPointF(0, 2);
    }
    else {
        emit finished();
    }
}

void BallEmitter::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->save();

    //painter->setClipRect(boundingRect());

    for (int i = 0; i < particles.size(); i++) {
        Particle *p = particles.at(i);
        if (!p->active) continue;
        painter->fillRect(QRectF(p->position.x(), p->position.y(), 1, 1), QColor("cyan"));
    }

    painter->restore();
}

QRectF BallEmitter::boundingRect() const
{
    qreal size = radius * 2;
    return QRectF(0, 0, size, size);
}
