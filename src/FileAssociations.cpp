#include "FileAssociations.h"
#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>

#ifdef _WIN32
#include <windows.h>
#include <shlobj.h>
#include <shobjidl.h>
#include <shellapi.h>
#include <shlwapi.h>

namespace {

bool setRegistryValue(HKEY hRoot, const std::wstring& subKey, const std::wstring& valueName, const std::wstring& valueData) {
    HKEY hKey = nullptr;
    LONG res = RegCreateKeyExW(hRoot, subKey.c_str(), 0, nullptr, REG_OPTION_NON_VOLATILE, KEY_WRITE, nullptr, &hKey, nullptr);
    if (res != ERROR_SUCCESS) return false;

    res = RegSetValueExW(hKey, valueName.empty() ? nullptr : valueName.c_str(), 0, REG_SZ,
                         reinterpret_cast<const BYTE*>(valueData.c_str()),
                         static_cast<DWORD>((valueData.size() + 1) * sizeof(wchar_t)));
    RegCloseKey(hKey);
    return res == ERROR_SUCCESS;
}

bool keyExists(HKEY hRoot, const std::wstring& subKey) {
    HKEY hKey = nullptr;
    LONG res = RegOpenKeyExW(hRoot, subKey.c_str(), 0, KEY_READ, &hKey);
    if (res == ERROR_SUCCESS) {
        RegCloseKey(hKey);
        return true;
    }
    return false;
}

bool valueExists(HKEY hRoot, const std::wstring& subKey, const std::wstring& valueName) {
    HKEY hKey = nullptr;
    LONG res = RegOpenKeyExW(hRoot, subKey.c_str(), 0, KEY_READ, &hKey);
    if (res != ERROR_SUCCESS) return false;

    res = RegQueryValueExW(hKey, valueName.c_str(), nullptr, nullptr, nullptr, nullptr);
    RegCloseKey(hKey);
    return res == ERROR_SUCCESS;
}

bool deleteSubKey(HKEY hRoot, const std::wstring& subKey, const std::wstring& childKey) {
    HKEY hKey = nullptr;
    LONG res = RegOpenKeyExW(hRoot, subKey.c_str(), 0, KEY_WRITE | DELETE, &hKey);
    if (res != ERROR_SUCCESS) return false;

    LONG delRes = RegDeleteKeyW(hKey, childKey.c_str());
    RegCloseKey(hKey);
    return delRes == ERROR_SUCCESS;
}

} // namespace
#endif

QList<FileTypeAssociation> FileAssociations::supportedTypes() {
    return {
        // Plain Text
        { ".txt",  "Scribe.txt",  "Text Document",           "Plain Text" },
        { ".md",   "Scribe.md",   "Markdown Document",       "Plain Text" },

        // Rich Text
        { ".stxt", "Scribe.stxt", "Scribe Rich Text Document", "Rich Text" },
        { ".rtf",  "Scribe.rtf",  "Rich Text Document",        "Rich Text" },

        // Spreadsheet
        { ".csv",  "Scribe.csv",  "CSV Spreadsheet",         "Spreadsheet" },

        // Code / Scripting
        { ".py",   "Scribe.py",   "Python Source File",      "Code" },
        { ".cpp",  "Scribe.cpp",  "C++ Source File",         "Code" },
        { ".h",    "Scribe.h",    "C/C++ Header File",       "Code" },
        { ".js",   "Scribe.js",   "JavaScript File",         "Code" },
        { ".cs",   "Scribe.cs",   "C# Source File",          "Code" },
        { ".json", "Scribe.json", "JSON Document",           "Code" },
        { ".html", "Scribe.html", "HTML Document",           "Code" },
        { ".htm",  "Scribe.htm",  "HTML Document",           "Code" },
        { ".css",  "Scribe.css",  "CSS Stylesheet",          "Code" }
    };
}

bool FileAssociations::registerAll(const QString& exePath) {
    QList<QString> allExts;
    for (const auto& t : supportedTypes()) {
        allExts.append(t.extension);
    }
    return registerTypes(allExts, exePath);
}

