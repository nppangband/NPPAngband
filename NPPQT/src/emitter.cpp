#include "emitter.h"
#include "npp.h"
#include <QPainter>
#include <QtCore/qmath.h>

static QPixmap *ball_pix = 0;

static double PI = 3.141592653589793238463;

static qreal magnitude(QPointF vec)
{
    return qSqrt(vec.x() * vec.x() + vec.y() * vec.y());
}

static qreal getAngle(QPointF vec)
{
    return qAtan2(vec.y(), vec.x());
}

static QPointF fromAngle(qreal angle, qreal magnitude)
{
    return QPointF(magnitude * qCos(angle), magnitude * qSin(angle));
}

Particle::Particle()
{
    active = true;
    type = 0;
}

Emitter::Emitter(QObject *parent) :
    QObject(parent)
{
    step = 0;
    max_steps = 100;
    pause = 25;
    next = 0;

    connect(&timer, SIGNAL(timeout()), this, SLOT(timeout()));
}

void Emitter::start()
{
    timer.start(pause);
}

BallEmitter::BallEmitter(int y, int x, int newRadius)
{
    if (ball_pix == 0) {
        QString fname = NPP_DIR_GRAF;
        fname.append("ball1.png");
        ball_pix = new QPixmap(fname);
    }

    QSize size_temp = ui_grid_size();
    size = size_temp.height();
    size *= (newRadius * 2 + 1);
    radius = newRadius;
    int w = size_temp.width();
    int h = size_temp.height();
    setPos(x * w + w / 2 - size / 2,
           y * h + h / 2 - size / 2);
    setZValue(300);
    position = QPointF(size / 2, size / 2);
    velocity = QPointF(0, 6);
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

void Emitter::finish()
{
    timer.stop();
    if (next) next->start();
    emit finished();
}

void BallEmitter::do_step()
{
    for (int i = 0; i < 25; i++) {
        qreal angle = rand_int(360) * 2 * PI / 360;
        Particle *p = new Particle;
        p->type = rand_int(3);
        p->velocity = fromAngle(angle, magnitude(velocity));
        p->position = position;
        particles.append(p);
    }

    for (int i = 0; i < particles.size(); i++) {
        Particle *p = particles.at(i);
        if (!p->active) continue;
        p->position += p->velocity;
        if (magnitude(p->position - position) > (size * 0.5)) {
            p->active = false;
            finish();
            return;
        }
    }

    update();
}

void BallEmitter::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->save();

    //painter->setClipRect(boundingRect());

    for (int i = 0; i < particles.size(); i++) {
        Particle *p = particles.at(i);
        if (!p->active) continue;
        QColor col("white");
        if (p->type == 0) {
            int s = 40;
            int x = p->position.x() - s / 2;
            int y = p->position.y() - s / 2;
            qreal opacity = 1;
            qreal m = magnitude(p->position - position);
            if (m > (size / 2 * 0.5)) opacity = 0.5;
            painter->setOpacity(opacity);
            painter->drawPixmap(x, y, s, s, *ball_pix);
        }
        else {
            painter->setOpacity(1);
            painter->fillRect(QRectF(p->position.x(), p->position.y(), 1, 1), col);
        }
    }

    painter->restore();
}

QRectF BallEmitter::boundingRect() const
{    
    return QRectF(0, 0, size, size);
}
