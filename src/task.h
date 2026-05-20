#ifndef TASK_H
#define TASK_H

#include <QString>
#include <QJsonObject>
#include <QVector>

enum class TaskStatus {
    Todo,      // 待做
    InProgress,// 进行中
    Done       // 已做
};

// 人员结构体
struct Person {
    QString name;      // 人员名字
    QString details;   // 该人员的任务详情
    int progress;      // 该人员的进度 0-100

    QJsonObject toJson() const;
    static Person fromJson(const QJsonObject& obj);
};

struct Task {
    QString id;
    QString title;
    QString description;
    QString details;   // 任务总体详情
    TaskStatus status;
    int progress;  // 0-100
    QVector<Person> people;  // 涉及的人员列表

    QJsonObject toJson() const;
    static Task fromJson(const QJsonObject& obj);
};

#endif // TASK_H
