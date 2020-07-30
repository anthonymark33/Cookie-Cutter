#ifndef CUTTER_H
#define CUTTER_H

#include "core/CutterCommon.h"
#include "core/CutterDescriptions.h"
#include "common/BasicInstructionHighlighter.h"

#include <QMap>
#include <QDebug>
#include <QObject>
#include <QStringList>
#include <QMessageBox>
#include <QJsonDocument>
#include <QErrorMessage>
#include <QMutex>

class AsyncTaskManager;
class BasicInstructionHighlighter;
class CutterCore;
class Decompiler;
class R2Task;
class R2TaskDialog;

#include "plugins/CutterPlugin.h"
#include "common/BasicBlockHighlighter.h"
#include "common/R2Task.h"
#include "dialogs/R2TaskDialog.h"

#define Core() (CutterCore::instance())

class RCoreLocked;

class CutterCore: public QObject
{
    Q_OBJECT

    friend class RCoreLocked;
    friend class R2Task;

public:
    explicit CutterCore(QObject *parent = nullptr);
    ~CutterCore();
    static CutterCore *instance();

    void initialize();
    void loadCutterRC();

    AsyncTaskManager *getAsyncTaskManager() { return asyncTaskManager; }

    RVA getOffset() const                   { return core_->offset; }

    /* Core functions (commands) */
    static QString sanitizeStringForCommand(QString s);
    /**
     * @brief send a command to radare2
     * @param str the command you want to execute
     * @return command output
     * @note if you want to seek to an address, you should use CutterCore::seek.
     */
    QString cmd(const char *str);
    QString cmd(const QString &str) { return cmd(str.toUtf8().constData()); }
    /**
     * @brief send a command to radare2 asynchronously
     * @param str the command you want to execute
     * @param task a shared pointer that will be returned with the R2 command task
     * @note connect to the &R2Task::finished signal to add your own logic once
     *       the command is finished. Use task->getResult()/getResultJson() for the 
     *       return value.
     *       Once you have setup connections you can start the task with task->startTask()
     *       If you want to seek to an address, you should use CutterCore::seek.
     */
    bool asyncCmd(const char *str, QSharedPointer<R2Task> &task);
    bool asyncCmd(const QString &str, QSharedPointer<R2Task> &task) { return asyncCmd(str.toUtf8().constData(), task); }
    QString cmdRaw(const QString &str);
    QJsonDocument cmdj(const char *str);
    QJsonDocument cmdj(const QString &str) { return cmdj(str.toUtf8().constData()); }
    QStringList cmdList(const char *str) { return cmd(str).split(QLatin1Char('\n'), QString::SkipEmptyParts); }
    QStringList cmdList(const QString &str) { return cmdList(str.toUtf8().constData()); }
    QString cmdTask(const QString &str);
    QJsonDocument cmdjTask(const QString &str);
    /**
     * @brief send a command to radare2 and check for ESIL errors
     * @param command the command you want to execute
     * @note If you want to seek to an address, you should use CutterCore::seek.
     */
    void cmdEsil(const char *command);
    void cmdEsil(const QString &command) { cmdEsil(command.toUtf8().constData()); }
    /**
     * @brief send a command to radare2 and check for ESIL errors
     * @param command the command you want to execute
     * @param task a shared pointer that will be returned with the R2 command task
     * @note connect to the &R2Task::finished signal to add your own logic once
     *       the command is finished. Use task->getResult()/getResultJson() for the 
     *       return value.
     *       Once you have setup connections you can start the task with task->startTask()
     *       If you want to seek to an address, you should use CutterCore::seek.
     */
    bool asyncCmdEsil(const char *command, QSharedPointer<R2Task> &task);
    bool asyncCmdEsil(const QString &command, QSharedPointer<R2Task> &task) { return asyncCmdEsil(command.toUtf8().constData(), task); }
    QString getVersionInformation();

    QJsonDocument parseJson(const char *res, const char *cmd = nullptr);
    QJsonDocument parseJson(const char *res, const QString &cmd = QString())
    {
        return parseJson(res, cmd.isNull() ? nullptr : cmd.toLocal8Bit().constData());
    }

