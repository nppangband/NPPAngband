#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include "npp.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QCheckBox>
#include <QTableWidget>
#include <QLabel>

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);

    for (int t = 0; t < 5; t++) {
         QWidget *tab = ui->tabWidget->widget(t);

         QVBoxLayout *l1 = new QVBoxLayout;
         tab->setLayout(l1);

         QTableWidget *table = new QTableWidget(0, 4);
         table->verticalHeader()->hide();
         table->horizontalHeader()->hide();
         table->hideColumn(3);
         table->setShowGrid(false);
         table->setAlternatingRowColors(true);
         l1->addWidget(table);

         for (int i = 0, j = 0; i < OPT_PAGE_PER; i++) {
             byte idx;
             if ((game_mode == GAME_NPPANGBAND) || (game_mode == GAME_MODE_UNDEFINED)) {
                 idx = option_page_nppangband[t][i];
             }
             else {
                 idx = option_page_nppmoria[t][i];
             }
             if (idx == OPT_NONE) continue;

             option_entry *opt = options + idx;
             if (opt->name == NULL) continue;

             table->insertRow(j);

             QTableWidgetItem *item = new QTableWidgetItem;
             item->setCheckState(op_ptr->opt[idx] ? Qt::Checked : Qt::Unchecked);
             table->setItem(j, 0, item);

             item = new QTableWidgetItem(opt->name);
             table->setItem(j, 1, item);

             item = new QTableWidgetItem(opt->description);
             table->setItem(j, 2, item);

             // Save the option number in the hidden column
             table->setItem(j, 3, new QTableWidgetItem(QString::number(idx)));

             ++j;
         }

         table->resizeColumnsToContents();
     }

     ui->spin_base_delay->setValue(op_ptr->delay_factor);
     ui->spin_hitpoint_warning->setValue(op_ptr->hitpoint_warn);
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if (button->text().compare("Save") == 0) {
        for (int t = 0; t < 5; t++) {
            QWidget *tab = ui->tabWidget->widget(t);

            QTableWidget *table = tab->findChild<QTableWidget *>();
            for (int i = 0; i < table->rowCount(); i++) {
                QTableWidgetItem *item_check = table->item(i, 0);
                QTableWidgetItem *item_idx = table->item(i, 3);
                int opt_idx = item_idx->text().toInt();
                op_ptr->opt[opt_idx] = (item_check->checkState() == Qt::Checked);
            }
        }

        op_ptr->delay_factor = ui->spin_base_delay->value();
        op_ptr->hitpoint_warn = ui->spin_hitpoint_warning->value();
    }

    this->close();
}
