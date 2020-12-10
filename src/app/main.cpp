/*
 * Copyright (C) 2020 Adrian Carpenter
 *
 * This file is part of pingnoo (https://github.com/fizzyade/pingnoo)
 * An open source ping path analyser
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "ComponentSystem/Component.h"
#include "ComponentSystem/ComponentLoader.h"
#include "ComponentSystem/IComponentManager.h"
#include <spdlog/spdlog.h>
#include "SplashScreen.h"

#include <QApplication>
#include <QDir>
#include <QDirIterator>
#include <QJsonDocument>
#include <QLibrary>
#include <QProcessEnvironment>
#include <QStandardPaths>
#include <QString>
#include <QTimer>
#include <vector>
#include <memory>

#if defined(Q_OS_MAC)
#include <CoreFoundation/CoreFoundation.h>
#endif

int main(int argc, char **argv) {
    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);

    qApp->setApplicationName("Pingnoo");
    qApp->setOrganizationName("Nedrysoft");

    auto componentLoader = new Nedrysoft::ComponentSystem::ComponentLoader;
    auto applicationInstance = new QApplication(argc, argv);

    Nedrysoft::SplashScreen *splashScreen = Nedrysoft::SplashScreen::getInstance();;

    splashScreen->show();

    auto componentManager = Nedrysoft::ComponentSystem::IComponentManager::getInstance();

    componentManager->addObject(componentLoader);

    spdlog::set_level(spdlog::level::trace);

    spdlog::debug("Application started.");

    QString sharedLibraryPath;

#ifdef Q_OS_MAC
    CFURLRef appUrlRef = CFBundleCopyBundleURL(CFBundleGetMainBundle());
    CFStringRef macPath = CFURLCopyFileSystemPath(appUrlRef, kCFURLPOSIXPathStyle);
    const char *pathPtr = CFStringGetCStringPtr(macPath, CFStringGetSystemEncoding());
    QString componentPath = QString(pathPtr) + "/Contents/PlugIns";

    sharedLibraryPath = QString(pathPtr) + "/Contents/Frameworks";

    CFRelease(appUrlRef);
    CFRelease(macPath);

    if (componentPath.isNull()) {
        QDir dir = applicationInstance->applicationDirPath();

        dir.cdUp();

        componentPath = dir.absolutePath() + "/PlugIns";
    }

    componentLoader->addComponents(componentPath);

    auto extraLibrarySearchPaths = QStringList() << "Frameworks" << "PlugIns";

    auto dataPaths = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);

    for (auto searchPath : dataPaths) {
        for (auto folderPath : extraLibrarySearchPaths) {
            auto folderName =
                    searchPath + "/" + qApp->organizationName() + "/" + qApp->applicationName() + "/" + folderPath;

            if (QDir(folderName).exists())
                qApp->addLibraryPath(folderName);
        }
    }

    for (auto searchPath : dataPaths) {
        auto folderName = searchPath + "/" + qApp->organizationName() + "/" + qApp->applicationName() + "/PlugIns";

        if (QDir(folderName).exists()) {
            componentLoader->addComponents(folderName);
        }
    }
#else
    if (QProcessEnvironment::systemEnvironment().contains("APPDIR")) {
        componentLoader->addComponents(QProcessEnvironment::systemEnvironment().value("APPDIR")+"/Components");
        sharedLibraryPath =QProcessEnvironment::systemEnvironment().value("APPDIR");
    } else {
        componentLoader->addComponents("Components");
        sharedLibraryPath = QDir::currentPath();
    }
#endif

    // load any shared libraries and see if they have the logging method

    QDirIterator frameworkIterator(sharedLibraryPath);

    QString appSettingsFilename = QStandardPaths::standardLocations(
            QStandardPaths::GenericDataLocation).at(0) + "/" +
            qApp->organizationName() + "/" + qApp->applicationName() + "/appSettings.json";

    QFile settingsFile(appSettingsFilename);
    QVariantList disabledComponents;

    settingsFile.open(QFile::ReadOnly);

    if (settingsFile.isOpen()) {
        auto settings = QJsonDocument::fromJson(settingsFile.readAll()).toVariant();

        if (settings.isValid()) {
            auto settingsMap = settings.toMap();

            if (settingsMap.contains("disabledComponents")) {
                disabledComponents = settingsMap["disabledComponents"].toList();
            }
        }
    }

    componentLoader->loadComponents([disabledComponents](Nedrysoft::ComponentSystem::Component *component) -> bool {
        if (component->canBeDisabled() == false) {
            return true;
        }

        if (disabledComponents.contains(( component->name() + "." + component->vendor()).toLower())) {
            return false;
        }

        return true;
    });

    QTimer::singleShot(3000, [=]() {
        splashScreen->hide();
    });

    auto exitCode = QApplication::exec();

    delete componentLoader;
    delete applicationInstance;

    return exitCode;
}