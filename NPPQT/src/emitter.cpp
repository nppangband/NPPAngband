#include "emitter.h"
#include "npp.h"
#include <QPainter>
#include <QtCore/qmath.h>
#include <QGraphicsScene>

static QPixmap *ball_pix = 0;

static void load_ball_pix()
{
    if (!ball_pix) {
        QString path(NPP_DIR_GRAF);
        path.append("ball1.png");
        ball_pix = new QPixmap(path);
    }
}

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

static QPointF getCenter(int y, int x)
{
    QSize dim = ui_grid_size();
    return QPointF(x * dim.width() + dim.width() / 2,
                   y * dim.height() + dim.height() / 2);
}

NPPAnimation::NPPAnimation()
{
    anim = 0;
    next = 0;
}

NPPAnimation::~NPPAnimation()
{
    if (next) next->start();
    if (anim) delete anim;
}

void NPPAnimation::start()
{
    if (anim) anim->start();
}

static int BOLT_SIZE = 40;

BoltAnimation::BoltAnimation(QPointF from, QPointF to)
{
    setZValue(300);
    anim = new QPropertyAnimation(this, "pos");
    anim->setDuration(500);
    anim->setStartValue(getCenter(from.y(), from.x()) - QPointF(BOLT_SIZE / 2, BOLT_SIZE / 2));
    anim->setEndValue(getCenter(to.y(), to.x()) - QPointF(BOLT_SIZE / 2, BOLT_SIZE / 2));
    connect(anim, SIGNAL(finished()), this, SLOT(deleteLater()));
}

void BoltAnimation::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->save();

    load_ball_pix();

    painter->drawPixmap(boundingRect(), *ball_pix, boundingRect());

    painter->restore();
}

QRectF BoltAnimation::boundingRect() const
{
    return QRectF(0, 0, BOLT_SIZE, BOLT_SIZE);
}

BoltAnimation::~BoltAnimation()
{
    if (scene()) scene()->removeItem(this);
}

static int BALL_TILE_SIZE = 40;

BallAnimation::BallAnimation(QPointF where, int newRadius)
{
    setZValue(300);

    load_ball_pix();

    QSize size_temp = ui_grid_size();
    size = size_temp.height();
    size *= (newRadius * 2 + 1);

    length = previousLength = 0;

    int size2 = size + BALL_TILE_SIZE;

    where = getCenter(where.y(), where.x());

    setPos(where.x() - size2 / 2, where.y() - size2 / 2);

    position = QPointF(size2 / 2, size2 / 2);

    anim = new QPropertyAnimation(this, "length");
    anim->setDuration(1000);
    anim->setStartValue(0);
    anim->setEndValue(size / 2.0);
    connect(anim, SIGNAL(finished()), this, SLOT(deleteLater()));
}

qreal BallAnimation::getLength()
{
    return length;
}

void BallAnimation::setLength(qreal newLength)
{
    length = newLength;

    if (length < previousLength + 4) return;

    qreal delta = length - previousLength;

    previousLength = length;

    for (int i = 0; (i < 25); i++) {
        qreal angle = rand_int(360) * 2 * PI / 360;
        BallParticle *p = new BallParticle;
        p->type = rand_int(3);
        p->angle = angle;
        p->currentLength = 0;
        particles.append(p);
    }

    for (int i = 0; i < particles.size(); i++) {
        BallParticle *p = particles.at(i);
        p->currentLength += delta;
    }

    update();
}

BallAnimation::~BallAnimation()
{
    for (int i = 0; i < particles.size(); i++) {
        delete particles.at(i);
    }
    particles.clear();

    if (scene()) scene()->removeItem(this);
}

void BallAnimation::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->save();

    //painter->setClipRect(boundingRect());

    for (int i = 0; i < particles.size(); i++) {
        BallParticle *p = particles.at(i);
        QColor col("white");
        QPointF where = position + fromAngle(p->angle, p->currentLength);
        if (p->type == 0) {
            qreal opacity = 1;
            if (p->currentLength > size / 4.0) opacity = 0.5;
            painter->setOpacity(opacity);
            painter->drawPixmap(where.x() - BALL_TILE_SIZE / 2,
                                where.y() - BALL_TILE_SIZE / 2,
                                BALL_TILE_SIZE, BALL_TILE_SIZE, *ball_pix);
        }
        else {
            painter->setOpacity(1);
            painter->fillRect(QRectF(where.x(), where.y(), 1, 1), col);
        }
    }

    painter->restore();
}

QRectF BallAnimation::boundingRect() const
{
    return QRectF(0, 0, size + BALL_TILE_SIZE, size + BALL_TILE_SIZE);
}

