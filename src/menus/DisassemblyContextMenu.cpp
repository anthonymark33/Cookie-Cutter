#include "DisassemblyContextMenu.h"
#include "dialogs/preferences/PreferencesDialog.h"
#include "dialogs/EditInstructionDialog.h"
#include "dialogs/CommentsDialog.h"
#include "dialogs/FlagDialog.h"
#include "dialogs/RenameDialog.h"
#include "dialogs/XrefsDialog.h"
#include "dialogs/SetToDataDialog.h"
#include <QtCore>
#include <QShortcut>
#include <QJsonArray>
#include <QClipboard>
#include <QApplication>

DisassemblyContextMenu::DisassemblyContextMenu(QWidget *parent)
    :   QMenu(parent),
        offset(0),
        canCopy(false)
{
    initAction(&actionCopy, tr("Copy"), SLOT(on_actionCopy_triggered()), getCopySequence());
    addAction(&actionCopy);

    copySeparator = addSeparator();

    initAction(&actionCopyAddr, tr("Copy address", SLOT(on_actionCopyAddr_triggered())));
    addAction(&actionCopyAddr);

    initAction(&actionAddComment, tr("Add Comment"),
               SLOT(on_actionAddComment_triggered()), getCommentSequence());
    addAction(&actionAddComment);

    initAction(&actionAddFlag, tr("Add Flag"),
               SLOT(on_actionAddFlag_triggered()), getAddFlagSequence());
    addAction(&actionAddFlag);

    initAction(&actionCreateFunction, tr("Create Function"),
               SLOT(on_actionCreateFunction_triggered()));
    addAction(&actionCreateFunction);

    initAction(&actionRename, tr("Rename"),
               SLOT(on_actionRename_triggered()), getRenameSequence());
    addAction(&actionRename);

    initAction(&actionRenameUsedHere, tr("Rename Flag/Fcn/Var Used Here"),
               SLOT(on_actionRenameUsedHere_triggered()), getRenameUsedHereSequence());
    addAction(&actionRenameUsedHere);

    initAction(&actionDeleteComment, tr("Delete comment"), SLOT(on_actionDeleteComment_triggered()));
    addAction(&actionDeleteComment);

    initAction(&actionDeleteFlag, tr("Delete flag"), SLOT(on_actionDeleteFlag_triggered()));
    addAction(&actionDeleteFlag);

    initAction(&actionDeleteFunction, tr("Undefine function"), SLOT(on_actionDeleteFunction_triggered()));
    addAction(&actionDeleteFunction);

    addSetBaseMenu();

    addSetBitsMenu();

    initAction(&actionSetToCode, tr("Set to Code"),
               SLOT(on_actionSetToCode_triggered()), getSetToCodeSequence());
    addAction(&actionSetToCode);

    addSetToDataMenu();

    addSeparator();

    initAction(&actionXRefs, tr("Show X-Refs"),
               SLOT(on_actionXRefs_triggered()), getXRefSequence());
    addAction(&actionXRefs);

    initAction(&actionDisplayOptions, tr("Show Options"),
               SLOT(on_actionDisplayOptions_triggered()), getDisplayOptionsSequence());

    addSeparator();

    addEditMenu();

    addSeparator();

    addDebugMenu();

    connect(this, &DisassemblyContextMenu::aboutToShow,
            this, &DisassemblyContextMenu::aboutToShowSlot);
}

DisassemblyContextMenu::~DisassemblyContextMenu()
{
    for (QAction *action : anonymousActions) {
        delete action;
    }
}

