#include "DisassemblyContextMenu.h"
#include "dialogs/preferences/PreferencesDialog.h"
#include "dialogs/EditInstructionDialog.h"
#include "dialogs/CommentsDialog.h"
#include "dialogs/FlagDialog.h"
#include "dialogs/RenameDialog.h"
#include "dialogs/XrefsDialog.h"
#include "dialogs/EditVariablesDialog.h"
#include "dialogs/SetToDataDialog.h"
#include "dialogs/EditFunctionDialog.h"
#include "dialogs/LinkTypeDialog.h"

#include <QtCore>
#include <QShortcut>
#include <QJsonArray>
#include <QClipboard>
#include <QApplication>
#include <QPushButton>

DisassemblyContextMenu::DisassemblyContextMenu(QWidget *parent)
    :   QMenu(parent),
        offset(0),
        canCopy(false)
{
    initAction(&actionCopy, tr("Copy"), SLOT(on_actionCopy_triggered()), getCopySequence());
    addAction(&actionCopy);

    initAction(&actionCopyAddr, tr("Copy address"), SLOT(on_actionCopyAddr_triggered()), getCopyAddressSequence());
    addAction(&actionCopyAddr);

    copySeparator = addSeparator();

    initAction(&actionAddComment, tr("Add Comment"),
               SLOT(on_actionAddComment_triggered()), getCommentSequence());
    addAction(&actionAddComment);

    initAction(&actionAddFlag, tr("Add Flag"),
               SLOT(on_actionAddFlag_triggered()), getAddFlagSequence());
    addAction(&actionAddFlag);

    initAction(&actionRename, tr("Rename"),
               SLOT(on_actionRename_triggered()), getRenameSequence());
    addAction(&actionRename);

    initAction(&actionEditFunction, tr("Edit function"),
               SLOT(on_actionEditFunction_triggered()));
    addAction(&actionEditFunction);

    initAction(&actionRenameUsedHere, tr("Rename Flag/Fcn/Var Used Here"),
               SLOT(on_actionRenameUsedHere_triggered()), getRenameUsedHereSequence());
    addAction(&actionRenameUsedHere);

    initAction(&actionSetFunctionVarTypes, tr("Re-type function local vars"),
               SLOT(on_actionSetFunctionVarTypes_triggered()), getRetypeSequence());
    addAction(&actionSetFunctionVarTypes);

    initAction(&actionDeleteComment, tr("Delete comment"), SLOT(on_actionDeleteComment_triggered()));
    addAction(&actionDeleteComment);

    initAction(&actionDeleteFlag, tr("Delete flag"), SLOT(on_actionDeleteFlag_triggered()));
    addAction(&actionDeleteFlag);

    initAction(&actionDeleteFunction, tr("Undefine function"),
               SLOT(on_actionDeleteFunction_triggered()));
    addAction(&actionDeleteFunction);

    initAction(&actionAnalyzeFunction, tr("Define function here..."),
               SLOT(on_actionAnalyzeFunction_triggered()));
    addAction(&actionAnalyzeFunction);

    addSeparator();

    addSetBaseMenu();

    addSetBitsMenu();

    structureOffsetMenu = addMenu(tr("Structure offset"));
    connect(structureOffsetMenu, SIGNAL(triggered(QAction*)),
            this, SLOT(on_actionStructureOffsetMenu_triggered(QAction*)));

    initAction(&actionLinkType, tr("Link Type to Address"),
               SLOT(on_actionLinkType_triggered()), getLinkTypeSequence());
    addAction(&actionLinkType);

    initAction(&actionSetToCode, tr("Set as Code"),
               SLOT(on_actionSetToCode_triggered()), getSetToCodeSequence());
    addAction(&actionSetToCode);

    initAction(&actionSetAsString, tr("Set as String"),
               SLOT(on_actionSetAsString_triggered()), getSetAsStringSequence());
    addAction(&actionSetAsString);

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

void DisassemblyContextMenu::setCurHighlightedWord(const QString &text)
{
    this->curHighlightedWord = text;
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

    // Create structure offset menu if it makes sense
    QString memBaseReg; // Base register
    QVariant memDisp; // Displacement
    if (instObject.contains("opex") && instObject["opex"].toObject().contains("operands")) {
        // Loop through both the operands of the instruction
        for (const QJsonValue value: instObject["opex"].toObject()["operands"].toArray()) {
            QJsonObject operand = value.toObject();
            if (operand.contains("type") && operand["type"].toString() == "mem" &&
                    operand.contains("base") && !operand["base"].toString().contains("bp") &&
                    operand.contains("disp") && operand["disp"].toVariant().toLongLong() > 0) {

                    // The current operand is the one which has an immediate displacement
                    memBaseReg = operand["base"].toString();
                    memDisp = operand["disp"].toVariant();
                    break;

            }
        }
    }
    if (memBaseReg.isEmpty()) {
        // hide structure offset menu
        structureOffsetMenu->menuAction()->setVisible(false);
    } else {
        // show structure offset menu
        structureOffsetMenu->menuAction()->setVisible(true);
        structureOffsetMenu->clear();

        // Get the possible offsets using the "tas" command
        // TODO: add tasj command to radare2 and then use it here
        QStringList ret = Core()->cmdList("tas " + memDisp.toString());
        for (const QString &val : ret) {
            if (val.isEmpty()) {
                continue;
            }
            structureOffsetMenu->addAction("[" + memBaseReg + " + " + val + "]")->setData(val);
        }
        if (structureOffsetMenu->isEmpty()) {
            // No possible offset was found so hide the menu
            structureOffsetMenu->menuAction()->setVisible(false);
        }
    }

    actionAnalyzeFunction.setVisible(true);

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
    RAnalFunction *in_fcn = Core()->functionAt(offset);
    RFlagItem *f = r_flag_get_i (core->flags, offset);

    actionDeleteFlag.setVisible(f ? true : false);
    actionDeleteFunction.setVisible(fcn ? true : false);

    if (fcn) {
        actionAnalyzeFunction.setVisible(false);
        actionRename.setVisible(true);
        actionRename.setText(tr("Rename function \"%1\"").arg(fcn->name));
    } else if (f) {
        actionRename.setVisible(true);
        actionRename.setText(tr("Rename flag \"%1\"").arg(f->name));
    } else {
        actionRename.setVisible(false);
    }

    // Only show retype for local vars if in a function
    if (in_fcn) {
        actionSetFunctionVarTypes.setVisible(true);
        actionEditFunction.setVisible(true);
        actionEditFunction.setText(tr("Edit function \"%1\"").arg(in_fcn->name));
    } else {
        actionSetFunctionVarTypes.setVisible(false);
        actionEditFunction.setVisible(false);
    }


    // Only show "rename X used here" if there is something to rename
    QJsonArray thingUsedHereArray = Core()->cmdj("anj @ " + QString::number(offset)).array();
    if (!thingUsedHereArray.isEmpty()) {
        actionRenameUsedHere.setVisible(true);
        QJsonObject thingUsedHere = thingUsedHereArray.first().toObject();
        if (thingUsedHere["type"] == "address") {
            RVA offset = thingUsedHere["offset"].toVariant().toULongLong();
            actionRenameUsedHere.setText(tr("Add flag at %1 (used here)").arg(RAddressString(offset)));
        } else {
            if (thingUsedHere["type"] == "function") {
                actionRenameUsedHere.setText(tr("Rename \"%1\"").arg(thingUsedHere["name"].toString()));
            }
            else {
                actionRenameUsedHere.setText(tr("Rename \"%1\" (used here)").arg(thingUsedHere["name"].toString()));
            }
        }
    } else {
        actionRenameUsedHere.setVisible(false);
    }

    // Decide to show Reverse jmp option
    showReverseJmpQuery();

    // Only show debug options if we are currently debugging
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

QKeySequence DisassemblyContextMenu::getCopyAddressSequence() const
{
    return {Qt::CTRL + Qt::SHIFT + Qt::Key_C};
}

QKeySequence DisassemblyContextMenu::getSetToCodeSequence() const
{
    return {Qt::Key_C};
}

QKeySequence DisassemblyContextMenu::getSetAsStringSequence() const
{
    return {Qt::Key_A};
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

QKeySequence DisassemblyContextMenu::getRetypeSequence() const
{
    return {Qt::Key_Y};
}

QKeySequence DisassemblyContextMenu::getXRefSequence() const
{
    return {Qt::Key_X};
}

QKeySequence DisassemblyContextMenu::getDisplayOptionsSequence() const
{
    return {}; //TODO insert correct sequence
}

QKeySequence DisassemblyContextMenu::getLinkTypeSequence() const
{
    return {Qt::Key_L};
}

QList<QKeySequence> DisassemblyContextMenu::getAddBPSequence() const
{
    return {Qt::Key_F2, Qt::CTRL + Qt::Key_B};
}

void DisassemblyContextMenu::on_actionEditInstruction_triggered()
{
    EditInstructionDialog e(EDIT_TEXT, this);
    e.setWindowTitle(tr("Edit Instruction at %1").arg(RAddressString(offset)));

    QString oldInstructionOpcode = Core()->getInstructionOpcode(offset);
    QString oldInstructionBytes = Core()->getInstructionBytes(offset);

    e.setInstruction(oldInstructionOpcode);

    if (e.exec()) {
        QString userInstructionOpcode = e.getInstruction();
        if (userInstructionOpcode != oldInstructionOpcode) {
            Core()->editInstruction(offset, userInstructionOpcode);

            // Check if the write failed
            auto newInstructionBytes = Core()->getInstructionBytes(offset);
            if (newInstructionBytes == oldInstructionBytes) {
                if (!writeFailed()) {
                    Core()->editInstruction(offset, userInstructionOpcode);
                }
            }
        }
    }
}

void DisassemblyContextMenu::on_actionNopInstruction_triggered()
{
    QString oldBytes = Core()->getInstructionBytes(offset);

    Core()->nopInstruction(offset);

    QString newBytes = Core()->getInstructionBytes(offset);
    if (oldBytes == newBytes) {
        if (!writeFailed()) {
            Core()->nopInstruction(offset);
        }
    }
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
    QString oldBytes = Core()->getInstructionBytes(offset);

    Core()->jmpReverse(offset);

    QString newBytes = Core()->getInstructionBytes(offset);
    if (oldBytes == newBytes) {
        if (!writeFailed()) {
            Core()->jmpReverse(offset);
        }
    }
}

void DisassemblyContextMenu::on_actionEditBytes_triggered()
{
    EditInstructionDialog e(EDIT_BYTES, this);
    e.setWindowTitle(tr("Edit Bytes at %1").arg(RAddressString(offset)));

    QString oldBytes = Core()->getInstructionBytes(offset);
    e.setInstruction(oldBytes);

    if (e.exec()) {
        QString bytes = e.getInstruction();
        if (bytes != oldBytes) {
            Core()->editBytes(offset, bytes);

            QString newBytes = Core()->getInstructionBytes(offset);
            if (oldBytes == newBytes) {
                if (!writeFailed()) {
                    Core()->editBytes(offset, bytes);
                }
            }
        }
    }
}

bool DisassemblyContextMenu::writeFailed()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Icon::Critical);
    msgBox.setWindowTitle(tr("Write error"));
    msgBox.setText(
        tr("Unable to complete write operation. Consider opening in write mode. \n\nWARNING: In write mode any changes will be commited to disk"));
    msgBox.addButton(tr("OK"), QMessageBox::NoRole);
    QAbstractButton *reopenButton = msgBox.addButton(tr("Reopen in write mode and try again"),
                                                     QMessageBox::YesRole);

    msgBox.exec();

    if (msgBox.clickedButton() == reopenButton) {
        Core()->cmd("oo+");
        return false;
    }

    return true;
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
    CommentsDialog c(this);

    if (oldComment.isNull() || oldComment.isEmpty()) {
        c.setWindowTitle(tr("Add Comment at %1").arg(RAddressString(offset)));
    } else {
        c.setWindowTitle(tr("Edit Comment at %1").arg(RAddressString(offset)));
    }

    c.setComment(oldComment);
    if (c.exec()) {
        QString comment = c.getComment();
        if (comment.isEmpty()) {
            Core()->delComment(offset);
        } else {
            Core()->setComment(offset, comment);
        }
    }
}

void DisassemblyContextMenu::on_actionAnalyzeFunction_triggered()
{
    RenameDialog dialog(this);
    dialog.setWindowTitle(tr("Analyze function at %1").arg(RAddressString(offset)));
    dialog.setPlaceholderText(tr("Function name"));
    if (dialog.exec()) {
        QString function_name = dialog.getName();
        Core()->createFunctionAt(offset, function_name);
    }
}

void DisassemblyContextMenu::on_actionAddFlag_triggered()
{
    FlagDialog dialog(offset, this->parentWidget());
    dialog.exec();
}

void DisassemblyContextMenu::on_actionRename_triggered()
{
    RCore *core = Core()->core();

    RenameDialog dialog(this);

    RAnalFunction *fcn = r_anal_get_fcn_at (core->anal, offset, R_ANAL_FCN_TYPE_NULL);
    RFlagItem *f = r_flag_get_i (core->flags, offset);
    if (fcn) {
        /* Rename function */
        dialog.setWindowTitle(tr("Rename function %1").arg(fcn->name));
        dialog.setName(fcn->name);
        if (dialog.exec()) {
            QString new_name = dialog.getName();
            Core()->renameFunction(fcn->name, new_name);
        }
    } else if (f) {
        /* Rename current flag */
        dialog.setWindowTitle(tr("Rename flag %1").arg(f->name));
        dialog.setName(f->name);
        if (dialog.exec()) {
            QString new_name = dialog.getName();
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

    RenameDialog dialog(this);

    QString oldName;

    if (type == "address") {
        RVA offset = thingUsedHere["offset"].toVariant().toULongLong();
        dialog.setWindowTitle(tr("Add flag at %1").arg(RAddressString(offset)));
        dialog.setName("label." + QString::number(offset, 16));
    } else {
        oldName = thingUsedHere.value("name").toString();
        dialog.setWindowTitle(tr("Rename %1").arg(oldName));
        dialog.setName(oldName);
    }

    if (dialog.exec()) {
        QString newName = dialog.getName().trimmed();
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

void DisassemblyContextMenu::on_actionSetFunctionVarTypes_triggered()
{
    RAnalFunction *fcn = Core()->functionAt(offset);

    if (!fcn) {
        QMessageBox::critical(this, tr("Re-type function local vars"), tr("You must be in a function to define variable types."));
        return;
    }

    EditVariablesDialog dialog(Core()->getOffset(), this);
    dialog.exec();
}

void DisassemblyContextMenu::on_actionXRefs_triggered()
{
    XrefsDialog dialog(nullptr);
    dialog.fillRefsForAddress(offset, RAddressString(offset), false);
    dialog.exec();
}

void DisassemblyContextMenu::on_actionDisplayOptions_triggered()
{
    PreferencesDialog dialog(this->window());
    dialog.showSection(PreferencesDialog::Section::Disassembly);
    dialog.exec();
}

void DisassemblyContextMenu::on_actionSetToCode_triggered()
{
    Core()->setToCode(offset);
}

void DisassemblyContextMenu::on_actionSetAsString_triggered()
{
    Core()->setAsString(offset);
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
    SetToDataDialog dialog(offset, this->window());
    if (!dialog.exec()) {
        return;
    }
    setToData(dialog.getItemSize(), dialog.getItemCount());
}

void DisassemblyContextMenu::on_actionStructureOffsetMenu_triggered(QAction *action)
{
    Core()->applyStructureOffset(action->data().toString(), offset);
}

void DisassemblyContextMenu::on_actionLinkType_triggered()
{
    LinkTypeDialog dialog(this);
    if (!dialog.setDefaultAddress(curHighlightedWord)) {
        dialog.setDefaultAddress(RAddressString(offset));
    }
    dialog.exec();
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

void DisassemblyContextMenu::on_actionEditFunction_triggered()
{
    RCore *core = Core()->core();
    EditFunctionDialog dialog(this);
    RAnalFunction *fcn = r_anal_get_fcn_in(core->anal, offset, 0);

    if (fcn) {
        dialog.setWindowTitle(tr("Edit function %1").arg(fcn->name));
        dialog.setNameText(fcn->name);

        QString startAddrText = "0x" + QString::number(fcn->addr, 16);
        dialog.setStartAddrText(startAddrText);

        QString endAddrText = "0x" + QString::number(fcn->addr + fcn->_size, 16);
        dialog.setEndAddrText(endAddrText);

        QString stackSizeText;
        stackSizeText.sprintf("%d", fcn->stack);
        dialog.setStackSizeText(stackSizeText);

        QStringList callConList = Core()->cmd("afcl").split("\n");
        callConList.removeLast();
        dialog.setCallConList(callConList);
        dialog.setCallConSelected(fcn->cc);


        if (dialog.exec()) {
            QString new_name = dialog.getNameText();
            Core()->renameFunction(fcn->name, new_name);
            QString new_start_addr = dialog.getStartAddrText();
            fcn->addr = Core()->math(new_start_addr);
            QString new_end_addr = dialog.getEndAddrText();
            Core()->cmd("afu " + new_end_addr);
            QString new_stack_size = dialog.getStackSizeText();
            fcn->stack = int(Core()->math(new_stack_size));
            Core()->cmd("afc " + dialog.getCallConSelected());
            emit Core()->functionsChanged();
        }
    }
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
    parentWidget()->addAction(action);
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
    action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
}

void DisassemblyContextMenu::initAction(QAction *action, QString name,
                                        const char *slot, QList<QKeySequence> keySequenceList)
{
    initAction(action, name, slot);
    if (keySequenceList.empty()) {
        return;
    }
    action->setShortcuts(keySequenceList);
    action->setShortcutContext(Qt::WidgetWithChildrenShortcut);
}
