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

    for (int t = 0; t < ui->tabWidget->count(); t++) {
        QWidget *tab = ui->tabWidget->widget(t);

        QVBoxLayout *l3 = new QVBoxLayout;
        tab->setLayout(l3);

        QScrollArea *area = new QScrollArea();
        area->setWidgetResizable(true);
        l3->addWidget(area);

        QWidget *content = new QWidget;
        area->setWidget(content);        

        QVBoxLayout *l1 = new QVBoxLayout;
        l1->setSpacing(0);
        content->setLayout(l1);                
        content->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);

        char *colors[] = {"#D8F7BB", "white"};
        int cl = 0;

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

            QWidget *row = new QWidget;
            l1->addWidget(row);
            row->setStyleSheet(QString("background-color: %1").arg(colors[cl++ % 2]));            

            QVBoxLayout *l2 = new QVBoxLayout;
            row->setLayout(l2);

            QCheckBox *ck = new QCheckBox(opt->name);            
            ck->setChecked(op_ptr->opt[idx]);
            ck->setProperty("npp_option", QVariant(idx));
            l2->addWidget(ck);

            l2->addWidget(new QLabel(opt->description));
        }
    }
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}


void OptionsDialog::on_buttonBox_clicked(QAbstractButton *button)
{
    if (button->text().compare("Save") == 0) {
        for (int t = 0; t < ui->tabWidget->count(); t++) {
            QWidget *tab = ui->tabWidget->widget(t);

            QList<QCheckBox *> cks = tab->findChildren<QCheckBox *>();

            for (int i = 0; i < cks.count(); i++) {
                int opt_idx = cks.at(i)->property("npp_option").toInt();
                op_ptr->opt[opt_idx] = cks.at(i)->isChecked();
            }
        }
    }

    this->close();
}
