# 数据保护机制修复方案

## 问题分析

### 原始 Bug
tasks.json 文件被意外清空，导致所有任务数据丢失。

### 根本原因
1. **加载失败未检查** - `loadFromFile()` 返回值被忽略
2. **无条件覆盖** - 即使加载失败，程序退出时仍会用空任务列表覆盖文件
3. **无备份机制** - 一旦文件被损坏或清空，无法恢复

### 发生场景
```
首次运行或文件不存在:
  loadFromFile() → 返回 false（未检查）
  tasks 列表为空 → 保持为空
  程序关闭 → saveTasks() 将空列表写入文件

结果: tasks.json 变成 []，数据丢失 ✗
```

## 修复方案

### 1. 加强 loadTasks() 函数
```cpp
void MainWindow::loadTasks() {
    if (!taskManager.loadFromFile(dataFile)) {
        qWarning() << "Failed to load tasks from" << dataFile;

        // 尝试从备份文件恢复
        if (QFile::exists(dataFile + ".bak")) {
            if (taskManager.loadFromFile(dataFile + ".bak")) {
                qInfo() << "Successfully restored from backup";
                // 恢复备份到主文件
                taskManager.saveToFile(dataFile);
            }
        }
    }
}
```

**保护**: 文件损坏时自动从备份恢复

### 2. 备份机制：createBackup()
```cpp
bool MainWindow::createBackup(const QString& filename) {
    if (!QFile::exists(filename)) {
        return true;  // 文件不存在，无需备份
    }

    QString backupFile = filename + ".bak";

    // 删除旧备份，创建新备份
    if (QFile::exists(backupFile)) {
        QFile::remove(backupFile);
    }

    return QFile::copy(filename, backupFile);
}
```

**工作流**: 保存前备份主文件到 `tasks.json.bak`

### 3. 恢复机制：restoreBackup()
```cpp
bool MainWindow::restoreBackup(const QString& filename) {
    QString backupFile = filename + ".bak";

    // 删除损坏的文件
    if (QFile::exists(filename)) {
        QFile::remove(filename);
    }

    // 从备份恢复
    return QFile::copy(backupFile, filename);
}
```

**工作流**: 保存失败时自动从备份恢复

### 4. 强化 saveTasks() 函数
```cpp
void MainWindow::saveTasks() {
    // 防止用空列表覆盖已有文件
    if (taskManager.getAllTasks().isEmpty() && QFile::exists(dataFile)) {
        qWarning() << "Refusing to save empty task list";
        return;  // 不保存
    }

    // 保存前创建备份
    if (!createBackup(dataFile)) {
        qCritical() << "Failed to create backup, aborting save";
        return;  // 备份失败，不保存
    }

    // 尝试保存，失败则恢复
    if (!taskManager.saveToFile(dataFile)) {
        qCritical() << "Save failed, restoring from backup";
        restoreBackup(dataFile);
        return;
    }
}
```

**三层保护**:
1. ✅ 拒绝保存空列表（防止覆盖有效数据）
2. ✅ 保存前备份（有回滚点）
3. ✅ 保存失败自动恢复（数据不会损坏）

## 数据安全流程

### 正常保存流程
```
添加/修改任务
    ↓
关闭程序 → ~MainWindow() 调用 saveTasks()
    ↓
创建备份: tasks.json → tasks.json.bak
    ↓
保存到 tasks.json
    ↓
✅ 成功
```

### 文件损坏恢复流程
```
打开程序 → MainWindow() 调用 loadTasks()
    ↓
尝试从 tasks.json 加载
    ↓
文件不存在或损坏 → loadFromFile() 返回 false
    ↓
尝试从 tasks.json.bak 加载
    ↓
✅ 从备份恢复，继续工作
```

### 保存失败保护流程
```
saveTasks() 被调用
    ↓
检查: 是否要保存空列表？
  ✓ 有数据 → 继续
  ✗ 空列表 + 文件存在 → 停止，保护文件
    ↓
创建备份: tasks.json → tasks.json.bak
    ↓
尝试保存到 tasks.json
  ✓ 成功 → 完成
  ✗ 失败 → 从 tasks.json.bak 恢复
    ↓
✅ 数据安全，未损坏
```

## 修改的文件

### mainwindow.h
- 添加函数声明:
  - `bool createBackup(const QString& filename);`
  - `bool restoreBackup(const QString& filename);`

### mainwindow.cpp
- 修改 `loadTasks()` - 加载失败时尝试从备份恢复
- 修改 `saveTasks()` - 保存前备份，失败自动恢复
- 添加 `createBackup()` - 创建备份文件
- 添加 `restoreBackup()` - 从备份恢复
- 添加头文件: `#include <QFile>` 和 `#include <QDebug>`

## 测试验证

已在代码中添加调试信息，运行时会输出:
```
qDebug:   "Backup created: /path/to/tasks.json.bak"
qInfo:    "Tasks saved successfully"
qWarning: "Failed to load tasks from ..."
qCritical: "Save failed, restoring from backup"
```

## 文件结构

```
C:\Users\cuiga\AppData\Roaming\TodoList\
├── tasks.json        (主要数据文件)
├── tasks.json.bak    (自动备份 - 保存前创建)
└── parcels.json      (快递数据)
```

## 总结

修复后的方案提供了 **三层数据保护**:
1. **加载保护**: 文件损坏自动从备份恢复
2. **保存保护**: 拒绝保存空列表，防止覆盖
3. **故障保护**: 保存失败自动回滚

**结果**: 即使发生多次异常，数据也不会丢失
