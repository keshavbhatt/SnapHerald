#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QSystemTrayIcon>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setWindowTitle(QApplication::applicationName());
    this->setWindowIcon(QIcon(":/icons/app/icon-128.png"));
    //theme settings mapped to values of indexes of themeCombo widget in SettingsWidget class
    int theme = settings.value("themeCombo",1).toInt();
    setStyle(":/qbreeze/"+QString(theme == 1 ? "dark" : "light") +".qss");
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

    _loader = new WaitingSpinnerWidget(ui->results,true,true);
    _loader->setRoundness(70.0);
    _loader->setMinimumTrailOpacity(15.0);
    _loader->setTrailFadePercentage(70.0);
    _loader->setNumberOfLines(10);
    _loader->setLineLength(8);
    _loader->setLineWidth(2);
    _loader->setInnerRadius(2);
    _loader->setRevolutionsPerSecond(3);
    _loader->setColor(QColor("#1e90ff"));

    ui->mainToolBar->addAction(QIcon(":/icons/home-8-line.png"),tr("Home"),this,SLOT(home()));
    reloadAction = ui->mainToolBar->addAction(QIcon(":/icons/refresh-line.png"),tr("Reload"),this,SLOT(forceReload()));
    reloadAction->setToolTip(tr("Results loaded from cache,\nClick to refresh cache"));
    reloadAction->setToolTip(tr("Refresh cache"));
    reloadAction->setEnabled(false);
    init_searchWidget();
    ui->mainToolBar->addWidget(_searchWidget);
    QWidget* hSpacer= new QWidget(this);
    hSpacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    ui->mainToolBar->addWidget(hSpacer);
    init_settings();
    ui->mainToolBar->addAction(QIcon(":/icons/setting-line.png"),tr("Settings"),this,SLOT(showSettings()));
    ui->mainToolBar->addAction(QIcon(":/icons/others/snapcraft.png"),tr("More Apps"),this,SLOT(moreApps()));
    ui->mainToolBar->addAction(QIcon(":/icons/star-line.png"),tr("Rate"),this,SLOT(rateApp()));
    ui->mainToolBar->addAction(QIcon(":/icons/information-line.png"),tr("About"),this,SLOT(aboutApp()));
    show_SysTrayIcon();
    init();
}

void MainWindow::init_searchWidget()
{
    _searchWidget = new searchWidget(this);
    _searchWidget->setWindowFlag(Qt::Widget);
    _searchWidget->setContentsMargins(0,0,0,0);
    _searchWidget->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding);
    _searchWidget->setMinimumWidth(280);
    connect(_searchWidget,&searchWidget::search,[=](const QString query){
       searchApps(query);
    });
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
        _loader->stop();
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
}

void MainWindow::show_SysTrayIcon(){

      QAction *minimizeAction = new QAction(QObject::tr("&Hide"), this);
      this->connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide()));

      QAction *restoreAction = new QAction(QObject::tr("&Restore"), this);
      this->connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

      QAction *quitAction = new QAction(QObject::tr("&Quit"), this);
      this->connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

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
      connect(notificationPopup,&NotificationPopup::notification_clicked,[=](){
          this->show();
          this->setWindowState(this->windowState() & ~Qt::WindowMinimized | Qt::WindowActive);
          this->raise();
      });
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

void MainWindow::get(const QUrl url,const QByteArray payload)
{
    int cacheTimeOut = (settings.value("autoRefreshInterval").toInt()-1) * 60;
    ui->results->clear();
    _loader->start();
    QString reqUrlStr = url.toString();
    QString ci_id = QString(QCryptographicHash::hash((reqUrlStr.toUtf8()),QCryptographicHash::Md5).toHex());
    QFileInfo cFile(utils::returnPath("store_cache")+ci_id);
    qDebug()<<QDateTime::currentSecsSinceEpoch() - cFile.lastModified().toSecsSinceEpoch();
    bool cacheExpired = (QDateTime::currentSecsSinceEpoch() - cFile.lastModified().toSecsSinceEpoch()) > cacheTimeOut ? true : false;
    if( !cacheExpired && cFile.isFile() && cFile.exists() && cFile.size()!=0 ){
        processResponse(utils::loadJson(cFile.filePath()).toJson());
        loadedFromCache(cFile.absoluteFilePath());
        qDebug()<<"data loaded from local cache";
    }else{
        reloadAction->setToolTip(tr("Refresh cache"));
        reloadAction->setEnabled(false); // we loading fresh data so disable reload button
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
        m_netwManager->post(request,payload);
    }
}

