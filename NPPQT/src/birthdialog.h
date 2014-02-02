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

    void on_race_combo_currentIndexChanged(int index);

    void on_class_combo_currentIndexChanged(int index);

    void on_next_button_clicked();

    void on_prev_button_clicked();

private:
    Ui::BirthDialog *ui;

    void update_stats();
};

#endif // BIRTHDIALOG_H
