#include "birthdialog.h"
#include "ui_birthdialog.h"
#include "optionsdialog.h"
#include "npp.h"

BirthDialog::BirthDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BirthDialog)
{
    ui->setupUi(this);

    byte mode = GAME_NPPANGBAND;

    ui->sex_combo->addItem("");
    for (int i = 0; i < MAX_SEXES; i++) {
        ui->sex_combo->addItem(sex_info[i].title);
    }
}

BirthDialog::~BirthDialog()
{
    delete ui;
}

void BirthDialog::on_cancel_button_clicked()
{
    this->close();
}

void BirthDialog::on_options_button_clicked()
{
    OptionsDialog *dlg = new OptionsDialog(this);
    dlg->exec();
    delete dlg;
}