void MainWindow::loadedFromCache(const QString cFilePath)
{
    reloadAction->setEnabled(true);
    reloadAction->setToolTip(tr("Results loaded from cache,\nClick to refresh cache"));
    reloadAction->disconnect();
    connect(reloadAction,&QAction::triggered,[=](){
        QFile c_file(cFilePath);
        if(c_file.exists()){
            if(c_file.remove()){
                forceReload();
            }
        }
    });
}

void MainWindow::downloadError(QString errorString)
{
    QMessageBox::critical(this,"Error",errorString,QMessageBox::Ok);
}

void MainWindow::processResponse(QByteArray data)
{
    ui->results->clear();
    QStringList lastRevStringList, newRevStringList;
    bool isHome = currentUrl.toString().contains("?home");
    if(isHome){
        //read last file
        QFile lastFile(utils::returnPath("revisions")+"/last");
        if(lastFile.exists()){
           QJsonDocument lastRevJson = utils::loadJson(lastFile.fileName());
           if(!lastRevJson.isEmpty()){
               foreach (QJsonValue val, lastRevJson.array()) {
                   lastRevStringList.append(val.toString());
               }
           }
        }
    }

    QString dataString = QTextCodec::codecForMib(106)->toUnicode(data);
    QJsonDocument jsonResponse = QJsonDocument::fromJson(dataString.toUtf8());
    QString snapId,title,summary,developer_name,iconUrl,p_name,date_published;
    if(jsonResponse.isEmpty())
    {
        QMessageBox::critical(this,tr("Error"),tr("No data returned from API"),QMessageBox::Ok);
    }else{
        QJsonArray sIdArray;
        QJsonObject dataObj   = jsonResponse.object().value("data").toObject();
        QJsonArray snapsArray = dataObj.value("findSnapsByName").toArray();
        foreach (QJsonValue value, snapsArray)
        {
            QJsonObject obj = value.toObject();
            //only add snaps
            if(obj.value("__typename").toString()=="Snap")
            {
                title   = obj.value("title").toString();
                summary = obj.value("summary").toString();
                iconUrl = obj.value("icon_url").toString();
                snapId  = obj.value("snap_id").toString();
                p_name  = obj.value("package_name").toString();
                developer_name = obj.value("developer_name").toString();
                date_published = QDateTime::fromMSecsSinceEpoch(obj.value("date_published").toDouble()).date().toString();

                //save to lastRev
                if(isHome){
                    QJsonDocument jsonDoc;
                    sIdArray.append(QJsonValue(snapId));
                    //append new snaps to newRevStringList
                    if(lastRevStringList.contains(snapId)==false)
                        newRevStringList.append(snapId);
                    jsonDoc.setArray(sIdArray);
                    if(!jsonDoc.isNull())
                        utils::saveJson(jsonDoc,utils::returnPath("revisions")+"/last");
                }

                QWidget *track_widget = new QWidget(ui->results);
                track_widget->setObjectName("track-widget-"+snapId);
                track_ui.setupUi(track_widget);
                track_widget->setStyleSheet("QWidget#"+track_widget->objectName()+"{background-color: transparent;}");
                track_ui.title->setText(title);
                track_ui.summary->setText(summary);
                track_ui.meta->setText(tr("Published on ")+date_published+tr(" by <b>")+developer_name+"</b>");
                track_widget->setToolTip(title+" ("+summary+")");
                track_ui.new_snap->hide();

                double ratio  = 200.0/200.0; //actual image aspect ratio
                double height = track_widget->minimumSizeHint().height();
                if(trackCoverWidth==0){
                    trackCoverWidth  = ratio * height; //calculated width based on ratio
                    trackWidgetSizeHint = track_widget->minimumSizeHint();
                }
                track_ui.icon->setFixedSize(trackCoverWidth,trackCoverWidth);
                track_ui.icon->setRemotePixmap(iconUrl,"qrc:/icons/others/snapcraft.png");

                connect(track_ui.info,&QPushButton::clicked,[=](){
                    if(settings.value("infoActionCombo",0).toInt()==0)
                        QDesktopServices::openUrl(QUrl("https://snapcraft.io/"+p_name));
                    else
                        QDesktopServices::openUrl(QUrl("snap://"+p_name));
                    hideNewLabel(snapId);
                });
                QListWidgetItem* item;
                item = new QListWidgetItem(ui->results);
                item->setSizeHint(trackWidgetSizeHint);
                ui->results->setItemWidget(item,track_widget);
                ui->results->addItem(item);
            }
        }
        if(isHome)
            notifyNewSnaps(newRevStringList);
    }
    _loader->stop();
}

