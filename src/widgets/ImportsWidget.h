#ifndef IMPORTSWIDGET_H
#define IMPORTSWIDGET_H

#include <memory>

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QRegularExpression>
#include <QStyledItemDelegate>
#include <QTreeWidgetItem>

#include "CutterDockWidget.h"
#include "Cutter.h"

class MainWindow;
class QTreeWidget;

namespace Ui {
class ImportsWidget;
}

class ImportsModel : public QAbstractTableModel
{
    Q_OBJECT

private:
    const QRegularExpression banned = QRegularExpression(QStringLiteral(
                "\\A(\\w\\.)*(system|strcpy|strcpyA|strcpyW|wcscpy|_tcscpy|_mbscpy|StrCpy|StrCpyA|StrCpyW|lstrcpy|lstrcpyA|lstrcpyW"
                "|_tccpy|_mbccpy|_ftcscpy|strcat|strcatA|strcatW|wcscat|_tcscat|_mbscat|StrCat|StrCatA|StrCatW|lstrcat|lstrcatA|"
                "lstrcatW|StrCatBuff|StrCatBuffA|StrCatBuffW|StrCatChainW|_tccat|_mbccat|_ftcscat|sprintfW|sprintfA|wsprintf|wsprintfW|"
                "wsprintfA|sprintf|swprintf|_stprintf|wvsprintf|wvsprintfA|wvsprintfW|vsprintf|_vstprintf|vswprintf|strncpy|wcsncpy|"
                "_tcsncpy|_mbsncpy|_mbsnbcpy|StrCpyN|StrCpyNA|StrCpyNW|StrNCpy|strcpynA|StrNCpyA|StrNCpyW|lstrcpyn|lstrcpynA|lstrcpynW|"
                "strncat|wcsncat|_tcsncat|_mbsncat|_mbsnbcat|StrCatN|StrCatNA|StrCatNW|StrNCat|StrNCatA|StrNCatW|lstrncat|lstrcatnA|"
                "lstrcatnW|lstrcatn|gets|_getts|_gettws|IsBadWritePtr|IsBadHugeWritePtr|IsBadReadPtr|IsBadHugeReadPtr|IsBadCodePtr|"
                "IsBadStringPtr|memcpy|RtlCopyMemory|CopyMemory|wmemcpy|wnsprintf|wnsprintfA|wnsprintfW|_snwprintf|_snprintf|_sntprintf|"
                "_vsnprintf|vsnprintf|_vsnwprintf|_vsntprintf|wvnsprintf|wvnsprintfA|wvnsprintfW|strtok|_tcstok|wcstok|_mbstok|makepath|"
                "_tmakepath| _makepath|_wmakepath|_splitpath|_tsplitpath|_wsplitpath|scanf|wscanf|_tscanf|sscanf|swscanf|_stscanf|snscanf|"
                "snwscanf|_sntscanf|_itoa|_itow|_i64toa|_i64tow|_ui64toa|_ui64tot|_ui64tow|_ultoa|_ultot|_ultow|CharToOem|CharToOemA|CharToOemW|"
                "OemToChar|OemToCharA|OemToCharW|CharToOemBuffA|CharToOemBuffW|alloca|_alloca|strlen|wcslen|_mbslen|_mbstrlen|StrLen|lstrlen|"
                "ChangeWindowMessageFilter)\\z"
    ));
    QList<ImportDescription> *imports;

public:
    enum Column { AddressColumn = 0, TypeColumn, SafetyColumn, NameColumn, ColumnCount };
    enum Role { ImportDescriptionRole = Qt::UserRole, AddressRole };

    ImportsModel(QList<ImportDescription> *imports, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    void beginReload();
    void endReload();
};

class ImportsProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    ImportsProxyModel(ImportsModel *sourceModel, QObject *parent = nullptr);

protected:
    bool filterAcceptsRow(int row, const QModelIndex &parent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
};

class ImportsWidget : public CutterDockWidget
{
    Q_OBJECT

public:
    explicit ImportsWidget(MainWindow *main, QAction *action);
    ~ImportsWidget();

private slots:
    void on_importsTreeView_doubleClicked(const QModelIndex &index);

    void refreshImports();

private:
    std::unique_ptr<Ui::ImportsWidget> ui;
    ImportsModel *importsModel;
    ImportsProxyModel *importsProxyModel;
    QList<ImportDescription> imports;

    void highlightUnsafe();
    void setScrollMode();
};

#endif // IMPORTSWIDGET_H
