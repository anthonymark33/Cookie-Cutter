
#ifndef R2TASK_H
#define R2TASK_H

#include "core/Cutter.h"

class R2Task: public QObject
{
    Q_OBJECT

private:
    RCoreTask *task;

    static void taskFinishedCallback(void *user, char *);
    void taskFinished();

public:
    using Ptr = QSharedPointer<R2Task>;

    explicit R2Task(const QString &cmd, bool transient = true);
    ~R2Task();

    void startTask();
    void breakTask();
    void joinTask();

    QString getResult();
    QJsonDocument getResultJson();
    const char *getResultRaw();

signals:
    void finished();
};

#endif // R2TASK_H