void MainWindow::hideNewLabel(QString snapId)
{
    QWidget *listWidget = ui->results->findChild<QWidget*>("track-widget-"+snapId);
    listWidget->findChild<QLabel*>("new_snap")->hide();
}

void MainWindow::notifyNewSnaps(QStringList newRevStringList)
{
    int count = newRevStringList.count();
    foreach (QString snapId, newRevStringList) {
        QWidget *listWidget = ui->results->findChild<QWidget*>("track-widget-"+snapId);
        listWidget->findChild<QLabel*>("new_snap")->show();
    }
    if(count>0)
       notify(tr("New snaps published"),QString::number(count)+" new snap"+QString(count > 1 ?"s":"")+" in store.");
}


void MainWindow::notify(QString title, QString message)
{
    //TODO: check settings for notification popup type
    if(settings.value("notificationCombo",1).toInt() == 0 && trayIcon != nullptr){
        trayIcon->showMessage(title,message,QSystemTrayIcon::Information,100);
    }else{
        //fallback to custom widget based notification widget
        notificationPopup->present(title,message,QPixmap(":/icons/information-fill.png"));
    }
}

void MainWindow::summonHome()
{
    _searchWidget->clear();
    ui->resultInfo->setText(tr("Recently published snaps"));
    //load home
    currentPayload.clear();
    currentPayload.append("{\"operationName\":null,\"variables\":{\"q\":\"\",\"field\":\"date_published\",\"order"
                     "\":-1,\"offset\":0,\"limit\":20},\"query\":\"query ($q: String!, $offset: Int!, $limit: Int!,"
                     " $field: String!, $order: Int!) {  findSnapsByName(name: $q, query: {offset: $offset,"
                     " limit: $limit, sort: {field: $field, order: $order}}) {    snap_id    "
                     "package_name    title    summary    icon_url   developer_name   date_published   "
                     " __typename  }  findSnapsByNameCount(name: $q) {    count    __typename"
                     "  }}\"}");
    currentUrl = api_end_point+"graphql?home";
}

void MainWindow::home(bool forceReload)
{
    summonHome();
    if(forceReload){
        QString ci_id = QString(QCryptographicHash::hash((currentUrl.toString().toUtf8()),QCryptographicHash::Md5).toHex());
        QFileInfo cFile(utils::returnPath("store_cache")+ci_id);
        QFile c_file(cFile.absoluteFilePath());
        if(c_file.exists()){
            if(c_file.remove()){
                get(currentUrl,currentPayload);
            }
        }
    }else{
        get(currentUrl,currentPayload);
    }
}

void MainWindow::init_settings()
{
    _settingsWidget = new SettingsWidget(this);
    _settingsWidget->setWindowFlag(Qt::Dialog);
    _settingsWidget->setWindowTitle(QApplication::applicationName()+" | "+tr("Settings"));

    connect(_settingsWidget,&SettingsWidget::testNotify,[=](){
       notify(QApplication::applicationName(),tr("Test Notification"));
    });
    connect(_settingsWidget,&SettingsWidget::themeChanged,[=](){
        //theme settings mapped to values of indexes of themeCombo widget in SettingsWidget class
        int theme = settings.value("themeCombo",1).toInt();
        setStyle(":/qbreeze/"+QString(theme == 1 ? "dark" : "light") +".qss");
    });
}