    QStringList autocomplete(const QString &cmd, RLinePromptType promptType, size_t limit = 4096);

    /* Functions methods */
    void renameFunction(const QString &oldName, const QString &newName);
    void delFunction(RVA addr);
    void renameFlag(QString old_name, QString new_name);
    RAnalFunction *functionAt(ut64 addr);
    RVA getFunctionStart(RVA addr);
    RVA getFunctionEnd(RVA addr);
    RVA getLastFunctionInstruction(RVA addr);
    QString cmdFunctionAt(QString addr);
    QString cmdFunctionAt(RVA addr);
    QString createFunctionAt(RVA addr);
    QString createFunctionAt(RVA addr, QString name);
    QStringList getDisassemblyPreview(RVA address, int num_of_lines);

    /* Flags */
    void delFlag(RVA addr);
    void delFlag(const QString &name);
    void addFlag(RVA offset, QString name, RVA size);
    /**
     * @brief Get nearest flag at or before offset.
     * @param offset search position
     * @param flagOffsetOut adress of returned flag
     * @return flag name
     */
    QString nearestFlag(RVA offset, RVA *flagOffsetOut);
    void triggerFlagsChanged();

    /* Edition functions */
    QString getInstructionBytes(RVA addr);
    QString getInstructionOpcode(RVA addr);
    void editInstruction(RVA addr, const QString &inst);
    void nopInstruction(RVA addr);
    void jmpReverse(RVA addr);
    void editBytes(RVA addr, const QString &inst);
    void editBytesEndian(RVA addr, const QString &bytes);

    /* Code/Data */
    void setToCode(RVA addr);
    void setAsString(RVA addr);
    void setToData(RVA addr, int size, int repeat = 1);
    int sizeofDataMeta(RVA addr);

    /* Comments */
    void setComment(RVA addr, const QString &cmt);
    void delComment(RVA addr);
    void setImmediateBase(const QString &r2BaseName, RVA offset = RVA_INVALID);
    void setCurrentBits(int bits, RVA offset = RVA_INVALID);

    /**
     * @brief Changes immediate displacement to structure offset
     * This function makes use of the "aht" command of r2 to apply structure
     * offset to the immediate displacement used in the given instruction
     * \param structureOffset The name of struct which will be applied
     * \param offset The address of the instruction where the struct will be applied
     */
    void applyStructureOffset(const QString &structureOffset, RVA offset = RVA_INVALID);

    /* Classes */
    QList<QString> getAllAnalClasses(bool sorted);
    QList<AnalMethodDescription> getAnalClassMethods(const QString &cls);
    QList<AnalBaseClassDescription> getAnalClassBaseClasses(const QString &cls);
    QList<AnalVTableDescription> getAnalClassVTables(const QString &cls);
    void createNewClass(const QString &cls);
    void renameClass(const QString &oldName, const QString &newName);
    void deleteClass(const QString &cls);
    bool getAnalMethod(const QString &cls, const QString &meth, AnalMethodDescription *desc);
    void renameAnalMethod(const QString &className, const QString &oldMethodName, const QString &newMethodName);
    void setAnalMethod(const QString &cls, const AnalMethodDescription &meth);

    /* File related methods */
    bool loadFile(QString path, ut64 baddr = 0LL, ut64 mapaddr = 0LL, int perms = R_PERM_R,
                  int va = 0, bool loadbin = false, const QString &forceBinPlugin = QString());
    bool tryFile(QString path, bool rw);
    bool openFile(QString path, RVA mapaddr);
    void loadScript(const QString &scriptname);
    QJsonArray getOpenedFiles();

    /* Seek functions */
    void seek(QString thing);
    void seek(ut64 offset);
    void seekPrev();
    void seekNext();
    void updateSeek();
    /**
     * @brief Raise a memory widget showing current offset, prefer last active
     * memory widget.
     */
    void showMemoryWidget();
    /**
     * @brief Seek to \p offset and raise a memory widget showing it.
     * @param offset
     */
    void seekAndShow(ut64 offset);
    /**
     * @brief \see CutterCore::show(ut64)
     * @param thing - addressable expression
     */
    void seekAndShow(QString thing);
    RVA getOffset();
    RVA prevOpAddr(RVA startAddr, int count);
    RVA nextOpAddr(RVA startAddr, int count);

