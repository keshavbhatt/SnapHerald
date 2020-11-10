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
#include <QSystemTrayIcon>
#include <QMenu>

#include "request.h"
#include "utils.h"
#include "notificationpopup.h"
#include "widgets/waitingspinnerwidget.h"
#include "searchwidget.h"

#include "ui_track.h"

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
    void get(const QUrl url, const QByteArray payload);
    void show_SysTrayIcon();
    void check_window_state();
    void notify(QString title, QString message);
    void donot_run_onStartupp();
    void check_for_startup();
    void run_onstartup();
    void on_runAtStartUp_toggled(bool arg1);
    void moreApps();
    void searchApps(const QString query);
    void home();
    void forceReload();
    void loadedFromCache(const QString cFilePath);
    void rateApp();
    void init_searchWidget();
    void on_autoRefreshInterval_valueChanged(int arg1);

private:
    Ui::MainWindow *ui;
    Ui::track track_ui;
    QSettings settings;
    Request *request = nullptr;

    QString api_end_point,_cache_path;
    QNetworkAccessManager * m_netwManager = nullptr;

    QSystemTrayIcon *trayIcon = nullptr;
    NotificationPopup *notificationPopup = nullptr;

    double trackCoverWidth = 0;
    QSize trackWidgetSizeHint;

    WaitingSpinnerWidget *_loader = nullptr;

    QByteArray currentPayload;
    QUrl currentUrl;
    QAction *reloadAction;
    searchWidget *_searchWidget = nullptr;
};

#endif // MAINWINDOW_H
