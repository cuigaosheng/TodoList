#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QJsonObject>

enum class TaskStatus {
    Todo,      // 待做
    InProgress,// 进行中
    Done       // 已做
};

struct Task {
    QString id;
    QString title;
    QString description;
    QString details;   // 任务详情
    TaskStatus status;
    int progress;  // 0-100

    QJsonObject toJson() const;
    static Task fromJson(const QJsonObject& obj);
};

#endif // TASK_H
