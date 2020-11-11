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
#include <QTimer>

#include "request.h"
#include "utils.h"
#include "notificationpopup.h"
#include "widgets/waitingspinnerwidget.h"
#include "searchwidget.h"
#include "settingswidget.h"

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
public slots:
    void home(bool forceReload = false);
private slots:
    void forceReload();
    void setStyle(QString fname);
    void aboutApp();
    void init();
    void downloadError(QString errorString);
    void processResponse(QByteArray data);
    void get(const QUrl url, const QByteArray payload);
    void show_SysTrayIcon();
    void check_window_state();
    void notify(QString title, QString message);

    void moreApps();
    void searchApps(const QString query);

    void loadedFromCache(const QString cFilePath);
    void rateApp();
    void init_searchWidget();
    void showSettings();
    void init_settings();
    void notifyNewSnaps(QStringList newRevStringList);
    void hideNewLabel(QString snapId);
    void summonHome();
    void updateInfoButtonTooltip();
    void updateAutoRefreshInterval();
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
    SettingsWidget *_settingsWidget = nullptr;
    QTimer *_autoRefreshTimer = nullptr;
};

#endif // MAINWINDOW_H
