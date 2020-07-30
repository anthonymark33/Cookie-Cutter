
#ifdef CUTTER_ENABLE_JUPYTER

#include <Python.h>
#include <marshal.h>

#include <QFile>
#include <csignal>

#include "Cutter.h"
#include "NestedIPyKernel.h"

NestedIPyKernel *NestedIPyKernel::start(const QStringList &argv)
{
    PyThreadState *parentThreadState = PyThreadState_Get();

    PyThreadState *threadState = Py_NewInterpreter();
    if (!threadState) {
        qWarning() << "Could not create subinterpreter.";
        return nullptr;
    }

    QFile moduleFile(":/python/cutter_ipykernel.pyc");
    bool isBytecode = moduleFile.exists();
    if (!isBytecode) {
        moduleFile.setFileName(":/python/cutter_ipykernel.py");
    }
    moduleFile.open(QIODevice::ReadOnly);
    QByteArray moduleCode = moduleFile.readAll();
    moduleFile.close();

    PyObject *moduleCodeObject;
    if (isBytecode) {
        moduleCodeObject = PyMarshal_ReadObjectFromString(moduleCode.constData() + 12,
                                                          moduleCode.size() - 12);
    } else {
        moduleCodeObject = Py_CompileString(moduleCode.constData(), "cutter_ipykernel.py",
                                            Py_file_input);
    }
    if (!moduleCodeObject) {
        qWarning() << "Could not compile cutter_ipykernel.";
        return nullptr;
    }
    auto cutterIPykernelModule = PyImport_ExecCodeModule("cutter_ipykernel", moduleCodeObject);
    Py_DECREF(moduleCodeObject);
    if (!cutterIPykernelModule) {
        qWarning() << "Could not import cutter_ipykernel.";
        return nullptr;
    }

    auto kernel = new NestedIPyKernel(cutterIPykernelModule, argv);

    PyThreadState_Swap(parentThreadState);

    return kernel;
}

NestedIPyKernel::NestedIPyKernel(PyObject *cutterIPykernelModule, const QStringList &argv)
{
    threadState = PyThreadState_Get();

    auto launchFunc = PyObject_GetAttrString(cutterIPykernelModule, "launch_ipykernel");

    PyObject *argvListObject = PyList_New(argv.size());
    for (int i = 0; i < argv.size(); i++) {
        QString s = argv[i];
        PyList_SetItem(argvListObject, i, PyUnicode_DecodeUTF8(s.toUtf8().constData(), s.length(),
                                                               nullptr));
    }

    kernel = PyObject_CallFunction(launchFunc, "O", argvListObject);
}

NestedIPyKernel::~NestedIPyKernel()
{
    auto parentThreadState = PyThreadState_Swap(threadState);
    auto ret = PyObject_CallMethod(kernel, "cleanup", nullptr);
    if (!ret) {
        PyErr_Print();
    }
    PyThreadState_Swap(parentThreadState);
}

void NestedIPyKernel::sendSignal(long signum)
{
    auto parentThreadState = PyThreadState_Swap(threadState);
    auto ret = PyObject_CallMethod(kernel, "send_signal", "l", signum);
    if (!ret) {
        PyErr_Print();
    }
    PyThreadState_Swap(parentThreadState);
}

QVariant NestedIPyKernel::poll()
{
    QVariant ret;
    auto parentThreadState = PyThreadState_Swap(threadState);
    PyObject *pyRet = PyObject_CallMethod(kernel, "poll", nullptr);
    if (pyRet) {
        if (PyLong_Check(pyRet)) {
            ret = (qlonglong)PyLong_AsLong(pyRet);
        }
    } else {
        PyErr_Print();
    }
    PyThreadState_Swap(parentThreadState);
    return ret;
}

#endif
