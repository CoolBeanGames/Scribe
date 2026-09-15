#include <QApplication>
#include <QStringList>
#include <QFileInfo>
#ifdef _WIN32
#include <windows.h>
#include <shellapi.h>
#include <shobjidl.h>
#endif
#include <QIcon>
#include "MainWindow.h"
#include "Theme.h"
#include "FileAssociations.h"

int main(int argc, char* argv[])
{
#ifdef _WIN32
    SetCurrentProcessExplicitAppUserModelID(L"CoolBeanGames.Scribe.App.1");
#endif

    QApplication app(argc, argv);
    app.setApplicationName("Scribe");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("CoolBeanGames");

    QStringList args = app.arguments();
    if (args.contains("--register") || args.contains("--register-associations")) {
        FileAssociations::registerAll();
        return 0;
    }

#ifdef _WIN32
    // Automatically ensure Scribe capabilities and file associations are registered in Windows
    FileAssociations::registerAll();
#endif

    QIcon appIcon(":/scribe.ico");
    if (appIcon.isNull()) {
        appIcon = QIcon(":/scribe.png");
    }
    app.setWindowIcon(appIcon);

    // Apply the Zen dark theme stylesheet
    app.setStyleSheet(Theme::APP_STYLESHEET);

    MainWindow window;
    window.setWindowIcon(appIcon);
    window.show();

    for (int i = 1; i < args.size(); ++i) {
        QString arg = args[i];
        if (arg == "-" || arg.startsWith("--")) continue;
        if (QFileInfo::exists(arg)) {
            window.openFile(arg);
        }
    }

    return app.exec();
}
