#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include <QDialog>
#include <QAbstractButton>

namespace Ui {
class OptionsDialog;
}

class OptionsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionsDialog(QWidget *parent = 0);    
    ~OptionsDialog();

private slots:
    void on_buttonBox_clicked(QAbstractButton *button);

private:
    Ui::OptionsDialog *ui;
};

#endif // OPTIONSDIALOG_H
