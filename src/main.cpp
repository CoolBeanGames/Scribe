#include <QApplication>
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

    return app.exec();
}