void DisassemblyContextMenu::addSetBaseMenu()
{
    setBaseMenu = addMenu(tr("Set Immediate Base to..."));

    initAction(&actionSetBaseBinary, tr("Binary"));
    setBaseMenu->addAction(&actionSetBaseBinary);
    connect(&actionSetBaseBinary, &QAction::triggered, this, [this] { setBase("b"); });

    initAction(&actionSetBaseOctal, tr("Octal"));
    setBaseMenu->addAction(&actionSetBaseOctal);
    connect(&actionSetBaseOctal, &QAction::triggered, this, [this] { setBase("o"); });

    initAction(&actionSetBaseDecimal, tr("Decimal"));
    setBaseMenu->addAction(&actionSetBaseDecimal);
    connect(&actionSetBaseDecimal, &QAction::triggered, this, [this] { setBase("d"); });

    initAction(&actionSetBaseHexadecimal, tr("Hexadecimal"));
    setBaseMenu->addAction(&actionSetBaseHexadecimal);
    connect(&actionSetBaseHexadecimal, &QAction::triggered, this, [this] { setBase("h"); });

    initAction(&actionSetBasePort, tr("Network Port"));
    setBaseMenu->addAction(&actionSetBasePort);
    connect(&actionSetBasePort, &QAction::triggered, this, [this] { setBase("p"); });

    initAction(&actionSetBaseIPAddr, tr("IP Address"));
    setBaseMenu->addAction(&actionSetBaseIPAddr);
    connect(&actionSetBaseIPAddr, &QAction::triggered, this, [this] { setBase("i"); });

    initAction(&actionSetBaseSyscall, tr("Syscall"));
    setBaseMenu->addAction(&actionSetBaseSyscall);
    connect(&actionSetBaseSyscall, &QAction::triggered, this, [this] { setBase("S"); });

    initAction(&actionSetBaseString, tr("String"));
    setBaseMenu->addAction(&actionSetBaseString);
    connect(&actionSetBaseString, &QAction::triggered, this, [this] { setBase("s"); });
}

void DisassemblyContextMenu::addSetBitsMenu()
{
    setBitsMenu = addMenu(tr("Set current bits to..."));

    initAction(&actionSetBits16, "16");
    setBitsMenu->addAction(&actionSetBits16);
    connect(&actionSetBits16, &QAction::triggered, this, [this] { setBits(16); });

    initAction(&actionSetBits32, "32");
    setBitsMenu->addAction(&actionSetBits32);
    connect(&actionSetBits32, &QAction::triggered, this, [this] { setBits(32); });

    initAction(&actionSetBits64, "64");
    setBitsMenu->addAction(&actionSetBits64);
    connect(&actionSetBits64, &QAction::triggered, this, [this] { setBits(64); });
}

void DisassemblyContextMenu::addSetToDataMenu()
{
    setToDataMenu = addMenu(tr("Set to Data..."));

    initAction(&actionSetToDataByte, tr("Byte"));
    setToDataMenu->addAction(&actionSetToDataByte);
    connect(&actionSetToDataByte, &QAction::triggered, this, [this] { setToData(1); });

    initAction(&actionSetToDataWord, tr("Word"));
    setToDataMenu->addAction(&actionSetToDataWord);
    connect(&actionSetToDataWord, &QAction::triggered, this, [this] { setToData(2); });

    initAction(&actionSetToDataDword, tr("Dword"));
    setToDataMenu->addAction(&actionSetToDataDword);
    connect(&actionSetToDataDword, &QAction::triggered, this, [this] { setToData(4); });

    initAction(&actionSetToDataQword, tr("Qword"));
    setToDataMenu->addAction(&actionSetToDataQword);
    connect(&actionSetToDataQword, &QAction::triggered, this, [this] { setToData(8); });

    initAction(&actionSetToDataEx, "...",
               SLOT(on_actionSetToDataEx_triggered()), getSetToDataExSequence());
    setToDataMenu->addAction(&actionSetToDataEx);

    auto switchAction = new QAction();
    initAction(switchAction, "Switch Data",
               SLOT(on_actionSetToData_triggered()), getSetToDataSequence());
}