    /* Math functions */
    ut64 math(const QString &expr);
    ut64 num(const QString &expr);
    QString itoa(ut64 num, int rdx = 16);

    /* Config functions */
    void setConfig(const char *k, const QString &v);
    void setConfig(const QString &k, const QString &v) { setConfig(k.toUtf8().constData(), v); }
    void setConfig(const char *k, int v);
    void setConfig(const QString &k, int v) { setConfig(k.toUtf8().constData(), v); }
    void setConfig(const char *k, bool v);
    void setConfig(const QString &k, bool v) { setConfig(k.toUtf8().constData(), v); }
    void setConfig(const char *k, const QVariant &v);
    void setConfig(const QString &k, const QVariant &v) { setConfig(k.toUtf8().constData(), v); }
    int getConfigi(const char *k);
    int getConfigi(const QString &k) { return getConfigi(k.toUtf8().constData()); }
    ut64 getConfigut64(const char *k);
    ut64 getConfigut64(const QString &k) { return getConfigut64(k.toUtf8().constData()); }
    bool getConfigb(const char *k);
    bool getConfigb(const QString &k) { return getConfigb(k.toUtf8().constData()); }
    QString getConfig(const char *k);
    QString getConfig(const QString &k) { return getConfig(k.toUtf8().constData()); }
    QList<QString> getColorThemes();

    /* Assembly\Hexdump related methods */
    QByteArray assemble(const QString &code);
    QString disassemble(const QByteArray &data);
    QString disassembleSingleInstruction(RVA addr);
    QList<DisassemblyLine> disassembleLines(RVA offset, int lines);

    static QByteArray hexStringToBytes(const QString &hex);
    static QString bytesToHexString(const QByteArray &bytes);
    enum class HexdumpFormats { Normal, Half, Word, Quad, Signed, Octal };
    QString hexdump(RVA offset, int size, HexdumpFormats format);
    QString getHexdumpPreview(RVA offset, int size);

    void setCPU(QString arch, QString cpu, int bits);
    void setEndianness(bool big);

    /* SDB */
    QList<QString> sdbList(QString path);
    QList<QString> sdbListKeys(QString path);
    QString sdbGet(QString path, QString key);
    bool sdbSet(QString path, QString key, QString val);

    /* Debug */
    QJsonDocument getRegistersInfo();
    QJsonDocument getRegisterValues();
    QString getRegisterName(QString registerRole);
    RVA getProgramCounterValue();
    void setRegister(QString regName, QString regValue);
    void setCurrentDebugThread(int tid);
    /**
     * @brief Attach to a given pid from a debug session
     */
    void setCurrentDebugProcess(int pid);
    QJsonDocument getStack(int size = 0x100);
    /**
     * @brief Get a list of a given process's threads
     * @param pid The pid of the process, -1 for the currently debugged process
     * @return JSON object result of dptj
     */
    QJsonDocument getProcessThreads(int pid);
    /**
     * @brief Get a list of a given process's child processes
     * @param pid The pid of the process, -1 for the currently debugged process
     * @return JSON object result of dptj
     */
    QJsonDocument getChildProcesses(int pid);
    QJsonDocument getBacktrace();
    void startDebug();
    void startEmulation();
    /**
     * @brief attach to a remote debugger
     * @param uri remote debugger uri
     * @note attachedRemote(bool) signals the result
     */
    void attachRemote(const QString &uri);
    void attachDebug(int pid);
    void stopDebug();
    void suspendDebug();
    void syncAndSeekProgramCounter();
    void continueDebug();
    void continueUntilCall();
    void continueUntilSyscall();
    void continueUntilDebug(QString offset);
    void stepDebug();
    void stepOverDebug();
    void stepOutDebug();
    void toggleBreakpoint(RVA addr);
    void toggleBreakpoint(QString addr);
    void delBreakpoint(RVA addr);
    void delAllBreakpoints();
    void enableBreakpoint(RVA addr);
    void disableBreakpoint(RVA addr);
    bool isBreakpoint(const QList<RVA> &breakpoints, RVA addr);
    QList<RVA> getBreakpointsAddresses();
    QString getActiveDebugPlugin();
    QStringList getDebugPlugins();
    void setDebugPlugin(QString plugin);
    bool isDebugTaskInProgress();
    /**
     * @brief Check if we can use output/input redirection with the currently debugged process
     */
    bool isRedirectableDebugee();
    bool currentlyDebugging = false;
    bool currentlyEmulating = false;
    int currentlyAttachedToPID = -1;
    QString currentlyOpenFile;

