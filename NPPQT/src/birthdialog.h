#ifndef BIRTHDIALOG_H
#define BIRTHDIALOG_H

#include <QDialog>

namespace Ui {
class BirthDialog;
}

class BirthDialog : public QDialog
{
    Q_OBJECT

public:
    explicit BirthDialog(QWidget *parent = 0);
    ~BirthDialog();

private slots:
    void on_cancel_button_clicked();

    void on_options_button_clicked();

    void on_next_button_clicked();

    void on_prev_button_clicked();

    void on_bg1_clicked(int index);

    void on_bg2_clicked(int index);

private:
    Ui::BirthDialog *ui;

    void update_stats();

    void update_points();

    int cur_race;
    int cur_class;
};

#endif // BIRTHDIALOG_H