void DisassemblyContextMenu::addEditMenu()
{
    editMenu = addMenu(tr("Edit"));

    initAction(&actionEditInstruction, tr("Instruction"), SLOT(on_actionEditInstruction_triggered()));
    editMenu->addAction(&actionEditInstruction);

    initAction(&actionNopInstruction, tr("Nop Instruction"), SLOT(on_actionNopInstruction_triggered()));
    editMenu->addAction(&actionNopInstruction);

    initAction(&actionEditBytes, tr("Bytes"), SLOT(on_actionEditBytes_triggered()));
    editMenu->addAction(&actionEditBytes);

    initAction(&actionJmpReverse, tr("Reverse Jump"), SLOT(on_actionJmpReverse_triggered()));
    editMenu->addAction(&actionJmpReverse);
}

void DisassemblyContextMenu::addDebugMenu()
{
    debugMenu = addMenu(tr("Debug"));

    initAction(&actionAddBreakpoint, tr("Add/remove breakpoint"),
               SLOT(on_actionAddBreakpoint_triggered()), getAddBPSequence());
    debugMenu->addAction(&actionAddBreakpoint);

    initAction(&actionContinueUntil, tr("Continue until line"),
               SLOT(on_actionContinueUntil_triggered()));
    debugMenu->addAction(&actionContinueUntil);

    initAction(&actionSetPC, "Set PC", SLOT(on_actionSetPC_triggered()));
    debugMenu->addAction(&actionSetPC);
}

void DisassemblyContextMenu::setOffset(RVA offset)
{
    this->offset = offset;
}

void DisassemblyContextMenu::setCanCopy(bool enabled)
{
    this->canCopy = enabled;
}

void DisassemblyContextMenu::aboutToShowSlot()
{
    // check if set immediate base menu makes sense
    QJsonObject instObject = Core()->cmdj("aoj @ " + QString::number(
                                              offset)).array().first().toObject();
    auto keys = instObject.keys();
    bool immBase = keys.contains("val") || keys.contains("ptr");
    setBaseMenu->menuAction()->setVisible(immBase);
    setBitsMenu->menuAction()->setVisible(true);

    actionCreateFunction.setVisible(true);

    QString comment = Core()->cmd("CC." + RAddressString(offset));
    if (comment.isNull() || comment.isEmpty()) {
        actionDeleteComment.setVisible(false);
        actionAddComment.setText(tr("Add Comment"));
    } else {
        actionDeleteComment.setVisible(true);
        actionAddComment.setText(tr("Edit Comment"));
    }

    actionCopy.setVisible(canCopy);
    copySeparator->setVisible(canCopy);


    RCore *core = Core()->core();
    RAnalFunction *fcn = r_anal_get_fcn_at (core->anal, offset, R_ANAL_FCN_TYPE_NULL);
    RFlagItem *f = r_flag_get_i (core->flags, offset);

    actionDeleteFlag.setVisible(f ? true : false);
    actionDeleteFunction.setVisible(fcn ? true : false);

    if (fcn) {
        actionCreateFunction.setVisible(false);
        actionRename.setVisible(true);
        actionRename.setText(tr("Rename function \"%1\"").arg(fcn->name));
    } else if (f) {
        actionRename.setVisible(true);
        actionRename.setText(tr("Rename flag \"%1\"").arg(f->name));
    } else {
        actionRename.setVisible(false);
    }


    // only show "rename X used here" if there is something to rename
    QJsonArray thingUsedHereArray = Core()->cmdj("anj @ " + QString::number(offset)).array();
    if (!thingUsedHereArray.isEmpty()) {
        actionRenameUsedHere.setVisible(true);
        QJsonObject thingUsedHere = thingUsedHereArray.first().toObject();
        if (thingUsedHere["type"] == "address") {
            RVA offset = thingUsedHere["offset"].toVariant().toULongLong();
            actionRenameUsedHere.setText(tr("Add flag at %1 (used here)").arg(RAddressString(offset)));
        } else {
            actionRenameUsedHere.setText(tr("Rename \"%1\" (used here)").arg(thingUsedHere["name"].toString()));
        }
    } else {
        actionRenameUsedHere.setVisible(false);
    }

    // decide to show Reverse jmp option
    showReverseJmpQuery();

    // only show debug options if we are currently debugging
    debugMenu->menuAction()->setVisible(Core()->currentlyDebugging);
    QString progCounterName = Core()->getRegisterName("PC");
    actionSetPC.setText("Set " + progCounterName + " here");

}

