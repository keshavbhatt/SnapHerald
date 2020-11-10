#ifndef SEARCHWIDGET_H
#define SEARCHWIDGET_H

#include <QWidget>

namespace Ui {
class searchWidget;
}

class searchWidget : public QWidget
{
    Q_OBJECT

public:
    explicit searchWidget(QWidget *parent = nullptr);
    ~searchWidget();

public slots:
    void clear();
signals:
    void search(const QString query);

private slots:
    void on_search_clicked();

    void on_queryEdit_textChanged(const QString &arg1);

    void on_queryEdit_returnPressed();

private:
    Ui::searchWidget *ui;
};

#endif // SEARCHWIDGET_H
