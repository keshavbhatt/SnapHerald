#include "settingswidget.h"
#include "ui_settingswidget.h"

SettingsWidget::SettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);

    //minmum duration of auto refresh timer
#ifdef QT_DEBUG
    ui->autoRefreshInterval->setMinimum(2);
#else
    ui->autoRefreshInterval->setMinimum(10);
#endif

    if(settings.value("SettingsWidget_geometry").isValid()){
        restoreGeometry(settings.value("SettingsWidget_geometry").toByteArray());
    }

    //load theme
    ui->themeCombo->setCurrentIndex(settings.value("themeCombo",1).toInt());
    //load notification type
    ui->notificationCombo->setCurrentIndex(settings.value("notificationCombo",1).toInt());
    //load auto refresh interval
    //init value, we need this on app init
    if(settings.value("autoRefreshInterval").isValid()==false){
        settings.setValue("autoRefreshInterval",ui->autoRefreshInterval->minimum());
    }
    ui->autoRefreshInterval->setValue(settings.value("autoRefreshInterval",ui->autoRefreshInterval->minimum()).toInt());
    //load run at startup settings
    check_for_startup();
    //load infoActionCombo settings
    ui->infoActionCombo->setCurrentIndex(settings.value("infoActionCombo",0).toInt());
    //load close button action settings
    ui->closeButtonActionCombo->setCurrentIndex(settings.value("closeButtonActionCombo",0).toInt());

}

void SettingsWidget::updateTimeLeftForRefresh(int timeLeft)
{
    ui->timeLeft->setText(QDateTime::fromTime_t(timeLeft/1000).toUTC().toString("hh:mm:ss"));
}

SettingsWidget::~SettingsWidget()
{
    delete ui;
}

void SettingsWidget::on_themeCombo_currentIndexChanged(int index)
{
    settings.setValue("themeCombo",index);
    themeChanged();
}

void SettingsWidget::on_notificationCombo_currentIndexChanged(int index)
{
    settings.setValue("notificationCombo",index);
}

void SettingsWidget::on_autoRefreshInterval_valueChanged(int arg1)
{
    settings.setValue("autoRefreshInterval",arg1);
    emit autoRefreshIntervalChanged();
}


void SettingsWidget::on_infoActionCombo_currentIndexChanged(int index)
{
    settings.setValue("infoActionCombo",index);
    emit infoButtonActionChanged();
}

void SettingsWidget::on_closeButtonActionCombo_currentIndexChanged(int index)
{
    settings.setValue("closeButtonActionCombo",index);
}


//run app at startup
void SettingsWidget::run_onstartup()
{
        QString launcher = QApplication::applicationFilePath();
        QString launcher_name = QApplication::applicationName();
        QString data ="[Desktop Entry]\n"
            "Type=Application\n"
            "Exec="+launcher+" --force-reload\n"
            "Hidden=false\n"
            "NoDisplay=false\n"
            "Name[en_IN]="+launcher_name+"\n"
            "Name="+launcher_name+"\n"
            "X-GNOME-Autostart-enabled=true";
    QString autostartpath = QDir::homePath()+"/.config/autostart";

    QDir dir(autostartpath);
    if (!dir.exists())
        dir.mkpath(autostartpath);
    //check if file exist !!
    QFileInfo file(autostartpath+"/"+launcher_name+".desktop");
    if(file.exists()){
        //remove file and copy file
        QFile desktop_file(autostartpath+"/"+launcher_name+".desktop");
        desktop_file.remove();

        //edit file and add X-GNOME-Autostart-enabled=true
        QFile autostartfile(autostartpath+"/"+launcher_name+".desktop");
        if (autostartfile.open(QIODevice::Append)) {
        QTextStream stream(&autostartfile);
        stream <<data<<endl;}
    }
    //if not exists or for other conditions
    else{
        //edit file and add X-GNOME-Autostart-enabled=true
        QFile autostartfile(autostartpath+"/"+launcher_name+".desktop");
        if (autostartfile.open(QIODevice::Append)) {
        QTextStream stream(&autostartfile);
        stream <<data<<endl;}
    }
}
//do not run app at startup
void SettingsWidget::donot_run_onStartupp()
{
    QString launcher_name = QApplication::applicationName();
    QString autostartpath = QDir::homePath()+"/.config/autostart";
    QFile(autostartpath+"/"+launcher_name+".desktop").remove();
}
//slot for startup toggled
void SettingsWidget::on_runAtStartUp_toggled(bool arg1)
{
    if(arg1){
        run_onstartup();
    }else{
        donot_run_onStartupp();
    }
}
//check if startup file is there .config/autostart
void SettingsWidget::check_for_startup()
{
    QString launcher_name = QApplication::applicationName();
    QString autostartpath = QDir::homePath()+"/.config/autostart";
    QFile autostartfile(autostartpath+"/"+launcher_name+".desktop");
    ui->runAtStartUp->setChecked(QFileInfo(autostartfile).exists());
}

void SettingsWidget::on_reset_clicked()
{
    ui->themeCombo->setCurrentIndex(1);
    ui->notificationCombo->setCurrentIndex(1);
    ui->autoRefreshInterval->setValue(ui->autoRefreshInterval->minimum());
    ui->infoActionCombo->setCurrentIndex(0);
    ui->runAtStartUp->setChecked(true);
    ui->closeButtonActionCombo->setCurrentIndex(0);
}

void SettingsWidget::closeEvent(QCloseEvent *event)
{
    settings.setValue("SettingsWidget_geometry",saveGeometry());
    QWidget::closeEvent(event);
}

void SettingsWidget::on_tryNotification_clicked()
{
    emit testNotify();
}
