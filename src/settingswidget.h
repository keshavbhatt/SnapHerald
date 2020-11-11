#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include <QSettings>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QScreen>


namespace Ui {
class SettingsWidget;
}

class SettingsWidget : public QWidget
{
    Q_OBJECT

signals:
    void themeChanged();
    void testNotify();

public:
    explicit SettingsWidget(QWidget *parent = nullptr);
    ~SettingsWidget();

protected slots:
    void closeEvent(QCloseEvent *event);

private slots:
    void on_themeCombo_currentIndexChanged(int index);

    void on_notificationCombo_currentIndexChanged(int index);

    void on_autoRefreshInterval_valueChanged(int arg1);

    void on_runAtStartUp_toggled(bool checked);

    void on_infoActionCombo_currentIndexChanged(int index);

    void on_closeButtonActionCombo_currentIndexChanged(int index);

    void donot_run_onStartupp();

    void check_for_startup();

    void run_onstartup();

    void on_reset_clicked();

    void on_tryNotification_clicked();

private:
    Ui::SettingsWidget *ui;
    QSettings settings;
};

#endif // SETTINGSWIDGET_H
