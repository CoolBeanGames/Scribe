#include <QApplication>
#include <QStringList>
#include <QFileInfo>
#include "MainWindow.h"
#include "Theme.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("Scribe");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("CoolBeanGames");

    // Apply the Zen dark theme stylesheet
    app.setStyleSheet(Theme::APP_STYLESHEET);

    MainWindow window;
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
