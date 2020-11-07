#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSystemTrayIcon>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle(QApplication::applicationName());
    this->setWindowIcon(QIcon(":/icons/app/icon-128.png"));
    setStyle(":/qbreeze/dark.qss");
    if(settings.value("geometry").isValid()){
        restoreGeometry(settings.value("geometry").toByteArray());
        if(settings.value("windowState").isValid()){
            restoreState(settings.value("windowState").toByteArray());
        }else{
            QScreen* pScreen = QApplication::primaryScreen();// (this->mapToGlobal({this->width()/2,0}));
            QRect availableScreenSize = pScreen->availableGeometry();
            this->move(availableScreenSize.center()-this->rect().center());
        }
    }
    ui->mainToolBar->setMovable(false);
    ui->mainToolBar->addAction(QIcon(":/icons/information-line.png"),tr("About"),this,SLOT(aboutApp()));
    show_SysTrayIcon();
    init();
}

void MainWindow::init()
{
    _cache_path   = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    api_end_point = "https://snapstats.org/";

    m_netwManager = new QNetworkAccessManager(this);
    QNetworkDiskCache* diskCache = new QNetworkDiskCache(this);
    diskCache->setCacheDirectory(_cache_path);
    m_netwManager->setCache(diskCache);
    connect(m_netwManager,&QNetworkAccessManager::finished,[=](QNetworkReply* rep){
        if(rep->error() == QNetworkReply::NoError){
            QByteArray response = rep->readAll();
            QJsonDocument jsonDoc = QJsonDocument::fromJson(response);
            if(!jsonDoc.isEmpty()){
                QString reqUrlStr = rep->request().url().toString();
                QString ci_id = QString(QCryptographicHash::hash((reqUrlStr.toUtf8()),QCryptographicHash::Md5).toHex());
                utils::saveJson(jsonDoc,utils::returnPath("store_cache")+ci_id);
            }
            processResponse(response);
        }else{
            downloadError(rep->errorString());
        }
        rep->deleteLater();
    });

    //load home
    get(api_end_point+"graphql");
}

void MainWindow::show_SysTrayIcon(){

      QAction *minimizeAction = new QAction(QObject::tr("&Hide"), this);
      this->connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

      QAction *restoreAction = new QAction(QObject::tr("&Restore"), this);
      this->connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

      QAction *quitAction = new QAction(QObject::tr("&Quit"), this);
      this->connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

      QMenu *trayIconMenu = new QMenu(this);
      trayIconMenu->addAction(minimizeAction);
      trayIconMenu->addAction(restoreAction);
      trayIconMenu->addSeparator();
      trayIconMenu->addAction(quitAction);

      trayIcon = new QSystemTrayIcon(this);
      trayIcon->setContextMenu(trayIconMenu);
      trayIconMenu->setObjectName("trayIconMenu");

      trayIcon->setIcon(QIcon(":/icons/app/icon-32.png"));
      connect(trayIconMenu,SIGNAL(aboutToShow()),this,SLOT(check_window_state()));
      if(trayIcon->isSystemTrayAvailable()){
          trayIcon->show();
      }else{
          trayIcon->deleteLater();
          trayIcon = nullptr;
          QMessageBox::information(this,"System tray:","System tray not found, Please install a system tray in your system.",QMessageBox::Ok);
      }
      // init widget notification
      notificationPopup = new NotificationPopup(0);
      notificationPopup->setWindowFlags(Qt::ToolTip | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);
      notificationPopup->adjustSize();
}

//check window state and set tray menus
void MainWindow::check_window_state()
{
    QObject *tray_icon_menu = this->findChild<QObject*>("trayIconMenu");
    if(tray_icon_menu != nullptr){
        if(this->isVisible()){
            ((QMenu*)(tray_icon_menu))->actions().at(0)->setDisabled(false);
            ((QMenu*)(tray_icon_menu))->actions().at(1)->setDisabled(true);
        }else{
            ((QMenu*)(tray_icon_menu))->actions().at(0)->setDisabled(true);
            ((QMenu*)(tray_icon_menu))->actions().at(1)->setDisabled(false);
        }
    }
}

void MainWindow::get(const QUrl url)
{
    QString reqUrlStr = url.toString();
    QString ci_id = QString(QCryptographicHash::hash((reqUrlStr.toUtf8()),QCryptographicHash::Md5).toHex());
    QFileInfo cFile(utils::returnPath("store_cache")+ci_id);
    if(cFile.isFile() && cFile.exists() && cFile.size()!=0){
        processResponse(utils::loadJson(cFile.filePath()).toJson());
        //emit loadedFromCache(cFile.absoluteFilePath());
        qDebug()<<"data loaded from local cache";
    }else{
        QNetworkRequest request(url);
        request.setRawHeader("user-agent","Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/84.0.4147.125 Safari/537.36");
        request.setRawHeader("authority","snapstats.org");
        request.setRawHeader("accept","*/*");
        request.setRawHeader("dnt","1");
        request.setRawHeader("referer","https://snapstats.org/snaps?q=&offset=0&limit=20&field=date_published&order=-1");
        request.setRawHeader("content-type" ,"application/json");
        request.setRawHeader("origin","https://snapstats.org");
        request.setRawHeader("sec-fetch-site","same-origin");
        request.setRawHeader("sec-fetch-mode","cors");
        request.setRawHeader("sec-fetch-dest","empty");
        request.setRawHeader("accept-language","en-GB,en;q=0.9,hi-IN;q=0.8,hi;q=0.7,en-US;q=0.6,lb;q=0.5");
        QByteArray post_data;
        post_data.append("{\"operationName\":null,\"variables\":{\"q\":\"\",\"field\":\"date_published\",\"order"
                         "\":-1,\"offset\":0,\"limit\":20},\"query\":\"query ($q: String!, $offset: Int!, $limit: Int!,"
                         " $field: String!, $order: Int!) {\\n  findSnapsByName(name: $q, query: {offset: $offset,"
                         " limit: $limit, sort: {field: $field, order: $order}}) {\\n    snap_id\\n    "
                         "package_name\\n    title\\n    summary\\n    icon_url\\n    description\\n   "
                         " __typename\\n  }\\n  findSnapsByNameCount(name: $q) {\\n    count\\n    __typename\\n"
                         "  }\\n}\\n\"}");
        m_netwManager->post(request,post_data);
    }
}

