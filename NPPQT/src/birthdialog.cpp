#include "birthdialog.h"
#include "ui_birthdialog.h"
#include "optionsdialog.h"
#include "npp.h"

static QString format_stat(s16b value)
{
    QString text;
    if (value >= 0) text.append('+');
    return QString("%1%2").arg(text).arg(value);
}

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

    for (int i = 0; i < z_info->c_max; i++) {
        ui->class_combo->addItem(c_info[i].cl_name);
    }
    ui->class_combo->setCurrentIndex(-1);
    for (int i = 0; i < ui->stats_table->rowCount(); i++) {
        for (int j = 0; j < ui->stats_table->columnCount(); j++) {
            ui->stats_table->setItem(i, j, new QTableWidgetItem());
        }
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

void BirthDialog::on_race_combo_currentIndexChanged(int index)
{
    if (!this->isVisible()) return;
    if (index < 0) return;
    player_race *r_ptr = p_info + index;
    for (int i = 0; i < A_MAX; i++) {
        ui->stats_table->item(i, 0)->setText(format_stat(r_ptr->r_adj[i]));
    }
    ui->stats_table->item(6, 0)->setText(QString::number(r_ptr->r_mhp));
    ui->stats_table->item(7, 0)->setText(QString::number(r_ptr->r_exp).append('%'));
    ui->stats_table->item(8, 0)->setText(QString::number(r_ptr->infra));
    this->update_stats();
}

void BirthDialog::on_class_combo_currentIndexChanged(int index)
{
    if (!this->isVisible()) return;
    if (index < 0) return;
    player_class *c_ptr = c_info + index;
    for (int i = 0; i < A_MAX; i++) {
        ui->stats_table->item(i, 1)->setText(format_stat(c_ptr->c_adj[i]));
    }
    ui->stats_table->item(6, 1)->setText(QString::number(c_ptr->c_mhp));
    ui->stats_table->item(7, 1)->setText(QString::number(c_ptr->c_exp).append('%'));
    this->update_stats();
}

void BirthDialog::update_stats()
{
    if (!this->isVisible()) return;
    int p_idx = ui->race_combo->currentIndex();
    int c_idx = ui->class_combo->currentIndex();
    if ((p_idx < 0) || (c_idx < 0)) return;
    for (int i = 0; i < A_MAX; i++) {
        int value = p_info[p_idx].r_adj[i] + c_info[c_idx].c_adj[i];
        ui->stats_table->item(i, 2)->setText(format_stat(value));
    }
    int hit_dice = p_info[p_idx].r_mhp + c_info[c_idx].c_mhp;
    ui->stats_table->item(6, 2)->setText(QString::number(hit_dice));
    int exp = p_info[p_idx].r_exp + c_info[c_idx].c_exp;
    ui->stats_table->item(7, 2)->setText(QString::number(exp).append('%'));
}

void BirthDialog::on_next_button_clicked()
{
    if (ui->stackedWidget->currentIndex() == 0) {
        if ((ui->name_edit->text().trimmed().length() == 0) ||
                (ui->sex_combo->currentIndex() == -1) ||
                (ui->race_combo->currentIndex() == -1) ||
                (ui->class_combo->currentIndex() == -1)) {
            pop_up_message_box(tr("Please fill all the fields"), QMessageBox::Critical);
            return;
        }
        ui->stackedWidget->setCurrentIndex(1);
        ui->prev_button->setEnabled(true);
    }
}

void BirthDialog::on_prev_button_clicked()
{
    int idx = ui->stackedWidget->currentIndex();
    if (idx > 0) {
        --idx;
        ui->stackedWidget->setCurrentIndex(idx);
    }
    if (idx < 1) {
        ui->prev_button->setEnabled(false);
    }
}