QKeySequence DisassemblyContextMenu::getCopySequence() const
{
    return QKeySequence::Copy;
}

QKeySequence DisassemblyContextMenu::getCommentSequence() const
{
    return {Qt::Key_Semicolon};
}

QKeySequence DisassemblyContextMenu::getSetToCodeSequence() const
{
    return {Qt::Key_C};
}

QKeySequence DisassemblyContextMenu::getSetToDataSequence() const
{
    return {Qt::Key_D};
}

QKeySequence DisassemblyContextMenu::getSetToDataExSequence() const
{
    return {Qt::Key_Asterisk};
}

QKeySequence DisassemblyContextMenu::getAddFlagSequence() const
{
    return {}; //TODO insert correct sequence
}

QKeySequence DisassemblyContextMenu::getRenameSequence() const
{
    return {Qt::Key_N};
}

QKeySequence DisassemblyContextMenu::getRenameUsedHereSequence() const
{
    return {Qt::SHIFT + Qt::Key_N};
}

QKeySequence DisassemblyContextMenu::getXRefSequence() const
{
    return {Qt::Key_X};
}

QKeySequence DisassemblyContextMenu::getDisplayOptionsSequence() const
{
    return {}; //TODO insert correct sequence
}

QList<QKeySequence> DisassemblyContextMenu::getAddBPSequence() const
{
    return {Qt::Key_F2, Qt::CTRL + Qt::Key_B};
}

void DisassemblyContextMenu::on_actionEditInstruction_triggered()
{
    EditInstructionDialog *e = new EditInstructionDialog(this);
    e->setWindowTitle(tr("Edit Instruction at %1").arg(RAddressString(offset)));

    QString oldInstruction = Core()->cmdj("aoj").array().first().toObject()["opcode"].toString();
    e->setInstruction(oldInstruction);

    if (e->exec()) {}
    {
        QString instruction = e->getInstruction();
        if (instruction != oldInstruction) {
            Core()->editInstruction(offset, instruction);
        }
    }
}

void DisassemblyContextMenu::on_actionNopInstruction_triggered()
{
    Core()->nopInstruction(offset);
}

void DisassemblyContextMenu::showReverseJmpQuery()
{
    QString type;

    QJsonArray array = Core()->cmdj("pdj 1 @ " + RAddressString(offset)).array();
    if (array.isEmpty()) {
        return;
    }

    type = array.first().toObject()["type"].toString();
    if (type == "cjmp") {
        actionJmpReverse.setVisible(true);
    } else {
        actionJmpReverse.setVisible(false);
    }
}

void DisassemblyContextMenu::on_actionJmpReverse_triggered()
{
    Core()->jmpReverse(offset);
}

void DisassemblyContextMenu::on_actionEditBytes_triggered()
{
    EditInstructionDialog *e = new EditInstructionDialog(this);
    e->setWindowTitle(tr("Edit Bytes at %1").arg(RAddressString(offset)));

    QString oldBytes = Core()->cmdj("aoj").array().first().toObject()["bytes"].toString();
    e->setInstruction(oldBytes);

    if (e->exec()) {}
    {
        QString bytes = e->getInstruction();
        if (bytes != oldBytes) {
            Core()->editBytes(offset, bytes);
        }
    }
}

void DisassemblyContextMenu::on_actionCopy_triggered()
{
    emit copy();
}

