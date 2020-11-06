#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QFile>
#include <QStyleFactory>
#include <QSettings>
#include <QScreen>
#include <QLabel>
#include <QDialog>
#include <QVBoxLayout>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QtNetwork>
#include <QStandardPaths>
#include <QCryptographicHash>
#include <QMessageBox>

#include "request.h"
#include "utils.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected slots:
    void closeEvent(QCloseEvent *event);
private slots:
    void setStyle(QString fname);
    void aboutApp();
    void init();
    void downloadError(QString errorString);
    void processResponse(QByteArray data);
    void get(const QUrl url);
private:
    Ui::MainWindow *ui;
    QSettings settings;
    Request *request = nullptr;

    QString api_end_point,_cache_path,fields;
    QNetworkAccessManager * m_netwManager = nullptr;
};

#endif // MAINWINDOW_H
