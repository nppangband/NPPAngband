#include "birthdialog.h"
#include "ui_birthdialog.h"
#include "optionsdialog.h"
#include "npp.h"
#include <QRadioButton>
#include <QGridLayout>
#include <QButtonGroup>

static QString format_stat(s16b value)
{
    QString text;
    if (value >= 0) text.append('+');
    return QString("%1%2").arg(text).arg(value);
}

void BirthDialog::on_bg1_clicked(int index)
{
    cur_race = index;
    player_race *r_ptr = p_info + index;
    for (int i = 0; i < A_MAX; i++) {
        ui->stats_table->item(i, 0)->setText(format_stat(r_ptr->r_adj[i]));
    }
    ui->stats_table->item(6, 0)->setText(QString::number(r_ptr->r_mhp));
    ui->stats_table->item(7, 0)->setText(QString::number(r_ptr->r_exp).append('%'));
    ui->stats_table->item(8, 0)->setText(QString::number(r_ptr->infra));
    update_stats();
}

void BirthDialog::on_bg2_clicked(int index)
{
    cur_class = index;
    player_class *c_ptr = c_info + index;
    for (int i = 0; i < A_MAX; i++) {
        ui->stats_table->item(i, 1)->setText(format_stat(c_ptr->c_adj[i]));
    }
    ui->stats_table->item(6, 1)->setText(QString::number(c_ptr->c_mhp));
    ui->stats_table->item(7, 1)->setText(QString::number(c_ptr->c_exp).append('%'));
    update_stats();
}

BirthDialog::BirthDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BirthDialog)
{
    ui->setupUi(this);

    cur_class = cur_race = -1;

    for (int i = 0; i < MAX_SEXES; i++) {
        ui->sex_combo->addItem(sex_info[i].title);
    }
    ui->sex_combo->setCurrentIndex(-1);

    QGridLayout *g1 = new QGridLayout();
    ui->raceBox->setLayout(g1);
    QButtonGroup *bg1 = new QButtonGroup();
    connect(bg1, SIGNAL(buttonClicked(int)), this, SLOT(on_bg1_clicked(int)));
    int col = 0;
    int row = 0;
    for (int i = 0; i < z_info->p_max; i++) {
        QRadioButton *radio = new QRadioButton(p_info[i].pr_name);
        bg1->addButton(radio, i);
        g1->addWidget(radio, row, col);
        ++col;
        if (col > 1) {
            col = 0;
            ++row;
        }
    }

    QGridLayout *g2 = new QGridLayout();
    ui->classBox->setLayout(g2);
    col = 0;
    row = 0;
    QButtonGroup *bg2 = new QButtonGroup();
    connect(bg2, SIGNAL(buttonClicked(int)), this, SLOT(on_bg2_clicked(int)));
    for (int i = 0; i < z_info->c_max; i++) {
        QRadioButton *radio = new QRadioButton(c_info[i].cl_name);
        bg2->addButton(radio, i);
        g2->addWidget(radio, row, col);
        ++col;
        if (col > 1) {
            col = 0;
            ++row;
        }
    }

    for (int i = 0; i < ui->stats_table->rowCount(); i++) {
        for (int j = 0; j < ui->stats_table->columnCount(); j++) {
            ui->stats_table->setItem(i, j, new QTableWidgetItem());
        }
    }

    for (int i = 0; i < ui->edit_table->rowCount(); i++) {
        for (int j = 0; j < ui->edit_table->columnCount(); j++) {
            ui->edit_table->setItem(i, j, new QTableWidgetItem());
        }
    }

    ui->stackedWidget->setCurrentIndex(0);
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

void BirthDialog::update_stats()
{
    int p_idx = cur_race;
    int c_idx = cur_class;
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
        if (ui->name_edit->text().trimmed().length() == 0) {
            pop_up_message_box(tr("Complete the character name"), QMessageBox::Critical);
            ui->name_edit->setFocus();
            return;
        }
        if (ui->sex_combo->currentIndex() < 0) {
            pop_up_message_box(tr("Select the character sex"), QMessageBox::Critical);
            ui->sex_combo->setFocus();
            return;
        }
        if (cur_race < 0) {
            pop_up_message_box(tr("Select the character race"), QMessageBox::Critical);
            return;
        }
        if (cur_class < 0) {
            pop_up_message_box(tr("Select the character class"), QMessageBox::Critical);
            return;
        }
        ui->stackedWidget->setCurrentIndex(1);
        ui->prev_button->setEnabled(true);
        ui->next_button->setText(tr("Finish"));

        for (int i = 0; i < A_MAX; i++) {
            int p = p_info[cur_race].r_adj[i];
            int c = c_info[cur_class].c_adj[i];
            ui->edit_table->item(i, 0)->setText(QString::number(10));
            ui->edit_table->item(i, 1)->setText(format_stat(p));
            ui->edit_table->item(i, 2)->setText(format_stat(c));
            ui->edit_table->item(i, 3)->setText(format_stat(p+c));
        }

        ui->edit_table->resizeColumnsToContents();
    }
}

void BirthDialog::on_prev_button_clicked()
{
    int idx = ui->stackedWidget->currentIndex();
    if (idx > 0) {
        --idx;
        ui->stackedWidget->setCurrentIndex(idx);
        ui->next_button->setText(tr("Next"));
    }
    if (idx < 1) {
        ui->prev_button->setEnabled(false);
    }
}
