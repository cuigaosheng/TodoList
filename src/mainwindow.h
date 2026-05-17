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
#include <QMap>
#include "taskmanager.h"

// 任务详情对话框 - 显示人员列表和详情
class TaskDetailsDialog : public QDialog {
    Q_OBJECT

public:
    TaskDetailsDialog(Task& task, QWidget* parent = nullptr);
    Task getTask() const;

private slots:
    void onAddPerson();
    void onDeletePerson(int index);
    void onSelectPerson(int index);
    void onNameChanged();
    void onDetailsChanged();
    void onProgressChanged(int value);

private:
    void refreshPersonList();
    void updatePersonDetails();

    Task& task;
    QLineEdit* descriptionEdit;
    QTextEdit* detailsEdit;
    QSpinBox* progressSpinBox;
    QWidget* personButtonsWidget;
    QPushButton* addPersonButton;

    // 右侧人员详情编辑区
    QLineEdit* personNameEdit;
    QTextEdit* personDetailsEdit;
    QSpinBox* personProgressSpinBox;

    int currentPersonIndex;
};

// 人员汇总对话框
class PeopleSummaryDialog : public QDialog {
    Q_OBJECT

public:
    PeopleSummaryDialog(const TaskManager& taskManager, QWidget* parent = nullptr);

private slots:
    void onPersonSelected(const QString& personName);

private:
    void buildPeopleSummary();
    void displayPersonTasks(const QString& personName);

    const TaskManager& taskManager;
    QWidget* peopleButtonsWidget;
    QListWidget* tasksListWidget;
    QTextEdit* detailsEdit;
    QSpinBox* progressSpinBox;

    // 数据结构: map<人名, vector<{任务描述, 工作安排, 进度}>>
    struct PersonTaskInfo {
        QString taskDescription;
        QString details;
        int progress;
    };
    QMap<QString, QVector<PersonTaskInfo>> peopleSummary;
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
    void onMoveToTop();
    void onMoveToBottom();
    void onTaskDoubleClicked(QListWidgetItem* item);
    void onTaskSelectionChanged();
    void onStatusFilterChanged(int index);
    void onViewTaskDetails();
    void onViewPeopleSummary();

private:
    void setupUI();
    void loadTasks();
    void saveTasks();
    void refreshTaskList();
    QString getStatusString(TaskStatus status) const;
    QListWidgetItem* createTaskItem(const Task& task, int index = 0);
    void moveTaskToStatus(const QString& taskId, TaskStatus newStatus);

    TaskManager taskManager;
    QString dataFile;

    QListWidget* taskListWidget;
    QComboBox* statusFilterCombo;
    QPushButton* addButton;
    QPushButton* deleteButton;
    QPushButton* upButton;
    QPushButton* downButton;
    QPushButton* topButton;
    QPushButton* bottomButton;
    QPushButton* detailsButton;
    QPushButton* peopleButton;
};

#endif // MAINWINDOW_H
