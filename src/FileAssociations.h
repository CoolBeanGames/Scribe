#pragma once
#include <QString>
#include <QList>

struct FileTypeAssociation {
    QString extension;   // e.g. ".txt"
    QString progId;      // e.g. "Scribe.txt"
    QString name;        // e.g. "Text Document"
    QString category;    // e.g. "Plain Text", "Rich Text", "Spreadsheet", "Code"
};

class FileAssociations {
public:
    static QList<FileTypeAssociation> supportedTypes();
    static bool registerAll(const QString& exePath = QString());
    static bool registerTypes(const QList<QString>& extensions, const QString& exePath = QString());
    static bool setAsDefault(const QList<QString>& extensions, const QString& exePath = QString());
    static bool setAllAsDefault(const QString& exePath = QString());
    static bool isTypeRegistered(const QString& extension);
    static bool isTypeDefault(const QString& extension);
    static bool launchAdvancedAssociationUI();
    static bool launchWindowsDefaultAppsSettings();
};
