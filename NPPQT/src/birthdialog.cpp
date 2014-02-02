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

    for (int i = 0; i < MAX_SEXES; i++) {
        ui->sex_combo->addItem(sex_info[i].title);
    }
    ui->sex_combo->setCurrentIndex(-1);

    for (int i = 0; i < z_info->p_max; i++) {
        ui->race_combo->addItem(p_info[i].pr_name);
    }
    ui->race_combo->setCurrentIndex(-1);

    for (int i; i < z_info->c_max; i++) {
        ui->class_combo->addItem(c_info[i].cl_name);
    }
    ui->class_combo->setCurrentIndex(-1);
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
