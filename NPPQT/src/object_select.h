#ifndef OBJECT_SELECT_H
#define OBJECT_SELECT_H

#include <QtWidgets>
#include "src/npp.h"


class QTabWidget;


class ObjectSelectDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ObjectSelectDialog(int *item, QString prompt, int mode, bool *oops, int sq_y, int sq_x);

private:
    QTabWidget *equip_tabs; 
    QDialogButtonBox *buttons;
    QWidget *floor_tab;
    QWidget *inven_tab;
    QWidget *equip_tab;
    QWidget *quiver_tab;

    // Functions to build the actual tabs
    void build_floor_tab();
    void build_inven_tab();
    void build_equip_tab();
    void build_quiver_tab();

    //Functions to track the list of possible items
    void floor_items_count(int mode, int sq_y, int sq_x);
    void inven_items_count(int mode);
    void equip_items_count(int mode);
    void quiver_items_count(int mode);

    //Vectors to store the index numbers of the actual objects
    QVector<int> floor_items;
    QVector<int> inven_items;
    QVector<int> equip_items;
    QVector<int> quiver_items;

    bool allow_floor;
    bool allow_inven;
    bool allow_equip;
    bool allow_quiver;
};


#endif // OBJECT_SELECT_H
