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

int main(int argc, char* argv[])
{
#ifdef _WIN32
    SetCurrentProcessExplicitAppUserModelID(L"CoolBeanGames.Scribe.App.1");
#endif

    QApplication app(argc, argv);
    app.setApplicationName("Scribe");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("CoolBeanGames");

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

    QStringList args = app.arguments();
    for (int i = 1; i < args.size(); ++i) {
        QString arg = args[i];
        if (arg == "-") continue;
        if (QFileInfo::exists(arg)) {
            window.openFile(arg);
        }
    }

    return app.exec();
}
