#include "optionsdialog.h"
#include "ui_optionsdialog.h"
#include "npp.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QCheckBox>
#include <QTableWidget>

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog)
{
    ui->setupUi(this);

    for (int t = 0; t < ui->tabWidget->count(); t++) {
        QWidget *tab = ui->tabWidget->widget(t);

        QVBoxLayout *l1 = new QVBoxLayout;
        tab->setLayout(l1);

        QTableWidget *table = new QTableWidget(0, 4);
        table->verticalHeader()->hide();
        table->horizontalHeader()->hide();
        table->hideColumn(3);
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
            item->setCheckState(opt->normal ? Qt::Checked : Qt::Unchecked);
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
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}