void MainWindow::showSettings()
{
    _settingsWidget->show();
    _settingsWidget->raise();
}

void MainWindow::moreApps()
{
    QDesktopServices::openUrl(QUrl("https://snapcraft.io/search?q=keshavnrj"));
}

void MainWindow::forceReload()
{
    get(currentUrl,currentPayload);
}

void MainWindow::searchApps(const QString query)
{
    ui->resultInfo->setText(tr("Results for %1").arg(query));
    currentPayload.clear();
    currentPayload.append("{\"operationName\":null,\"variables\":{\"q\":\""+query+"\",\"field\":\"date_published\",\"order"
                     "\":1,\"offset\":0,\"limit\":50},\"query\":\"query ($q: String!, $offset: Int!, $limit: Int!,"
                     " $field: String!, $order: Int!) {  findSnapsByName(name: $q, query: {offset: $offset,"
                     " limit: $limit, sort: {field: $field, order: $order}}) {    snap_id    "
                     "package_name    title    summary    icon_url   developer_name   date_published   "
                     " __typename  }  findSnapsByNameCount(name: $q) {    count    __typename"
                     "  }}\"}");
    currentUrl = api_end_point+"graphql?"+query;
    get(currentUrl,currentPayload);
}

void MainWindow::rateApp()
{
    QDesktopServices::openUrl(QUrl("snap://snap-herald"));
}

void MainWindow::aboutApp()
{
    QDialog *aboutDialog = new QDialog(this,Qt::Dialog);
    aboutDialog->setWindowModality(Qt::WindowModal);
    QVBoxLayout *layout = new QVBoxLayout;
    QLabel *message = new QLabel(aboutDialog);
    layout->addWidget(message);
    connect(message,&QLabel::linkActivated,[=](const QString linkStr){
        if(linkStr.contains("about_qt")){
            qApp->aboutQt();
        }else{
            QDesktopServices::openUrl(QUrl(linkStr));
        }
    });
    aboutDialog->setLayout(layout);
    aboutDialog->setAttribute(Qt::WA_DeleteOnClose,true);
    aboutDialog->show();

    QString mes =
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><img src=':/icons/app/icon-64.png' /></p>"
                 "<p align='center' style='-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><br /></p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>Designed and Developed</p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>by <span style=' font-weight:600;'>Keshav Bhatt</span> &lt;keshavnrj@gmail.com&gt;</p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>Website: https://ktechpit.com</p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>Runtime: <a style='text-decoration:none' href='http://about_qt'>Qt Toolkit</a></p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'>Version: "+QApplication::applicationVersion()+"</p>"
                 "<p align='center' style='-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><br /></p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><a href='https://snapcraft.io/search?q=keshavnrj'>More Apps</p>"
                 "<p align='center' style='-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><br /></p>"
                 "<p align='center' style=' margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px;'><a href='https://snapstats.org/about'>Powered by snapstats.org</p>"
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
    palette.setColor(QPalette::Link,QColor("#3DAEE9"));
    palette.setColor(QPalette::Highlight,QColor(49,106,150));
    palette.setColor(QPalette::Disabled,QPalette::Highlight,QColor(80,80,80));
    palette.setColor(QPalette::HighlightedText,Qt::white);
    palette.setColor(QPalette::Disabled,QPalette::HighlightedText,QColor(127,127,127));
    qApp->setPalette(palette);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if(QSystemTrayIcon::isSystemTrayAvailable() && settings.value("closeButtonActionCombo",0).toInt() == 0){
        this->hide();
        event->ignore();
        notify(QApplication::applicationName(),"Application is minimized to system tray.");
        return;
    }
    settings.setValue("geometry",saveGeometry());
    settings.setValue("windowState", saveState());
    QWidget::closeEvent(event);
}

MainWindow::~MainWindow()
{
    delete ui;
}