    /* Decompilers */
    QList<Decompiler *> getDecompilers();
    Decompiler *getDecompilerById(const QString &id);

    /**
     * Register a new decompiler
     *
     * The decompiler must have a unique id, otherwise this method will fail.
     * The decompiler's parent will be set to this CutterCore instance, so it will automatically be freed later.
     *
     * @return whether the decompiler was registered successfully
     */
    bool registerDecompiler(Decompiler *decompiler);

    RVA getOffsetJump(RVA addr);
    QJsonDocument getFileInfo();
    QJsonDocument getSignatureInfo();
    QJsonDocument getFileVersionInfo();
    QStringList getStats();
    void setGraphEmpty(bool empty);
    bool isGraphEmpty();

    void getOpcodes();
    QList<QString> opcodes;
    QList<QString> regs;
    void setSettings();

    void loadPDB(const QString &file);

    QByteArray ioRead(RVA addr, int len);

    QList<RVA> getSeekHistory();

    /* Plugins */
    QStringList getAsmPluginNames();
    QStringList getAnalPluginNames();

    /* Projects */
    QStringList getProjectNames();
    void openProject(const QString &name);
    void saveProject(const QString &name);
    void deleteProject(const QString &name);
    static bool isProjectNameValid(const QString &name);

    /* Widgets */
    QList<RBinPluginDescription> getRBinPluginDescriptions(const QString &type = QString());
    QList<RIOPluginDescription> getRIOPluginDescriptions();
    QList<RCorePluginDescription> getRCorePluginDescriptions();
    QList<RAsmPluginDescription> getRAsmPluginDescriptions();
    QList<FunctionDescription> getAllFunctions();
    QList<ImportDescription> getAllImports();
    QList<ExportDescription> getAllExports();
    QList<SymbolDescription> getAllSymbols();
    QList<HeaderDescription> getAllHeaders();
    QList<ZignatureDescription> getAllZignatures();
    QList<CommentDescription> getAllComments(const QString &filterType);
    QList<RelocDescription> getAllRelocs();
    QList<StringDescription> getAllStrings();
    QList<FlagspaceDescription> getAllFlagspaces();
    QList<FlagDescription> getAllFlags(QString flagspace = QString());
    QList<SectionDescription> getAllSections();
    QList<SegmentDescription> getAllSegments();
    QList<EntrypointDescription> getAllEntrypoint();
    QList<BinClassDescription> getAllClassesFromBin();
    QList<BinClassDescription> getAllClassesFromFlags();
    QList<ResourcesDescription> getAllResources();
    QList<VTableDescription> getAllVTables();

    /**
     * @return all loaded types
     */
    QList<TypeDescription> getAllTypes();

    /**
     * @return all loaded primitive types
     */
    QList<TypeDescription> getAllPrimitiveTypes();

    /**
     * @return all loaded unions
     */
    QList<TypeDescription> getAllUnions();

    /**
     * @return all loaded structs
     */
    QList<TypeDescription> getAllStructs();

    /**
     * @return all loaded enums
     */
    QList<TypeDescription> getAllEnums();

    /**
     * @return all loaded typedefs
     */
    QList<TypeDescription> getAllTypedefs();

    /**
     * @brief Fetching the C representation of a given Type
     * @param name - the name or the type of the given Type / Struct
     * @param category - the category of the given Type (Struct, Union, Enum, ...)
     * @return The type decleration as C output
     */
    QString getTypeAsC(QString name, QString category);


    /**
     * @brief Adds new types
     * It first uses the r_parse_c_string() function from radare2 API to parse the
     * supplied C file (in the form of a string). If there were errors, they are displayed.
     * If there were no errors, it uses sdb_query_lines() function from radare2 API
     * to save the parsed types returned by r_parse_c_string()
     * \param str Contains the definition of the data types
     * \return returns an empty QString if there was no error, else returns the error
     */
    QString addTypes(const char *str);
    QString addTypes(const QString &str) { return addTypes(str.toUtf8().constData()); }

