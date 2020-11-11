#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include "rungaurd.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QString appname = "SnapHerald";
    RunGuard guard("org.keshavnrj.ubuntu."+appname);
        if ( !guard.tryToRun() ){
            QMessageBox::critical(0, appname,"An instance of "+appname+" is already running.");
            return 0;
        }
    QApplication::setApplicationName(appname);
    QApplication::setOrganizationName("org.keshavnrj.ubuntu");
    QApplication::setApplicationVersion(VERSIONSTR);

    MainWindow w;
    QStringList argsList = a.arguments();
    if(argsList.contains("--force-reload",Qt::CaseSensitive)){
        w.home(true);
    }else{
        w.home(false);
        w.show();
    }

    return a.exec();
}
