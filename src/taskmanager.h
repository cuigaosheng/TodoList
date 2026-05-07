#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include "task.h"
#include <QVector>
#include <QString>

class TaskManager {
public:
    TaskManager();

    void addTask(const Task& task);
    void removeTask(const QString& id);
    void updateTask(const Task& task);
    Task getTask(const QString& id) const;

    QVector<Task> getTasks(TaskStatus status) const;
    QVector<Task> getAllTasks() const;

    void moveTaskUp(const QString& id);
    void moveTaskDown(const QString& id);
    void moveTaskToStatus(const QString& id, TaskStatus newStatus);

    bool loadFromFile(const QString& filename);
    bool saveToFile(const QString& filename);

private:
    QVector<Task> tasks;
    QString generateId();
};

#endif // TASKMANAGER_H
