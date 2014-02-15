#ifndef DUNGEONBOX_H
#define DUNGEONBOX_H

#include <QPlainTextEdit>

class DungeonBox : public QPlainTextEdit
{
    Q_OBJECT
public:
    explicit DungeonBox(QWidget *parent = 0);

    void keyPressEvent(QKeyEvent *e);

    void redraw();
signals:

public slots:

private:
};

#endif // DUNGEONBOX_H