bool FileAssociations::registerTypes(const QList<QString>& extensions, const QString& exePath) {
#ifdef _WIN32
    QString appPath = exePath.isEmpty() ? QDir::toNativeSeparators(QCoreApplication::applicationFilePath())
                                        : QDir::toNativeSeparators(exePath);
    std::wstring wExePath = appPath.toStdWString();
    std::wstring wOpenCmd = L"\"" + wExePath + L"\" \"%1\"";
    std::wstring wIconPath = L"\"" + wExePath + L"\",0";

    // 1. Register Scribe.exe application in HKCU\Software\Classes\Applications\Scribe.exe
    std::wstring appRegKey = L"Software\\Classes\\Applications\\Scribe.exe";
    setRegistryValue(HKEY_CURRENT_USER, appRegKey, L"FriendlyAppName", L"Scribe");
    setRegistryValue(HKEY_CURRENT_USER, appRegKey + L"\\DefaultIcon", L"", wIconPath);
    setRegistryValue(HKEY_CURRENT_USER, appRegKey + L"\\shell\\open\\command", L"", wOpenCmd);

    // 2. Register capabilities in HKCU\Software\CoolBeanGames\Scribe\Capabilities
    std::wstring capKey = L"Software\\CoolBeanGames\\Scribe\\Capabilities";
    setRegistryValue(HKEY_CURRENT_USER, capKey, L"ApplicationName", L"Scribe");
    setRegistryValue(HKEY_CURRENT_USER, capKey, L"ApplicationDescription", L"Scribe Text, Rich Text, Spreadsheet, and Code Editor");
    setRegistryValue(HKEY_CURRENT_USER, L"Software\\RegisteredApplications", L"Scribe", capKey);

    // 3. Register each requested file type
    auto types = supportedTypes();
    for (const auto& t : types) {
        if (!extensions.contains(t.extension, Qt::CaseInsensitive)) continue;

        std::wstring wExt = t.extension.toStdWString();
        std::wstring wProgId = t.progId.toStdWString();
        std::wstring wName = t.name.toStdWString();

        // ProgID registration
        std::wstring progKey = L"Software\\Classes\\" + wProgId;
        setRegistryValue(HKEY_CURRENT_USER, progKey, L"", wName);
        setRegistryValue(HKEY_CURRENT_USER, progKey, L"FriendlyTypeName", wName);
        setRegistryValue(HKEY_CURRENT_USER, progKey + L"\\DefaultIcon", L"", wIconPath);
        setRegistryValue(HKEY_CURRENT_USER, progKey + L"\\shell\\open\\command", L"", wOpenCmd);

        // Extension registration
        std::wstring extClassKey = L"Software\\Classes\\" + wExt;
        setRegistryValue(HKEY_CURRENT_USER, extClassKey, L"", wProgId);
        setRegistryValue(HKEY_CURRENT_USER, extClassKey + L"\\shell\\open\\command", L"", wOpenCmd);

        // Add to Applications\Scribe.exe\SupportedTypes
        setRegistryValue(HKEY_CURRENT_USER, appRegKey + L"\\SupportedTypes", wExt, L"");

        // Add to OpenWithProgids for extension
        std::wstring extKey = L"Software\\Classes\\" + wExt + L"\\OpenWithProgids";
        setRegistryValue(HKEY_CURRENT_USER, extKey, wProgId, L"");

        // Add to OpenWithList
        std::wstring openWithListKey = L"Software\\Classes\\" + wExt + L"\\OpenWithList\\Scribe.exe";
        setRegistryValue(HKEY_CURRENT_USER, openWithListKey, L"", L"");

        // Add to FileExts
        std::wstring fileExtKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\" + wExt;
        setRegistryValue(HKEY_CURRENT_USER, fileExtKey + L"\\OpenWithProgids", wProgId, L"");
        setRegistryValue(HKEY_CURRENT_USER, fileExtKey + L"\\OpenWithList", L"a", L"Scribe.exe");
        setRegistryValue(HKEY_CURRENT_USER, fileExtKey + L"\\OpenWithList", L"MRUList", L"a");

        // Add to Capabilities\FileAssociations
        setRegistryValue(HKEY_CURRENT_USER, capKey + L"\\FileAssociations", wExt, wProgId);
    }

    // Notify shell that associations have changed
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    return true;
#else
    Q_UNUSED(extensions);
    Q_UNUSED(exePath);
    return false;
#endif
}

bool FileAssociations::setAllAsDefault(const QString& exePath) {
    QList<QString> allExts;
    for (const auto& t : supportedTypes()) {
        allExts.append(t.extension);
    }
    return setAsDefault(allExts, exePath);
}

bool FileAssociations::setAsDefault(const QList<QString>& extensions, const QString& exePath) {
#ifdef _WIN32
    // 1. Ensure full registration first
    if (!registerTypes(extensions, exePath)) return false;

    QString appPath = exePath.isEmpty() ? QDir::toNativeSeparators(QCoreApplication::applicationFilePath())
                                        : QDir::toNativeSeparators(exePath);
    std::wstring wExePath = appPath.toStdWString();
    std::wstring wOpenCmd = L"\"" + wExePath + L"\" \"%1\"";

    CoInitialize(nullptr);
    IApplicationAssociationRegistration* pAAR = nullptr;
    CoCreateInstance(CLSID_ApplicationAssociationRegistration,
                     nullptr, CLSCTX_INPROC_SERVER,
                     IID_IApplicationAssociationRegistration,
                     reinterpret_cast<void**>(&pAAR));

    auto types = supportedTypes();
    for (const auto& t : types) {
        if (!extensions.contains(t.extension, Qt::CaseInsensitive)) continue;

        std::wstring wExt = t.extension.toStdWString();
        std::wstring wProgId = t.progId.toStdWString();

        // 2. Set default handler under HKCU\Software\Classes\<ext>
        std::wstring extClassKey = L"Software\\Classes\\" + wExt;
        setRegistryValue(HKEY_CURRENT_USER, extClassKey, L"", wProgId);
        setRegistryValue(HKEY_CURRENT_USER, extClassKey + L"\\shell\\open\\command", L"", wOpenCmd);

        // 3. Clear old third-party UserChoice if present so Windows falls back to Scribe
        std::wstring fileExtSubKey = L"Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\FileExts\\" + wExt;
        deleteSubKey(HKEY_CURRENT_USER, fileExtSubKey, L"UserChoice");

        // 4. Update FileExts OpenWithList so Scribe is topmost MRU
        setRegistryValue(HKEY_CURRENT_USER, fileExtSubKey + L"\\OpenWithList", L"a", L"Scribe.exe");
        setRegistryValue(HKEY_CURRENT_USER, fileExtSubKey + L"\\OpenWithList", L"MRUList", L"a");
        setRegistryValue(HKEY_CURRENT_USER, fileExtSubKey + L"\\OpenWithProgids", wProgId, L"");

        // 5. Try COM SetAppAsDefault as well
        if (pAAR) {
            pAAR->SetAppAsDefault(L"Scribe", wExt.c_str(), AT_FILEEXTENSION);
        }
    }

    if (pAAR) {
        pAAR->SetAppAsDefaultAll(L"Scribe");
        pAAR->Release();
    }
    CoUninitialize();

    // 6. Notify the Windows shell
    SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, nullptr, nullptr);
    return true;