void MainWindow::downloadError(QString errorString)
{
    QMessageBox::critical(this,"Error",errorString,QMessageBox::Ok);
}

void MainWindow::processResponse(QByteArray data)
{
    QString dataString = QTextCodec::codecForMib(106)->toUnicode(data);

}

void MainWindow::notify(QString title, QString message)
{
    //TODO: check settings for notification popup type
    if(settings.value("nativeNotifiaction").toBool() && trayIcon != nullptr){
        trayIcon->showMessage(title,message,QSystemTrayIcon::Information,100);
    }else{
        //fallback to custom widget based notification widget
        notificationPopup->present(title,message,QImage(":/icons/information-fill.png"));
    }
}

void MainWindow::aboutApp()
{
    notify("About","Opened");
    QDialog *aboutDialog = new QDialog(this,Qt::Dialog);
    aboutDialog->setWindowModality(Qt::WindowModal);
    QVBoxLayout *layout = new QVBoxLayout;
    QLabel *message = new QLabel(aboutDialog);
    layout->addWidget(message);
    aboutDialog->setLayout(layout);
    aboutDialog->setAttribute(Qt::WA_DeleteOnClose,true);
    aboutDialog->show();

    QString mes =
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><img src=':/icons/app/icon-64.png' /></p>"
                 "<p align='center' style='-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><br /></p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>Designed and Developed</p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>by <span style=' font-weight:600;'>Keshav Bhatt</span> &lt;keshavnrj@gmail.com&gt;</p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>Website: https://ktechpit.com</p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>Runtime: Qt 5.5.1</p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>Version: "+QApplication::applicationVersion()+"</p>"
                 "<p align='center' style='-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><br /></p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><a href='https://snapcraft.io/search?q=keshavnrj'>More Apps</p>"
                 "<p align='center' style='-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><br /></p>";

    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
    message->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
    a->setDuration(1000);
    a->setStartValue(0);
    a->setEndValue(1);
    a->setEasingCurve(QEasingCurve::InCurve);
    a->start(QPropertyAnimation::DeleteWhenStopped);
    message->setText(mes);
    message->show();
}

void MainWindow::setStyle(QString fname)
{
    QFile styleSheet(fname);
    if (!styleSheet.open(QIODevice::ReadOnly))
    {
        qWarning("Unable to open file");
        return;
    }
    qApp->setStyleSheet(styleSheet.readAll());
    styleSheet.close();
    qApp->setStyle(QStyleFactory::create("Fusion"));
    QPalette palette;
    palette.setColor(QPalette::Window,QColor(53,53,53));
    palette.setColor(QPalette::WindowText,Qt::white);
    palette.setColor(QPalette::Disabled,QPalette::WindowText,QColor(127,127,127));
    palette.setColor(QPalette::Base,QColor(42,42,42));
    palette.setColor(QPalette::AlternateBase,QColor(66,66,66));
    palette.setColor(QPalette::ToolTipBase,Qt::white);
    palette.setColor(QPalette::ToolTipText,QColor(53,53,53));
    palette.setColor(QPalette::Text,Qt::white);
    palette.setColor(QPalette::Disabled,QPalette::Text,QColor(127,127,127));
    palette.setColor(QPalette::Dark,QColor(35,35,35));
    palette.setColor(QPalette::Shadow,QColor(20,20,20));
    palette.setColor(QPalette::Button,QColor(53,53,53));
    palette.setColor(QPalette::ButtonText,Qt::white);
    palette.setColor(QPalette::Disabled,QPalette::ButtonText,QColor(127,127,127));
    palette.setColor(QPalette::BrightText,Qt::red);
    palette.setColor(QPalette::Link,QColor("skyblue"));
    palette.setColor(QPalette::Highlight,QColor(49,106,150));
    palette.setColor(QPalette::Disabled,QPalette::Highlight,QColor(80,80,80));
    palette.setColor(QPalette::HighlightedText,Qt::white);
    palette.setColor(QPalette::Disabled,QPalette::HighlightedText,QColor(127,127,127));
    qApp->setPalette(palette);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    //TODO: impliment default window close method (hide to tray or quit)
    settings.setValue("geometry",saveGeometry());
    settings.setValue("windowState", saveState());
    QWidget::closeEvent(event);
}

MainWindow::~MainWindow()
{
    delete ui;
}
