// OFFIS Automation Framework
// Copyright (C) 2013-2016 OFFIS e.V.
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <QApplication>
#include <QSettings>
#include <QPainter>
#include <QDesktopWidget>
#include <QSplashScreen>
#include <QStyleFactory>
#include <Notifications.h>

#include "MasterWindow.h"
#include "PluginLoader.h"
#include <interfaces/logging.h>
#include "DockWidgetTitle.h"
#include <LogWindow.h>
#include "PerspectiveControl.h"
#include "Application.h"
#include "qglobal.h"
#include "TranslationLoader.h"
#include "version.h"

int main(int argc, char *argv[])
{
    Application a(argc, argv);
    a.setLibraryPaths(a.libraryPaths() << a.applicationDirPath() + "/plugins");
    a.setOrganizationName("OFFIS");
    a.setApplicationName("Automation Framework");
    a.setStyle(QStyleFactory::create("Fusion"));

    // Splash image
    QPixmap* splashPicture = new QPixmap(":/img/SplashScreen.png");
    QString versionString = Version::versionString();
    QPainter* painter=new QPainter(splashPicture);
    painter->drawText(QRect(0, 16, 250, 20), Qt::AlignCenter, versionString);

    // Create splash screen
    QSettings::setDefaultFormat(QSettings::IniFormat);
    QFileInfo settingsFile(QSettings().fileName());
    if(!settingsFile.exists()){
        QFileInfo oldFile = settingsFile.dir().absoluteFilePath("Automation Toolbox.ini");
        if(oldFile.exists()){
            QFile::rename(oldFile.absoluteFilePath(), settingsFile.absoluteFilePath());
        }
    }
    QSplashScreen *splash = new QSplashScreen;
    splash->setPixmap(*splashPicture);
    // Show
    splash->show();
    TranslationLoader translator;
    translator.installSystemTranslator();
    splash->showMessage(a.translate("splash screen", "Starting"), Qt::AlignHCenter);

    a.processEvents();
    QDesktopWidget* desktop = a.desktop();
    bool multiScreen = desktop->screenCount() > 1;
    int retVal;
    if(a.arguments().contains("--singleScreen")){
        multiScreen = false;
    }
    bool noload = a.arguments().contains("--noload");

#ifdef Q_OS_MAC
    PluginLoader loader(a.applicationDirPath() + "/../../../plugins");
#else
    PluginLoader loader(a.applicationDirPath() + "/plugins");
#endif

    QSettings settings;
    {
        QDockWidget* logWindow = new LogWindow();
        MasterWindow* master = new MasterWindow();
        MainWindow* slave = 0;
        if(multiScreen){
            QString title = master->windowTitle();
            QString titleDetail = QString("%1 %2.%3.%4").arg(title).arg(Version::MAJOR).arg(Version::MINOR).arg(Version::PATCHLEVEL);
            slave = new MainWindow();
            master->setWindowTitle(a.translate("MainWindow", "%1 - Main window").arg(titleDetail));
            slave->setWindowTitle(a.translate("MainWindow", "%1 - Second window").arg(titleDetail));
        }
        QObject::connect(master, SIGNAL(openProject(QString)), &loader, SLOT(openProject(QString)));
        QObject::connect(master, SIGNAL(closeProject()), &loader, SLOT(closeProject()));
        loader.load(splash);
        PerspectiveControl perspectives(master, slave);
        perspectives.addDockWidget(Qt::BottomDockWidgetArea, logWindow, "Default");
        loader.configure(splash, &perspectives, noload);
        master->setAlternative(slave);
#ifdef Q_OS_LINUX
        master->move(desktop->availableGeometry(0).topLeft());
#endif
        master->showMaximized();

        Notifications::setMainWindow(master);
        if(multiScreen){
            slave->setAlternative(master);
            int screen = 1;
            if(desktop->primaryScreen() == 1){
                screen = 0;
            }
            slave->show();
            slave->move(desktop->availableGeometry(screen).topLeft());
            slave->showMaximized();
        }
        splash->finish(master);
        perspectives.start();
        retVal = a.exec();

        // Free notification reference
        Notifications::setMainWindow(0);

        // Save view for next use
        perspectives.savePerspective();

        // Close project and DLLs
        loader.closeProject();
        loader.deinitializeGuis();

        // Free memory
        if(multiScreen){
            delete slave;
        }
        delete master;
    }
    loader.unload();
    return retVal;
}