    /**
     * @brief Checks if the given address is mapped to a region
     * @param addr The address to be checked
     * @return true if addr is mapped, false otherwise
     */
    bool isAddressMapped(RVA addr);

    QList<MemoryMapDescription> getMemoryMap();
    QList<SearchDescription> getAllSearch(QString search_for, QString space);
    BlockStatistics getBlockStatistics(unsigned int blocksCount);
    QList<BreakpointDescription> getBreakpoints();
    QList<ProcessDescription> getAllProcesses();
    QList<RegisterRefDescription> getRegisterRefs();
    QJsonObject getRegisterJson();
    QList<VariableDescription> getVariables(RVA at);

    QList<XrefDescription> getXRefs(RVA addr, bool to, bool whole_function,
                                    const QString &filterType = QString());

    QList<StringDescription> parseStringsJson(const QJsonDocument &doc);

    void handleREvent(int type, void *data);

    /* Signals related */
    void triggerVarsChanged();
    void triggerFunctionRenamed(const QString &prevName, const QString &newName);
    void triggerRefreshAll();
    void triggerAsmOptionsChanged();
    void triggerGraphOptionsChanged();

    void message(const QString &msg, bool debug = false);

    QStringList getSectionList();

    RCoreLocked core();

    static QString ansiEscapeToHtml(const QString &text);
    BasicBlockHighlighter *getBBHighlighter();
    BasicInstructionHighlighter *getBIHighlighter();

signals:
    void refreshAll();

    void functionRenamed(const QString &prev_name, const QString &new_name);
    void varsChanged();
    void functionsChanged();
    void flagsChanged();
    void commentsChanged();
    void registersChanged();
    void instructionChanged(RVA offset);
    void breakpointsChanged();
    void refreshCodeViews();
    void stackChanged();
    /**
     * @brief update all the widgets that are affected by rebasing in debug mode
     */
    void codeRebased();

    void switchedThread();
    void switchedProcess();

    void classNew(const QString &cls);
    void classDeleted(const QString &cls);
    void classRenamed(const QString &oldName, const QString &newName);
    void classAttrsChanged(const QString &cls);

    void attachedRemote(bool successfully);

    void projectSaved(bool successfully, const QString &name);

    /**
     * emitted when debugTask started or finished running
     */
    void debugTaskStateChanged();

    /**
     * emitted when config regarding disassembly display changes
     */
    void asmOptionsChanged();

    /**
     * emitted when config regarding graph display changes
     */
    void graphOptionsChanged();

    /**
     * @brief seekChanged is emitted each time radare2 seek value is modified
     * @param offset
     */
    void seekChanged(RVA offset);

    void toggleDebugView();

    void newMessage(const QString &msg);
    void newDebugMessage(const QString &msg);

    void showMemoryWidgetRequested();

private:
    QString notes;

    /**
     * Internal reference to the RCore.
     * NEVER use this directly! Always use the CORE_LOCK(); macro and access it like core->...
     */
    RCore *core_ = nullptr;
    QMutex coreMutex;
    int coreLockDepth = 0;
    void *coreBed = nullptr;

    AsyncTaskManager *asyncTaskManager;
    RVA offsetPriorDebugging = RVA_INVALID;
    QErrorMessage msgBox;

    QList<Decompiler *> decompilers;

    bool emptyGraph = false;
    BasicBlockHighlighter *bbHighlighter;
    BasicInstructionHighlighter biHighlighter;

    QSharedPointer<R2Task> debugTask;
    R2TaskDialog *debugTaskDialog;
};

class RCoreLocked
{
    CutterCore * const core;

public:
    explicit RCoreLocked(CutterCore *core);
    RCoreLocked(const RCoreLocked &) = delete;
    RCoreLocked &operator=(const RCoreLocked &) = delete;
    RCoreLocked(RCoreLocked &&);
    ~RCoreLocked();
    operator RCore *() const;
    RCore *operator->() const;
};

#endif // CUTTER_H