void DisassemblyContextMenu::on_actionCopyAddr_triggered()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(RAddressString(offset));
}

void DisassemblyContextMenu::on_actionAddBreakpoint_triggered()
{
    Core()->toggleBreakpoint(offset);
}

void DisassemblyContextMenu::on_actionContinueUntil_triggered()
{
    Core()->continueUntilDebug(RAddressString(offset));
}

void DisassemblyContextMenu::on_actionSetPC_triggered()
{
    QString progCounterName = Core()->getRegisterName("PC");
    Core()->setRegister(progCounterName, RAddressString(offset));
}

void DisassemblyContextMenu::on_actionAddComment_triggered()
{
    QString oldComment = Core()->cmd("CC." + RAddressString(offset));
    // Remove newline at the end added by cmd
    oldComment.remove(oldComment.length() - 1, 1);
    CommentsDialog *c = new CommentsDialog(this);

    if (oldComment.isNull() || oldComment.isEmpty()) {
        c->setWindowTitle(tr("Add Comment at %1").arg(RAddressString(offset)));
    } else {
        c->setWindowTitle(tr("Edit Comment at %1").arg(RAddressString(offset)));
    }

    c->setComment(oldComment);
    if (c->exec()) {
        QString comment = c->getComment();
        if (comment.isEmpty()) {
            Core()->delComment(offset);
        } else {
            Core()->setComment(offset, comment);
        }
    }
}

void DisassemblyContextMenu::on_actionCreateFunction_triggered()
{
    RenameDialog *dialog = new RenameDialog(this);
    dialog->setWindowTitle(tr("Add function at %1").arg(RAddressString(offset)));
    if (dialog->exec()) {
        QString function_name = dialog->getName();
        Core()->createFunctionAt(offset, function_name);
    }
}

void DisassemblyContextMenu::on_actionAddFlag_triggered()
{
    FlagDialog *dialog = new FlagDialog(offset, this->parentWidget());
    dialog->exec();
}

void DisassemblyContextMenu::on_actionRename_triggered()
{
    RCore *core = Core()->core();

    RenameDialog *dialog = new RenameDialog(this);

    RAnalFunction *fcn = r_anal_get_fcn_at (core->anal, offset, R_ANAL_FCN_TYPE_NULL);
    RFlagItem *f = r_flag_get_i (core->flags, offset);
    if (fcn) {
        /* Rename function */
        dialog->setWindowTitle(tr("Rename function %1").arg(fcn->name));
        dialog->setName(fcn->name);
        if (dialog->exec()) {
            QString new_name = dialog->getName();
            Core()->renameFunction(fcn->name, new_name);
        }
    } else if (f) {
        /* Rename current flag */
        dialog->setWindowTitle(tr("Rename flag %1").arg(f->name));
        dialog->setName(f->name);
        if (dialog->exec()) {
            QString new_name = dialog->getName();
            Core()->renameFlag(f->name, new_name);
        }
    } else {
        return;
    }
}

void DisassemblyContextMenu::on_actionRenameUsedHere_triggered()
{
    QJsonArray array = Core()->cmdj("anj @ " + QString::number(offset)).array();
    if (array.isEmpty()) {
        return;
    }

    QJsonObject thingUsedHere = array.first().toObject();
    QString type = thingUsedHere.value("type").toString();

    RenameDialog *dialog = new RenameDialog(this);

    QString oldName;

    if (type == "address") {
        RVA offset = thingUsedHere["offset"].toVariant().toULongLong();
        dialog->setWindowTitle(tr("Add flag at %1").arg(RAddressString(offset)));
        dialog->setName("label." + QString::number(offset, 16));
    } else {
        oldName = thingUsedHere.value("name").toString();
        dialog->setWindowTitle(tr("Rename %1").arg(oldName));
        dialog->setName(oldName);
    }

    if (dialog->exec()) {
        QString newName = dialog->getName().trimmed();
        if (!newName.isEmpty()) {
            Core()->cmd("an " + newName + " @ " + QString::number(offset));

            if (type == "address" || type == "flag") {
                Core()->triggerFlagsChanged();
            } else if (type == "var") {
                Core()->triggerVarsChanged();
            } else if (type == "function") {
                Core()->triggerFunctionRenamed(oldName, newName);
            }
        }
    }
}

