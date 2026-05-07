#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QStyledItemDelegate>
#include <QDialog>
#include <QLineEdit>
#include <QSpinBox>
#include <QTextEdit>
#include "taskmanager.h"

// 任务详情对话框
class TaskDetailsDialog : public QDialog {
    Q_OBJECT

public:
    TaskDetailsDialog(const Task& task, QWidget* parent = nullptr);
    Task getTask() const;

private:
    Task task;
    QLineEdit* titleEdit;
    QLineEdit* descriptionEdit;
    QTextEdit* detailsEdit;
    QSpinBox* progressSpinBox;
};

// 编辑任务对话框
class EditTaskDialog : public QDialog {
    Q_OBJECT

public:
    EditTaskDialog(const QString& description, int progress, QWidget* parent = nullptr);
    QString getDescription() const;
    int getProgress() const;

private:
    QLineEdit* descriptionEdit;
    QSpinBox* progressSpinBox;
};

// 自定义委托用于绘制任务项
class TaskItemDelegate : public QStyledItemDelegate {
    Q_OBJECT

public:
    TaskItemDelegate(QObject* parent = nullptr);
    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
    QSize sizeHint(const QStyleOptionViewItem& option,
                   const QModelIndex& index) const override;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onAddTask();
    void onDeleteTask();
    void onEditTask();
    void onMoveUp();
    void onMoveDown();
    void onTaskDoubleClicked(QListWidgetItem* item);
    void onTaskSelectionChanged();
    void onStatusFilterChanged(int index);
    void onViewTaskDetails();

private:
    void setupUI();
    void loadTasks();
    void saveTasks();
    void refreshTaskList();
    QString getStatusString(TaskStatus status) const;
    QListWidgetItem* createTaskItem(const Task& task, int index = 0);
    void moveTaskToStatus(const QString& taskId, TaskStatus newStatus);
    void editTaskProgress(const QString& taskId);

    TaskManager taskManager;
    QString dataFile;

    QListWidget* taskListWidget;
    QComboBox* statusFilterCombo;
    QPushButton* addButton;
    QPushButton* deleteButton;
    QPushButton* upButton;
    QPushButton* downButton;
    QPushButton* detailsButton;
};

#endif // MAINWINDOW_H
