#include "taskmanager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QUuid>
#include <QDateTime>
#include <algorithm>

TaskManager::TaskManager() {}

void TaskManager::addTask(const Task& task) {
    Task newTask = task;
    if (newTask.id.isEmpty()) {
        newTask.id = generateId();
    }
    tasks.append(newTask);
}

void TaskManager::removeTask(const QString& id) {
    for (int i = 0; i < tasks.size(); ++i) {
        if (tasks[i].id == id) {
            tasks.removeAt(i);
            return;
        }
    }
}

void TaskManager::updateTask(const Task& task) {
    for (int i = 0; i < tasks.size(); ++i) {
        if (tasks[i].id == task.id) {
            tasks[i] = task;
            return;
        }
    }
}

Task TaskManager::getTask(const QString& id) const {
    for (const auto& task : tasks) {
        if (task.id == id) {
            return task;
        }
    }
    return Task();
}

QVector<Task> TaskManager::getTasks(TaskStatus status) const {
    QVector<Task> result;
    for (const auto& task : tasks) {
        if (task.status == status) {
            result.append(task);
        }
    }
    return result;
}

QVector<Task> TaskManager::getAllTasks() const {
    return tasks;
}

void TaskManager::moveTaskUp(const QString& id) {
    for (int i = 1; i < tasks.size(); ++i) {
        if (tasks[i].id == id) {
            std::swap(tasks[i], tasks[i - 1]);
            return;
        }
    }
}

void TaskManager::moveTaskDown(const QString& id) {
    for (int i = 0; i < tasks.size() - 1; ++i) {
        if (tasks[i].id == id) {
            std::swap(tasks[i], tasks[i + 1]);
            return;
        }
    }
}

void TaskManager::moveTaskToTop(const QString& id) {
    for (int i = 0; i < tasks.size(); ++i) {
        if (tasks[i].id == id) {
            Task task = tasks[i];
            tasks.removeAt(i);
            tasks.insert(0, task);
            return;
        }
    }
}

void TaskManager::moveTaskToBottom(const QString& id) {
    for (int i = 0; i < tasks.size(); ++i) {
        if (tasks[i].id == id) {
            Task task = tasks[i];
            tasks.removeAt(i);
            tasks.append(task);
            return;
        }
    }
}

void TaskManager::moveTaskToStatus(const QString& id, TaskStatus newStatus) {
    for (auto& task : tasks) {
        if (task.id == id) {
            task.status = newStatus;
            return;
        }
    }
}

bool TaskManager::loadFromFile(const QString& filename) {
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }

    QByteArray data = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isArray()) {
        return false;
    }

    tasks.clear();
    QJsonArray array = doc.array();
    for (const auto& value : array) {
        if (value.isObject()) {
            tasks.append(Task::fromJson(value.toObject()));
        }
    }

    return true;
}

bool TaskManager::saveToFile(const QString& filename) {
    QJsonArray array;
    for (const auto& task : tasks) {
        array.append(task.toJson());
    }

    QJsonDocument doc(array);
    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }

    file.write(doc.toJson());
    file.close();
    return true;
}

QString TaskManager::generateId() {
    return QUuid::createUuid().toString(QUuid::WithoutBraces);
}

QJsonObject Person::toJson() const {
    QJsonObject obj;
    obj["name"] = name;
    obj["details"] = details;
    obj["progress"] = progress;
    return obj;
}

Person Person::fromJson(const QJsonObject& obj) {
    Person person;
    person.name = obj["name"].toString();
    person.details = obj["details"].toString();
    person.progress = obj["progress"].toInt(0);
    return person;
}

QJsonObject Task::toJson() const {
    QJsonObject obj;
    obj["id"] = id;
    obj["title"] = title;
    obj["description"] = description;
    obj["details"] = details;
    obj["status"] = static_cast<int>(status);
    obj["progress"] = progress;

    // 序列化人员列表
    QJsonArray peopleArray;
    for (const auto& person : people) {
        peopleArray.append(person.toJson());
    }
    obj["people"] = peopleArray;

    return obj;
}

Task Task::fromJson(const QJsonObject& obj) {
    Task task;
    task.id = obj["id"].toString();
    task.title = obj["title"].toString();
    task.description = obj["description"].toString();
    task.details = obj["details"].toString();
    task.status = static_cast<TaskStatus>(obj["status"].toInt());
    task.progress = obj["progress"].toInt(0);

    // 反序列化人员列表
    QJsonArray peopleArray = obj["people"].toArray();
    for (const auto& value : peopleArray) {
        if (value.isObject()) {
            task.people.append(Person::fromJson(value.toObject()));
        }
    }

    return task;
}
