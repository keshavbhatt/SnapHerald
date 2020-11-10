#include "searchwidget.h"
#include "ui_searchwidget.h"

searchWidget::searchWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::searchWidget)
{
    ui->setupUi(this);
    ui->search->setEnabled(!ui->queryEdit->text().isEmpty());
}

searchWidget::~searchWidget()
{
    delete ui;
}

void searchWidget::clear()
{
    ui->queryEdit->clear();
}

void searchWidget::on_search_clicked()
{
    emit search(ui->queryEdit->text());
}

void searchWidget::on_queryEdit_textChanged(const QString &arg1)
{
    ui->search->setEnabled(!arg1.isEmpty());
}

void searchWidget::on_queryEdit_returnPressed()
{
    if(ui->search->isEnabled())
        on_search_clicked();
}
