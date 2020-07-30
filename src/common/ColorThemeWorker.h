#ifndef COLORTHEMEWORKER_H
#define COLORTHEMEWORKER_H

#include <QFile>
#include <QColor>
#include <QObject>
#include "Cutter.h"
#include <QJsonObject>

#define ThemeWorker() (ColorThemeWorker::instance())

/**
 * @brief The ColorThemeWorker class is a singletone that provides API for working with
 * color themes.
 */
class ColorThemeWorker : public QObject
{
    Q_OBJECT
public:
    /**
     * @brief radare2SpecificOptions is list of all available radare2-only color options.
     */
    const QStringList radare2SpecificOptions = Core()->cmdj("ecj").object().keys();

    /**
     * @brief cutterSpecificOptions is list of all available Cutter-only color options.
     */
    static const QStringList cutterSpecificOptions;

    /**
     * @brief radare2UnusedOptions is a list of all radare2 options that Cutter does not use.
     */
    static const QStringList radare2UnusedOptions;

    static ColorThemeWorker &instance()
    {
        static ColorThemeWorker ex;
        return ex;
    }

    virtual ~ColorThemeWorker() {}


    /**
     * @brief Copies @a srcThemeName with name @a copyThemeName.
     * @param srcThemeName
     * Name of theme to be copied.
     * @param copyThemeName
     * Name of copy.
     * @return "" on success or error message.
     */
    QString copy(const QString &srcThemeName, const QString &copyThemeName) const;

    /**
     * @brief Saves @a theme as @a themeName theme.
     * @param theme
     * Theme to be saved.
     * @param themeName
     * Name of theme to save.
     * @return "" on success or error message.
     */
    QString save(const QJsonDocument& theme, const QString &themeName) const;

    /**
     * @brief Returns whether or not @a themeName theme is custom (created by user or imported) or not.
     * @param themeName
     * Name of theme to check.
     */
    bool isCustomTheme(const QString &themeName) const;

    /**
     * @brief Returns whether or not @a name theme already exists.
     * @return true if theme exists, false - if not.
     */
    bool isThemeExist(const QString &name) const;

    /**
     * @brief Returns theme as Json where key is option name and value is array of 3 Ints (Red, Green, Blue).
     * @param themeName
     * Theme to get.
     */
    QJsonDocument getTheme(const QString &themeName) const;

    /**
     * @brief Deletes theme named @a themeName.
     * @param themeName
     * Name of theme to be removed.
     * @return "" on success or error message.
     */
    QString deleteTheme(const QString &themeName) const;

    /**
     * @brief Imports theme from @a file.
     * @return "" on success or error message.
     */
    QString importTheme(const QString& file) const;

    /**
     * @brief Renames theme from @a themeName to @a newName.
     * @return "" on success or error message.
     */
    QString renameTheme(const QString& themeName, const QString& newName) const;

    /**
     * @brief Returns whether or not file at @a filePath is a color theme.
     * @param filePath
     * Path to file to check.
     * @param ok
     * Output parameter. Indicates wheter or not check was successful.
     * @return true if given file is color theme and ok == true, otherwise returns false.
     */
    bool isFileTheme(const QString &filePath, bool *ok) const;

    /**
     * @brief Returns list of all custom themes.
     */
    QStringList customThemes() const;

private:
    QString standardR2ThemesLocationPath;
    QString customR2ThemesLocationPath;

    ColorThemeWorker(QObject *parent = nullptr);
    ColorThemeWorker(const ColorThemeWorker &root) = delete;
    ColorThemeWorker &operator=(const ColorThemeWorker &) = delete;

    QColor mergeColors(const QColor &upper, const QColor &lower) const;
};

#endif // COLORTHEMEWORKER_H
