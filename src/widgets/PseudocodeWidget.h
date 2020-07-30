#ifndef PSEUDOCODEWIDGET_H
#define PSEUDOCODEWIDGET_H

#include <memory>

#include "core/Cutter.h"
#include "MemoryDockWidget.h"

namespace Ui {
class PseudocodeWidget;
}

class QTextEdit;
class SyntaxHighlighter;

class PseudocodeWidget : public MemoryDockWidget
{
    Q_OBJECT

public:
    explicit PseudocodeWidget(MainWindow *main, QAction *action = nullptr);
    ~PseudocodeWidget();

private slots:
    void fontsUpdated();
    void colorsUpdatedSlot();
    void refreshPseudocode();

private:
    enum DecompilerComboBoxValues { DecompilerCBR2Dec, DecompilerCBPdc };
    std::unique_ptr<Ui::PseudocodeWidget> ui;

    SyntaxHighlighter *syntaxHighLighter;

    void doRefresh(RVA addr);
    void setupFonts();
};

#endif // PSEUDOCODEWIDGET_H
