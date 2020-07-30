#ifndef DECOMPILERWIDGET_H
#define DECOMPILERWIDGET_H

#include <QTextEdit>
#include <memory>

#include "core/Cutter.h"
#include "MemoryDockWidget.h"
#include "Decompiler.h"

namespace Ui {
class DecompilerWidget;
}

class QTextEdit;
class QSyntaxHighlighter;
class QTextCursor;
class DecompilerContextMenu;
struct DecompiledCodeTextLine;

class DecompilerWidget : public MemoryDockWidget
{
    Q_OBJECT
protected:
    DecompilerContextMenu *mCtxMenu;

public:
    explicit DecompilerWidget(MainWindow *main);
    ~DecompilerWidget();
public slots:
    void showDisasContextMenu(const QPoint &pt);

    void highlightPC();
private slots:
    void fontsUpdatedSlot();
    void colorsUpdatedSlot();
    void refreshDecompiler();
    void decompilerSelected();
    void cursorPositionChanged();
    void seekChanged();
    void decompilationFinished(RAnnotatedCode *code);

private:
    std::unique_ptr<Ui::DecompilerWidget> ui;

    RefreshDeferrer *refreshDeferrer;

    QSyntaxHighlighter *syntaxHighlighter;
    bool decompilerSelectionEnabled;
    bool autoRefreshEnabled;

    /**
     * True if doRefresh() was called, but the decompiler was still running
     * This means, after the decompiler has finished, it should be refreshed immediately.
     */
    bool decompilerWasBusy;

    RVA decompiledFunctionAddr;
    std::unique_ptr<RAnnotatedCode, void (*)(RAnnotatedCode *)> code;
    bool seekFromCursor = false;

    Decompiler *getCurrentDecompiler();

    void setAutoRefresh(bool enabled);
    void doAutoRefresh();
    void doRefresh(RVA addr = Core()->getOffset());
    void updateRefreshButton();
    void setupFonts();
    void updateSelection();
    void connectCursorPositionChanged(bool disconnect);
    void updateCursorPosition();

    QString getWindowTitle() const override;
    bool eventFilter(QObject *obj, QEvent *event) override;

    /**
     * @brief a wrapper around CutterSeekable::seekToReference to seek to an object which is
     * referenced from the address under cursor
     */
    void seekToReference();

    /**
     * @brief Retrieve the Cursor for a location as close as possible to the given address
     * @param addr - an address in the decompiled function
     * @return a Cursor object for the given address
     */
    QTextCursor getCursorForAddress(RVA addr);

    /**
     * @brief Append a highlighted line to the TextEdit
     * @param extraSelection - an ExtraSelection object colored with the appropriate color
     * @return True on success, otherwise False
     */
    bool colorLine(QTextEdit::ExtraSelection extraSelection);

    /**
     * @brief This function responsible to highlight all the breakpoints in the decompiler view.
     * It will also run when a breakpoint is added, removed or modified.
     */
    void highlightBreakpoints();
    /**
     * @brief Finds the earliest offset and breakpoints within the specified range [startPos, endPos]
     * in the specified RAnnotatedCode
     *
     * This function is supposed to be used for finding the earliest offset and breakpoints within the specified range
     * [startPos, endPos]. This will set the value of the variables 'RVA firstOffsetInLine' and 'QVector<RVA> availableBreakpoints' in
     * this->mCtxMenu.
     *
     * @param codeDecompiled - A reference to the RAnnotatedCode for the function that is decompiled.
     * @param startPos - Position of the start of the range(inclusive).
     * @param endPos - Position of the end of the range(inclusive).
     */
    void gatherBreakpointInfo(RAnnotatedCode &codeDecompiled, size_t startPos, size_t endPos);

    void setInfoForBreakpoints();

    void setAnnotationsAtCursor(size_t pos);
};

#endif // DECOMPILERWIDGET_H
