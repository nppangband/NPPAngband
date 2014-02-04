#include "ui_birthdialog.h"
#include "optionsdialog.h"
#include "npp.h"
#include "birthdialog.h"
#include <QRadioButton>
#include <QGridLayout>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QToolButton>

static QString format_stat(s16b value)
{
    QString text;
    if (value >= 0) text.append('+');
    return QString("%1%2").arg(text).arg(value);
}

static QString stat_notation(int value)
{
    int right = value - 18;
    if (value > 18) value = 18;
    QString text = QString::number(value);
    if (right > 0) {
        text.append(QString("/"));
        text.append(QString("%1").arg(right, 2, 10, QChar('0')));
    }
    return text;
}

void BirthDialog::update_points()
{
    for (int i = 0; i < A_MAX; i++) {        
        int self = p_ptr->stat_max[i];
        ui->edit_table->item(i, 0)->setText(QString::number(self));

        int cost = points_spent[i];
        ui->edit_table->item(i, 5)->setText(QString::number(cost));

        int best;        
        if (point_based) {
            int effective = ui->edit_table->item(i, 3)->text().toInt();
            best = self + effective;
            int right = best - 18;
            if (best > 18) best = 18;
            if (right > 0) best += (right * 10);
        }
        else {
            best = stats[i];
        }
        ui->edit_table->item(i, 4)->setText(stat_notation(best));
    }
    ui->edit_table->item(6, 4)->setText(QString("Left:"));
    ui->edit_table->item(6, 5)->setText(QString::number(points_left));
    ui->edit_table->resizeColumnToContents(4);
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
    dirty = true;
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
    dirty = true;
}

BirthDialog::BirthDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BirthDialog)
{
    ui->setupUi(this);

    dirty = true;

    cur_class = cur_race = -1;

    for (int i = 0; i < MAX_SEXES; i++) {
        ui->sex_combo->addItem(sex_info[i].title);
    }
    ui->sex_combo->setCurrentIndex(-1);

    // Create the race radios
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

    // Create the class radios
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

    // Fill the tables with empty items
    for (int i = 0; i < ui->stats_table->rowCount(); i++) {
        for (int j = 0; j < ui->stats_table->columnCount(); j++) {
            ui->stats_table->setItem(i, j, new QTableWidgetItem());
        }
    }

    for (int i = 0; i < ui->edit_table->rowCount(); i++) {
        for (int j = 0; j < ui->edit_table->columnCount() - 1; j++) {
            ui->edit_table->setItem(i, j, new QTableWidgetItem());
        }
    }

    // Create the buy/sell buttons for all stats
    for (int i = 0; i < A_MAX; i++) {
        QWidget *container = new QWidget;
        container->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
        QHBoxLayout *l1 = new QHBoxLayout;
        container->setLayout(l1);
        QToolButton *button1 = new QToolButton();
        button1->setProperty("stat_idx", QVariant(i));
        button1->setText("<");
        l1->addWidget(button1);
        connect(button1, SIGNAL(clicked()), this, SLOT(on_sell_clicked()));
        QToolButton *button2 = new QToolButton();
        button2->setProperty("stat_idx", QVariant(i));
        button2->setText(">");
        l1->addWidget(button2);
        connect(button2, SIGNAL(clicked()), this, SLOT(on_buy_clicked()));
        ui->edit_table->setCellWidget(i, 6, container);
        ui->edit_table->resizeRowToContents(i);
    }

    ui->stats_table->setShowGrid(false);
    ui->stats_table->verticalHeader()->setVisible(true);
    ui->edit_table->setShowGrid(false);
    ui->edit_table->verticalHeader()->setVisible(true);

    ui->stackedWidget->setCurrentIndex(0);
}

void BirthDialog::on_sell_clicked()
{
    if (!point_based) return;
    QWidget *button = dynamic_cast<QWidget *>(sender());
    int idx = button->property("stat_idx").toInt();
    if (sell_stat(idx, stats, points_spent, &points_left)) update_points();
}

void BirthDialog::on_buy_clicked()
{
    if (!point_based) return;
    QWidget *button = dynamic_cast<QWidget *>(sender());
    int idx = button->property("stat_idx").toInt();
    if (buy_stat(idx, stats, points_spent, &points_left)) update_points();
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
        // Validations
        if (ui->name_edit->text().trimmed().length() == 0) {
            /*
            pop_up_message_box(tr("Complete the character name"), QMessageBox::Critical);
            ui->name_edit->setFocus();
            return;
            */
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

        // Set global variables
        rp_ptr = p_info + cur_race;
        cp_ptr = c_info + cur_class;

        ui->groupBox1->setTitle(QString("Stat allocation for %1 %2").arg(rp_ptr->pr_name).arg(cp_ptr->cl_name));

        ui->prev_button->setEnabled(true);
        ui->next_button->setText(tr("Finish"));

        // Update race class bonuses
        for (int i = 0; i < A_MAX; i++) {
            int p = p_info[cur_race].r_adj[i];
            int c = c_info[cur_class].c_adj[i];
            ui->edit_table->item(i, 1)->setText(format_stat(p));
            ui->edit_table->item(i, 2)->setText(format_stat(c));
            ui->edit_table->item(i, 3)->setText(format_stat(p+c));
        }

        // Reset to point based generation if race or class changed
        if (dirty) {
            ui->point_radio->click();
            dirty = false;
        }

        update_points();

        ui->edit_table->resizeColumnsToContents();
        ui->stackedWidget->setCurrentIndex(1);
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
    //ui->stats_table->verticalHeader()->show();
}

void BirthDialog::on_point_radio_clicked()
{
    point_based = true;
    reset_stats(stats, points_spent, &points_left);
    generate_stats(stats, points_spent, &points_left);
    ui->edit_table->showColumn(5);
    ui->edit_table->showColumn(6);
    ui->edit_table->showRow(6);
    ui->roll_button->setEnabled(false);
    update_points();
}

void BirthDialog::on_roller_radio_clicked()
{
    point_based = false;
    reset_stats(stats, points_spent, &points_left);
    for (int i = 0; i < A_MAX; i++) {
        points_spent[i] = 0;
    }
    points_left = 0;
    ui->edit_table->hideColumn(5);
    ui->edit_table->hideColumn(6);
    ui->edit_table->hideRow(6);
    ui->roll_button->setEnabled(true);
    ui->roll_button->click();
}

void BirthDialog::on_roll_button_clicked()
{
    if (point_based) return;
    get_stats(stats);
    update_points();
}
