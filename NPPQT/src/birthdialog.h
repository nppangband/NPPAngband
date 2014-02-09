#ifndef BIRTHDIALOG_H
#define BIRTHDIALOG_H

#include <QDialog>
#include "defines.h"
#include "player.h"

namespace Ui {
class BirthDialog;
}

class QButtonGroup;
class QPushButton;

class BirthDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BirthDialog(QWidget *parent = 0);
    ~BirthDialog();

    bool done_birth;

    bool run();

    void set_quick_start(bool enable);

private slots:
    void on_cancel_button_clicked();

    void on_options_button_clicked();

    void on_next_button_clicked();

    void on_prev_button_clicked();

    void on_bg1_clicked(int index);

    void on_bg2_clicked(int index);

    void on_sell_clicked();

    void on_buy_clicked();

    void on_point_radio_clicked();

    void on_roller_radio_clicked();

    void on_roll_button_clicked();

    void on_gen_name_button_clicked();

    void on_ran_sex_button_clicked();

    void on_random_race_clicked();

    void on_random_class_clicked();

    void on_sex_combo_currentIndexChanged(int index);

    void on_ran_char_button_clicked();

private:
    Ui::BirthDialog *ui;

    void update_stats();

    void update_points();

    void update_others();

    int cur_race;
    int cur_class;

    int stats[A_MAX];
    int points_spent[A_MAX];
    int points_left;

    bool dirty;

    bool point_based;

    QButtonGroup *bg1;
    QButtonGroup *bg2;
    QPushButton *ran_race_button;
    QPushButton *ran_class_button;

    bool quick_start;
};

#endif // BIRTHDIALOG_H