void DisassemblyContextMenu::on_actionXRefs_triggered()
{
    XrefsDialog *dialog = new XrefsDialog(this);
    dialog->fillRefsForAddress(offset, RAddressString(offset), false);
    dialog->exec();
}

void DisassemblyContextMenu::on_actionDisplayOptions_triggered()
{
    auto *dialog = new PreferencesDialog(this->window());
    dialog->showSection(PreferencesDialog::Section::Disassembly);
    dialog->show();
}

void DisassemblyContextMenu::on_actionSetToCode_triggered()
{
    Core()->setToCode(offset);
}

void DisassemblyContextMenu::on_actionSetToData_triggered()
{
    int size = Core()->sizeofDataMeta(offset);
    if (size > 8 || (size && (size & (size - 1)))) {
        return;
    }
    if (size == 0 || size == 8) {
        size = 1;
    } else {
        size *= 2;
    }
    setToData(size);
}

void DisassemblyContextMenu::on_actionSetToDataEx_triggered()
{
    auto dialog = new SetToDataDialog(offset, this->window());
    if (!dialog->exec()) {
        return;
    }
    setToData(dialog->getItemSize(), dialog->getItemCount());
}

void DisassemblyContextMenu::on_actionDeleteComment_triggered()
{
    Core()->delComment(offset);
}

void DisassemblyContextMenu::on_actionDeleteFlag_triggered()
{
    Core()->delFlag(offset);
}

void DisassemblyContextMenu::on_actionDeleteFunction_triggered()
{
    Core()->delFunction(offset);
}

void DisassemblyContextMenu::setBase(QString base)
{
    Core()->setImmediateBase(base, offset);
}

void DisassemblyContextMenu::setBits(int bits)
{
    Core()->setCurrentBits(bits, offset);
}

void DisassemblyContextMenu::setToData(int size, int repeat)
{
    Core()->setToData(offset, size, repeat);
}

QAction *DisassemblyContextMenu::addAnonymousAction(QString name, const char *slot,
                                                    QKeySequence keySequence)
{
    auto action = new QAction();
    addAction(action);
    anonymousActions.append(action);
    initAction(action, name, slot, keySequence);
    return action;
}

void DisassemblyContextMenu::initAction(QAction *action, QString name, const char *slot)
{
    action->setParent(this);
    action->setText(name);
    if (slot) {
        connect(action, SIGNAL(triggered(bool)), this, slot);
    }
}

void DisassemblyContextMenu::initAction(QAction *action, QString name,
                                        const char *slot, QKeySequence keySequence)
{
    initAction(action, name, slot);
    if (keySequence.isEmpty()) {
        return;
    }
    action->setShortcut(keySequence);
    auto pWidget = parentWidget();
    auto shortcut = new QShortcut(keySequence, pWidget);
    shortcut->setContext(Qt::WidgetWithChildrenShortcut);
    connect(shortcut, SIGNAL(activated()), this, slot);
}

void DisassemblyContextMenu::initAction(QAction *action, QString name,
                                        const char *slot, QList<QKeySequence> keySequenceList)
{
    initAction(action, name, slot);
    if (keySequenceList.empty()) {
        return;
    }
    action->setShortcuts(keySequenceList);
    auto pWidget = parentWidget();
    for (auto keySequence : keySequenceList) {
        auto shortcut = new QShortcut(keySequence, pWidget);
        shortcut->setContext(Qt::WidgetWithChildrenShortcut);
        connect(shortcut, SIGNAL(activated()), this, slot);
    }
}