#else
    Q_UNUSED(extensions);
    Q_UNUSED(exePath);
    return false;
#endif
}

bool FileAssociations::isTypeRegistered(const QString& extension) {
#ifdef _WIN32
    std::wstring wExt = extension.toStdWString();
    std::wstring extKey = L"Software\\Classes\\" + wExt + L"\\OpenWithProgids";
    std::wstring capKey = L"Software\\CoolBeanGames\\Scribe\\Capabilities\\FileAssociations";
    return keyExists(HKEY_CURRENT_USER, extKey) && valueExists(HKEY_CURRENT_USER, capKey, wExt);
#else
    Q_UNUSED(extension);
    return false;
#endif
}

bool FileAssociations::isTypeDefault(const QString& extension) {
#ifdef _WIN32
    std::wstring wExt = extension.toStdWString();

    // 1. Direct query via AssocQueryString
    wchar_t outPath[MAX_PATH] = {0};
    DWORD cch = MAX_PATH;
    if (SUCCEEDED(AssocQueryStringW(ASSOCF_NONE, ASSOCSTR_EXECUTABLE, wExt.c_str(), L"open", outPath, &cch))) {
        QString exe = QString::fromWCharArray(outPath);
        if (exe.contains("Scribe", Qt::CaseInsensitive)) {
            return true;
        }
    }

    // 2. Query via IApplicationAssociationRegistration
    bool isDefault = false;
    CoInitialize(nullptr);
    IApplicationAssociationRegistration* pAAR = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistration,
                                  nullptr, CLSCTX_INPROC_SERVER,
                                  IID_IApplicationAssociationRegistration,
                                  reinterpret_cast<void**>(&pAAR));
    if (SUCCEEDED(hr) && pAAR) {
        LPWSTR currentApp = nullptr;
        hr = pAAR->QueryCurrentDefault(wExt.c_str(), AT_FILEEXTENSION, AL_EFFECTIVE, &currentApp);
        if (SUCCEEDED(hr) && currentApp) {
            QString app = QString::fromWCharArray(currentApp);
            if (app.startsWith("Scribe.", Qt::CaseInsensitive) || app.contains("Scribe", Qt::CaseInsensitive)) {
                isDefault = true;
            }
            CoTaskMemFree(currentApp);
        }
        pAAR->Release();
    }
    CoUninitialize();
    return isDefault;
#else
    Q_UNUSED(extension);
    return false;
#endif
}

bool FileAssociations::launchAdvancedAssociationUI() {
#ifdef _WIN32
    CoInitialize(nullptr);
    IApplicationAssociationRegistrationUI* pAARUI = nullptr;
    HRESULT hr = CoCreateInstance(CLSID_ApplicationAssociationRegistrationUI,
                                  nullptr, CLSCTX_INPROC_SERVER,
                                  IID_IApplicationAssociationRegistrationUI,
                                  reinterpret_cast<void**>(&pAARUI));
    if (SUCCEEDED(hr) && pAARUI) {
        hr = pAARUI->LaunchAdvancedAssociationUI(L"Scribe");
        pAARUI->Release();
        CoUninitialize();
        if (SUCCEEDED(hr)) {
            return true;
        }
    } else {
        CoUninitialize();
    }

    // Fall back to modern Windows Settings
    return launchWindowsDefaultAppsSettings();
#else
    return false;
#endif
}

bool FileAssociations::launchWindowsDefaultAppsSettings() {
#ifdef _WIN32
    HINSTANCE hInst = ShellExecuteW(nullptr, L"open", L"ms-settings:defaultapps", nullptr, nullptr, SW_SHOWNORMAL);
    return (reinterpret_cast<INT_PTR>(hInst) > 32);
#else
    return false;
#endif
}
