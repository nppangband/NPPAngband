#ifndef OBJECT_SELECT_H
#define OBJECT_SELECT_H

#include <QtWidgets>
#include "src/npp.h"

class QTabWidget;


class floor_tab : public QWidget
{
    Q_OBJECT

public:
    floor_tab();
    bool count_floor_items(int mode, int sq_y, int sq_x);

private:
    QVector<int> floor_items;
};

class inven_tab : public QWidget
{
    Q_OBJECT

public:
    inven_tab();
    bool count_inven_items(int mode);

private:
    QVector<int> inven_items;
};

class equip_tab : public QWidget
{
    Q_OBJECT

public:
    equip_tab();
    bool count_equip_items(int mode);

private:
    QVector<int> equip_items;
};

class quiver_tab : public QWidget
{
    Q_OBJECT

public:
    quiver_tab();
    bool count_quiver_items(int mode);

private:
    QVector<int> quiver_items;
};


class ObjectSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ObjectSelectDialog(QString prompt, bool allow_floor, bool allow_inven, bool allow_equip, bool allow_quiver, QWidget *parent = 0);

private:
    QTabWidget *equip_tabs;
    QDialogButtonBox *buttons;
};


#endif // OBJECT_SELECT_H
