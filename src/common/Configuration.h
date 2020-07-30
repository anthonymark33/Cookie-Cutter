#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <QSettings>
#include <QFont>
#include <core/Cutter.h>

#define Config() (Configuration::instance())
#define ConfigColor(x) Config()->getColor(x)

enum ColorFlags {
    LightFlag = 1,
    DarkFlag = 2
};

struct CutterInterfaceTheme {
    QString name;
    ColorFlags flag;
};

extern const QList<CutterInterfaceTheme> kCutterInterfaceThemesList;

class Configuration : public QObject
{
    Q_OBJECT
private:
    QPalette nativePalette;
    QSettings s;
    static Configuration *mPtr;

    // Colors
    void loadBaseThemeNative();
    void loadBaseThemeDark();
    void loadNativeTheme();
    void loadLightTheme();
    void loadDarkTheme();

    // Asm Options
    void applySavedAsmOptions();

public:
    // Functions
    Configuration();
    static Configuration *instance();

    void loadInitial();

    void resetAll();

    // Auto update
    bool getAutoUpdateEnabled() const;
    void setAutoUpdateEnabled(bool au);

    // Languages
    QLocale getCurrLocale() const;
    void setLocale(const QLocale &l);
    bool setLocaleByName(const QString &language);
    QStringList getAvailableTranslations();

    // Fonts
    const QFont getFont() const;
    void setFont(const QFont &font);

    // Colors
    bool windowColorIsDark();
    void setLastThemeOf(const CutterInterfaceTheme &currInterfaceTheme, const QString &theme);
    QString getLastThemeOf(const CutterInterfaceTheme &currInterfaceTheme) const;
    void setInterfaceTheme(int theme);
    int getInterfaceTheme()
    {
        return s.value("ColorPalette", 0).toInt();
    }

    const CutterInterfaceTheme *getCurrentTheme();

    QString getDirProjects();
    void setDirProjects(const QString &dir);

    QString getRecentFolder();
    void setRecentFolder(const QString &dir);

    void setNewFileLastClicked(int lastClicked);
    int getNewFileLastClicked();

    // Images
    QString getLogoFile();

    // Asm Options
    void resetToDefaultAsmOptions();

    // Graph
    int getGraphBlockMaxChars() const
    {
        return s.value("graph.maxcols", 100).toInt();
    }
    void setGraphBlockMaxChars(int ch)
    {
        s.setValue("graph.maxcols", ch);
    }

    QString getColorTheme() const     { return s.value("theme", "cutter").toString(); }
    void setColorTheme(const QString &theme);

    void setColor(const QString &name, const QColor &color);
    const QColor getColor(const QString &name) const;

    /**
     * @brief Get the value of a config var either from r2 or settings, depending on the key.
     */
    QVariant getConfigVar(const QString &key);
    bool getConfigBool(const QString &key);
    int getConfigInt(const QString &key);
    QString getConfigString(const QString &key);

    /**
     * @brief Set the value of a config var either to r2 or settings, depending on the key.
     */
    void setConfig(const QString &key, const QVariant &value);
    bool isFirstExecution();
    
    /**
     * @brief Get list of available translation directories (depends on configuration and OS)
     * @return list of directories
     */
    QStringList getTranslationsDirectories() const;

signals:
    void fontsUpdated();
    void colorsUpdated();
    void interfaceThemeChanged();
};

#endif // CONFIGURATION_H
